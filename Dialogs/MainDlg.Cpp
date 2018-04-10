#ifndef _MAINDLG_CPP
#define _MAINDLG_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <ShlObj.H>

#include "../Source/Entry.H"
#include "../Source/Init.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/NetLogoDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)
MAINDIALOGINFO gMDI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK MainDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

	static HWND hIPHost = NULL;
	static HWND hLocalFile = NULL;
	static HWND hFTPServer = NULL;
	static HWND hFTPUsername = NULL;
	static HWND hFTPPassword = NULL;
	static HWND hFTPPath = NULL;
	static HWND hUpdateInterval = NULL;
	static HWND hLastUpdate = NULL;
	static HWND hLastIPAddr = NULL;
	static HWND hLastStatus = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG) // Received A Initilize Dialog Message
    {
        gMDI.hWindow = hWnd;

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, MAINDIALOG_MENU_ABOUT, "About");

		hMenu = LoadMenu(ghAppInstance, MAKEINTRESOURCE(IDR_MAINMENU));
		SetMenu(hWnd, hMenu);

		char sTitle[100];
		sprintf_s(sTitle, sizeof(sTitle), "%s :: Configure", gsTitleCaption);

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sTitle);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

		hIPHost				= GetDlgItem(hWnd, IDC_IPHOST);
		hLocalFile			= GetDlgItem(hWnd, IDC_LOCALFILE);
		hFTPServer			= GetDlgItem(hWnd, IDC_FTPSERVER);
		hFTPUsername		= GetDlgItem(hWnd, IDC_FTPUSERNAME);
		hFTPPassword		= GetDlgItem(hWnd, IDC_FTPPASSWORD);
		hFTPPath			= GetDlgItem(hWnd, IDC_FTPPATH);
		hUpdateInterval		= GetDlgItem(hWnd, IDC_UPDATEINTERVAL);
		hLastUpdate			= GetDlgItem(hWnd, IDC_LASTUPDATE);
		hLastIPAddr			= GetDlgItem(hWnd, IDC_LASTIP);
		hLastStatus			= GetDlgItem(hWnd, IDC_LASTSTATUS);

		SetTimer(hWnd, 0, 1000, NULL);

		CenterWindow(hWnd);

		if(gbUploadFileToFTP)
		{
			CheckDlgButton(hWnd, IDC_UPLOADFILETOFTP, BST_CHECKED);
		}

		if(gbUsePassiveFTP)
		{
			CheckDlgButton(hWnd, IDC_PASSIVEFTP, BST_CHECKED);
		}

		SendMessage(hWnd, WM_COMMAND, IDC_UPLOADFILETOFTP, 0);

		Set_Text(hIPHost, gsIPHost);
		Set_Text(hLocalFile, gsLocalFile);
		Set_Text(hFTPServer, gsFTPServer);
		Set_Text(hFTPUsername, gsFTPUsername);
		Set_Text(hFTPPassword, gsFTPPassword);
		Set_Text(hFTPPath, gsFTPPath);
		Set_Long(hUpdateInterval, gdwUpdateInterval);

		Set_Text(hLastUpdate, gsLastTime);
		Set_Text(hLastIPAddr, gsLastIP);
		Set_Text(hLastStatus, gsLastStatus);

		if(gService.RunningAsService())
		{
			ModifyMenu(hMenu, ID_HELP_DOCUMENTATION, MF_BYCOMMAND | MF_GRAYED, NULL, "Help");
		}

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
	else if(uMsg == WM_TIMER)
    {
		Set_Text(hLastUpdate, gsLastTime);
		Set_Text(hLastIPAddr, gsLastIP);
		Set_Text(hLastStatus, gsLastStatus);
	}
    else if(uMsg == WM_COMMAND)
    {
        if(wParam == IDC_UPLOADFILETOFTP)
        {
			bool bEnable = (IsDlgButtonChecked(hWnd, IDC_UPLOADFILETOFTP) > 0);

			EnableWindow(hFTPServer, bEnable);
			EnableWindow(hFTPUsername, bEnable);
			EnableWindow(hFTPPassword, bEnable);
			EnableWindow(hFTPPath, bEnable);
			EnableWindow(GetDlgItem(hWnd, IDC_PASSIVEFTP), bEnable);

            return TRUE;
        }
        else if(wParam == IDC_BROWSEPATH)
        {
			char sDirLoc[MAX_PATH];

			if(BrowseDirectory(hWnd,
				"Select the directoy where your IP information files will be stored.",
				BIF_RETURNONLYFSDIRS|BIF_USENEWUI, sDirLoc, sizeof(sDirLoc)))
			{
				Set_Text(hLocalFile, sDirLoc);
			}
		}
        else if(wParam == IDC_OK)
        {
			SetRegistryValues();

			int iSize = 0;
			
			iSize = Get_Text(hIPHost, gsIPHost, sizeof(gsIPHost));
			if(iSize <= 0 || iSize >= sizeof(gsIPHost))
			{
				MessageBox(hWnd, "You must specifiy a valid URL to obtain your IP from.", gsTitleCaption, MB_ICONINFORMATION);
				GetRegistryValues();
				return TRUE;
			}
			iSize = Get_Text(hLocalFile, gsLocalFile, sizeof(gsLocalFile));
			if(iSize <= 0 || iSize >= sizeof(gsLocalFile))
			{
				MessageBox(hWnd, "You must specifiy a valid local file name to save your IP information to.", gsTitleCaption, MB_ICONINFORMATION);
				GetRegistryValues();
				return TRUE;
			}

			Get_Text(hFTPServer, gsFTPServer, sizeof(gsFTPServer));
			Get_Text(hFTPUsername, gsFTPUsername, sizeof(gsFTPUsername));
			Get_Text(hFTPPassword, gsFTPPassword, sizeof(gsFTPPassword));
			Get_Text(hFTPPath, gsFTPPath, sizeof(gsFTPPath));

			gbUsePassiveFTP = (IsDlgButtonChecked(hWnd, IDC_PASSIVEFTP) > 0);

			if((gbUploadFileToFTP = (IsDlgButtonChecked(hWnd, IDC_UPLOADFILETOFTP) > 0)))
			{
				iSize = strlen(gsFTPServer);
				if(iSize <= 0 || iSize >= sizeof(gsFTPServer))
				{
					MessageBox(hWnd, "You must specifiy a valid FTP server to upload your IP information to.", gsTitleCaption, MB_ICONINFORMATION);
					GetRegistryValues();
					return TRUE;
				}
				iSize = strlen(gsFTPUsername);
				if(iSize <= 0 || iSize >= sizeof(gsFTPUsername))
				{
					MessageBox(hWnd, "You must specifiy an FTP user name.", gsTitleCaption, MB_ICONINFORMATION);
					GetRegistryValues();
					return TRUE;
				}
				iSize = strlen(gsFTPPassword);
				if(iSize <= 0 || iSize >= sizeof(gsFTPPassword))
				{
					MessageBox(hWnd, "You must specifiy an FTP password.", gsTitleCaption, MB_ICONINFORMATION);
					GetRegistryValues();
					return TRUE;
				}
				iSize = strlen(gsFTPPath);
				if(iSize <= 0 || iSize >= sizeof(gsFTPPath))
				{
					MessageBox(hWnd, "You must specifiy a valid remote path and filename for the FTP server.", gsTitleCaption, MB_ICONINFORMATION);
					GetRegistryValues();
					return TRUE;
				}
			}

			gdwUpdateInterval = Get_Long(hUpdateInterval);
			if(gdwUpdateInterval < 10 || gdwUpdateInterval > 15000)
			{
				MessageBox(hWnd, "You must specifiy an update interval between 10 and 15,000 minutes.", gsTitleCaption, MB_ICONINFORMATION);
				GetRegistryValues();
				return TRUE;
			}

			SetRegistryValues();

			gbUpdateNow = true;

			KillTimer(hWnd, 0);
			EndDialog(hWnd, 0);
            DestroyWindow(hWnd);

			gMDI.hWindow = NULL;

            return TRUE;
        }
        else if(wParam == IDC_CANCEL || wParam == ID_FILE_CLOSEAPP)
        {
			KillTimer(hWnd, 0);
            EndDialog(hWnd, 0);
            DestroyWindow(hWnd);

			gMDI.hWindow = NULL;

            return TRUE;
        }
        else if(wParam == ID_HELP_DOCUMENTATION)
        {
			OpenHelpFile();
            return TRUE;
        }
        else if(wParam == ID_HELP_ABOUT)
        {
            _AboutDialogInfo ADI;
    
            ADI.DisplayIcon  = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN));
            ADI.TitleCaption = gsTitleCaption;
            ADI.FileVersion  = gsFileVersion;
            ADI.BuildDate    = __DATE__;
            ADI.BuildTime    = __TIME__;
            ADI.CopyRight    = gsAppCopyRight;
            ADI.OwnerHandle  = hWnd;

            NetLogo(&ADI);
            return TRUE;
        }

        return FALSE;
    }
    //--------------------------------------------------------------------------
	else if(uMsg == WM_PAINT)
    {
        HDC ThisHDC;
        PAINTSTRUCT ThisPS;

        ThisHDC = BeginPaint(hWnd, &ThisPS);

        //- Any painting should be done here.

        EndPaint(hWnd, &ThisPS);
        return TRUE;
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_SYSCOMMAND) //- Received a system menu message.
    {
        if(LOWORD(wParam) == MAINDIALOG_MENU_ABOUT) //- About.
        {
            _AboutDialogInfo ADI;
    
            ADI.DisplayIcon  = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN));
            ADI.TitleCaption = gsTitleCaption;
            ADI.FileVersion  = gsFileVersion;
            ADI.BuildDate    = __DATE__;
            ADI.BuildTime    = __TIME__;
            ADI.CopyRight    = gsAppCopyRight;
            ADI.OwnerHandle  = hWnd;

            NetLogo(&ADI);

            return TRUE;
        }

        return FALSE;
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_CLOSE) //- Received close message.
    {
		KillTimer(hWnd, 0);
        EndDialog(hWnd, 0);
        DestroyWindow(hWnd);

		gMDI.hWindow = NULL;

        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

