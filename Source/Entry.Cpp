#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <WinINet.H>
#include <URLMon.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Init.H"

#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::Registry;
using namespace NSWFL::DateTime;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ID_COPYIPTOCLIP WM_USER + 4
#define ID_UPDATENOW    WM_USER + 5
#define ID_CONFIGURE    WM_USER + 6
#define ID_HELP			WM_USER + 7
#define ID_CANCEL       WM_USER + 8
#define ID_EXIT         WM_USER + 9

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE ghAppInstance = NULL;

char gsAppCopyRight[64];
char gsFileVersion[10];
char gsTitleCaption[64];
char gsRegistryKey[255];
char gsIPHost[1024];
char gsLocalFile[MAX_PATH];

char gsFTPServer[MAX_PATH];
char gsFTPUsername[255];
char gsFTPPassword[255];
char gsFTPPath[1024];
char gsLastIP[200];
char gsLastTime[1024];
char gsLastStatus[1024];

CMemPool gMem; //The global memory object.

bool gbApplicationRunning = false;
bool gbUploadFileToFTP = false;
bool gbUpdateNow = false;
bool gbUsePassiveFTP = false;

DWORD gdwUpdateInterval = 10;

CService gService;
CTrayIcon gTrayIcon;

HANDLE hMonitorThread = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool OpenHelpFile(void)
{
	char sPath[MAX_PATH];
	DWORD dwSize = sizeof(sPath);
	memset(sPath, 0, sizeof(sPath));
	if(Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", sPath, dwSize))
	{
		if(dwSize > 0)
		{
			strcat_s(sPath, sizeof(sPath), "\\Help.chm");
			return(((int)ShellExecute(gTrayIcon.hTrayHandle, "OPEN", sPath, NULL, NULL, SW_MAXIMIZE)) > 32);
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MyTrayIconProc(CTrayIcon *lpTI, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_CTRAYICON_INITIALIZE)
	{
		lpTI = (CTrayIcon *)lParam;
		SetTimer(hWnd, 0, 1000, NULL);
	}
	else if(uMsg == WM_TIMER)
    {
		ResumeThread(hMonitorThread);
		KillTimer(hWnd, 0);
	}
	else if(uMsg == WM_COMMAND)
    {
		if(LOWORD(wParam) == ID_UPDATENOW)
        {
			gbUpdateNow = true;
		}
		else if(LOWORD(wParam) == ID_COPYIPTOCLIP)
        {
			if(OpenClipboard(NULL))
			{
				HANDLE hMemory = NULL;

				int iSize = strlen(gsLastIP);
				if((hMemory = LocalAlloc(LPTR, iSize + 1)))
				{
					EmptyClipboard();
					hMemory = LocalLock(hMemory); 
					memcpy(hMemory, gsLastIP, iSize); 
					LocalUnlock(hMemory); 

					SetClipboardData(CF_TEXT, hMemory); 
				}
				CloseClipboard();
			}
		}
		else if(LOWORD(wParam) == ID_CONFIGURE)
        {
			if(IsWindow(gMDI.hWindow))
			{
				SetForegroundWindow(gMDI.hWindow);
				SetActiveWindow(gMDI.hWindow);
			}
			else{
				DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialog);
			}
        }
        else if(LOWORD(wParam) == ID_EXIT)
        {
			if(gService.RunningAsService())
			{
				if(MessageBox(hWnd, "Are you sure you want to shut down the RWanIP service?", gsTitleCaption, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
				{
					gService.Stop();
				}
			}
			else{
				if(MessageBox(hWnd, "Are you sure you want to exit RWanIP?", gsTitleCaption, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
				{
					DestroyWindow(hWnd);
				}
			}
        }
		else if(LOWORD(wParam) == ID_HELP)
        {
			OpenHelpFile();
        }
		else if(LOWORD(wParam) == ID_CANCEL)
        {
			//Do nothing.
        }
    }
    else if(uMsg == lpTI->WM_TASKBARCREATED)
    {
		gbUpdateNow = true;
    }
    else if(uMsg == WM_SHELLNOTIFY)
    {
		if(lParam == WM_LBUTTONDBLCLK)
		{
			if(IsWindow(gMDI.hWindow))
			{
				SetForegroundWindow(gMDI.hWindow);
				SetActiveWindow(gMDI.hWindow);
			}
			else{
				DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialog);
			}
		}
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ServiceThread(LPVOID lpVoid)
{
    gbApplicationRunning = true;

    hMonitorThread = CreateThread(NULL, 0, MonitorThreadProc, (LPVOID)0, CREATE_SUSPENDED, NULL);

	if(gService.RunningAsService())
	{
		gService.KeepServiceRunning();
	}
	else {
		HMENU hPopupMenu = CreateMenu();
		AppendMenu(hPopupMenu, MF_STRING, ID_CONFIGURE, "&Configuration");

		AppendMenu(hPopupMenu, MF_STRING, ID_HELP, "&Help");
		AppendMenu(hPopupMenu, MF_SEPARATOR, 0, "");
		AppendMenu(hPopupMenu, MF_STRING, ID_UPDATENOW, "Update &Now");
		AppendMenu(hPopupMenu, MF_STRING, ID_COPYIPTOCLIP, "&Copy IP to Clipboard");
		AppendMenu(hPopupMenu, MF_SEPARATOR, 0, "");
		AppendMenu(hPopupMenu, MF_STRING, ID_CANCEL, "&Cancel");

		AppendMenu(hPopupMenu, MF_STRING, ID_EXIT, "E&xit");

		gTrayIcon.Create(IDI_MAIN, gsTitleCaption, hPopupMenu, MyTrayIconProc);
	}

	gbApplicationRunning = false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WaitOnApplicationToExit(void)
{
	gTrayIcon.Destroy();

    while(gbApplicationRunning) // Wait on application to exit
    {
        Sleep(1);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI MonitorThreadProc(LPVOID lpVoid)
{
    while(gbApplicationRunning) // Wait on application to exit
    {
		TrayMessage("Downloading IP information.");

		if(URLDownloadToFile(NULL, gsIPHost, gsLocalFile, NULL, NULL) == S_OK)
		{
			TrayMessage("Parsing XML tags.");

			CXMLReader MyIP;
			if(MyIP.FromFile(gsLocalFile))
			{
				char sText[1024];
				char sDate[64];
				char sTime[64];
				char sIP[200];

				TrayMessage("Retreiving IP XML tag.");
				if(MyIP.ToString("IP", sIP, sizeof(sIP)) >= 0)
				{
					strcpy_s(gsLastIP, sizeof(gsLastIP), sIP);

					Get_ShortDate(sDate, sizeof(sDate));
					Get_Time(sTime, sizeof(sTime));
					sprintf_s(gsLastTime, sizeof(gsLastTime), "%s %s", sDate, sTime);

					if(gbUploadFileToFTP)
					{
						TrayMessage("Uploading file to FTP server.");

						if(UploadToFTP(gsFTPServer, gsFTPUsername, gsFTPPassword, gsLocalFile, gsFTPPath))
						{
							sprintf_s(sText, sizeof(sText), "IP: %s, Time: %s", sIP, gsLastTime);
							TraySuccess(sText);
						}
						else TrayError("Failed to upload IP to FTP server.");
					}
					else{
						sprintf_s(sText, sizeof(sText), "IP: %s, Time: %s", sIP, gsLastTime);
						TraySuccess(sText);
					}
				}
				else{
					TrayError("Coundnt find IP in XML file.");
				}
				MyIP.Destroy();
			}
			else{
				TrayError("Failed to open XML file.");
			}
		}
		else{
			TrayError("Failed to download XML file.");
		}

		int iSleeps = (gdwUpdateInterval * 60);

		if(iSleeps <= 10)
		{
			iSleeps = 10;
		}

		Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "LastTime", gsLastTime);
		Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "LastIP", gsLastIP);
		Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "LastStatus", gsLastStatus);

		while(!gbUpdateNow && iSleeps > 0)
		{
			iSleeps--;
			Sleep(1000);
			if(Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UpdateValues"))
			{
				DeleteRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UpdateValues");
				GetRegistryValues();
				break;
			}
		}

		gbUpdateNow = false;
		Sleep(1);
    }
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int StartRoutine(const char *sCmdLine)
{
	HANDLE hMutex = NULL;

	if( (hMutex = OpenMutex(SYNCHRONIZE, FALSE, gsTitleCaption)) )
	{
		MessageBox(NULL, "Only one instance can run at a time!",
			gsTitleCaption, MB_ICONASTERISK | MB_SYSTEMMODAL);

		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return 0;
	}
	else hMutex = CreateMutex(0, 0, gsTitleCaption);

	if(_strcmpi(sCmdLine, "") == 0)
	{
		gService.RunAsStandAlone();
	}
	else if(_strcmpi(sCmdLine, "/RunService") == 0)
	{
		gService.RunAsService();
	}
	else{
		if(_strcmpi(sCmdLine, "/NonService") == 0)
		{
			gService.RunAsStandAlone();
		}
	}

	WaitOnApplicationToExit();

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TrayMessage(const char *sMessage)
{
	strcpy_s(gsLastStatus, sizeof(gsLastStatus), sMessage);
	gTrayIcon.Update(sMessage);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TraySuccess(const char *sMessage)
{
	strcpy_s(gsLastStatus, sizeof(gsLastStatus), "Success!");
	gTrayIcon.Update(IDI_LINK, sMessage);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TrayError(const char *sError)
{
	strcpy_s(gsLastStatus, sizeof(gsLastStatus), sError);
	gTrayIcon.Update(IDI_UNLINK, sError);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR sCmdLine, int iCmdShow)
{
	ghAppInstance = hInstance;

    int iResult = 0;

	if(!InitializeApp())
    {
        return 1;
    }

	if(_strcmpi(sCmdLine, "/Config") == 0)
	{
		DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialog);
		Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UpdateValues", 1);
	}
	else{

		if(strlen(sCmdLine) > 0)
		{
			CCmdLine *pCmdLine = new CCmdLine();

			int iArgSz = 0;
			char *sArg = NULL;
			char *sMsg = NULL;

			for(int iArg = 0; iArg < pCmdLine->Count(); iArg++)
			{
				sArg = "/NonService";
				iArgSz = strlen(sArg);
				if(_strnicmp(pCmdLine->Item(iArg), sArg, iArgSz) == 0)
				{
					return StartRoutine(sArg);
				}

				sArg = "/RunService";
				iArgSz = strlen(sArg);
				if(_strnicmp(pCmdLine->Item(iArg), sArg, iArgSz) == 0)
				{
					return StartRoutine(sArg);
				}
				
				sArg = "/Install";
				iArgSz = strlen(sArg);
				if(_strnicmp(pCmdLine->Item(iArg), sArg, iArgSz) == 0)
				{
					if( gService.IsInstalled() == _SVC_NO_ )
					{
						char sDisplayName[255];
						sprintf_s(sDisplayName, sizeof(sDisplayName), "NetworkDLS %s", gsTitleCaption);
						if(gService.Install(sDisplayName) == _SVC_SUCCESS_)
						{
							gService.SetDescription("Polls the WAN IP address of the local machine.");
							return 0;
						}
						else{
							MessageBox(NULL, "Failed to install the service.", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
							return 0;
						}
					}
					return 0;
				}

				sArg = "/Start";
				iArgSz = strlen(sArg);
				if(_strnicmp(pCmdLine->Item(iArg), sArg, iArgSz) == 0)
				{
					if( gService.IsInstalled() == _SVC_NO_ )
					{
    					MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
						return 0;
					}

					if(gService.IsActive() == _SVC_YES_)
					{
						return 0;
					}

					if(gService.Start() == _SVC_SUCCESS_)
					{
						return 0;
					}
					else{
    					MessageBox(NULL, "Failed to start the service.", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
   						return 0;
					}

					return 0;
				}

				sArg = "/Delete";
				iArgSz = strlen(sArg);
				if(_strnicmp(pCmdLine->Item(iArg), sArg, iArgSz) == 0)
				{
					if( gService.IsInstalled() == _SVC_NO_ )
					{
						return 0;
					}

					if(gService.IsActive() == _SVC_YES_)
					{
						gService.Stop(); // Failure would not be an error.
						return 0;
					}

    				if( gService.Uninstall() == _SVC_SUCCESS_ )
    				{
    					return 0;
       				}
					else{
    					MessageBox(NULL, "Failed to mark the service for deletion.", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
						return 0;
					}

					return 0;
				}

				sArg = "/Stop";
				iArgSz = strlen(sArg);
				if(_strnicmp(pCmdLine->Item(iArg), sArg, iArgSz) == 0)
				{
					if( gService.IsInstalled() == _SVC_NO_ )
					{
						return 0;
					}

					if(gService.IsActive() == _SVC_NO_)
					{
						return 0;
					}

    				if(gService.Stop())
    				{
    					return 0;
       				}
					else{
    					MessageBox(NULL, "Failed to stop the service.", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
						return 0;
					}
					return 0;
				}
			}

			delete pCmdLine;
		}
		else{
			iResult = StartRoutine("/NonService");
		}
	}

    if(!UninitializeApp())
    {
        return 1;
    }

	return iResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UploadToFTP(char *sServer, char *sUsername, char *sPassword, char *sLocalFile, char *sRemoteFile)
{
	HINTERNET hInternet = InternetOpen(gsTitleCaption, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	bool bResult = false;

    if(hInternet)
    {
		DWORD dwFlags = 0;

		if(gbUsePassiveFTP)
		{
			dwFlags = INTERNET_FLAG_PASSIVE;
		}

        HINTERNET hService = InternetConnect(hInternet, sServer,
			INTERNET_DEFAULT_FTP_PORT, sUsername, sPassword, INTERNET_SERVICE_FTP, dwFlags, 0);


    	if(hService)
        {
            bResult = (FtpPutFile(hService, sLocalFile, sRemoteFile, FTP_TRANSFER_TYPE_ASCII, 0) > 0);
			InternetCloseHandle(hInternet);

        }

		InternetCloseHandle(hInternet);
    }

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

