#ifndef _ENTRY_H
#define _ENTRY_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../@Libraries/Debug/Debug.H"
#include "../../@Libraries/CMemPool/CMemPool.H"
#include "../../@Libraries/CCmdLine/CCmdLine.H"
#include "../../@Libraries/NSWFL/NSWFL.H"
#include "../../@Libraries/CService/CService.H"
#include "../../@Libraries/CTrayIcon/CTrayIcon.H"
#include "../../@Libraries/CXML/CXML.H"

#include "../Resources/Resource.H"

#include "Routines.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern HINSTANCE ghAppInstance;

extern char gsAppCopyRight[64];
extern char gsFileVersion[10];
extern char gsTitleCaption[64];
extern char gsRegistryKey[255];
extern char gsIPHost[1024];
extern char gsLocalFile[MAX_PATH];
extern char gsFTPServer[MAX_PATH];
extern char gsFTPUsername[255];
extern char gsFTPPassword[255];
extern char gsFTPPath[1024];
extern char gsLastIP[200];
extern char gsLastTime[1024];
extern char gsLastStatus[1024];

extern CMemPool gMem;

extern bool gbApplicationRunning;
extern bool gbUploadFileToFTP;
extern DWORD gdwUpdateInterval;
extern bool gbUpdateNow;
extern bool gbUsePassiveFTP;

extern CService gService;
extern CTrayIcon gTrayIcon;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WaitOnApplicationToExit(void);
bool ServiceThread(LPVOID lpVoid);
bool UploadToFTP(char *sServer, char *sUsername, char *sPassword, char *sLocalFile, char *sRemoteFile);
DWORD WINAPI MonitorThreadProc(LPVOID lpVoid);
void TrayError(const char *sError);
bool OpenHelpFile(void);
void TrayMessage(const char *sMessage);
void TraySuccess(const char *sMessage);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

