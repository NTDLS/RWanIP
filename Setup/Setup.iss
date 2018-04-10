[Setup]
;-- Main Setup Information
 AppName                = RWanIP
 AppVerName             = RWanIP 1.0.0.4
 AppCopyright           = Copyright © 1995-2015 NetworkDLS.
 DefaultDirName         = {pf}\NetworkDLS\RWanIP
 DefaultGroupName       = NetworkDLS\RWanIP
 UninstallDisplayIcon   = {app}\RWanIP.Exe
 WizardImageFile        = ..\..\@Resources\Setup\LgSetup.bmp
 WizardSmallImageFile   = ..\..\@Resources\Setup\SmSetup.bmp
 PrivilegesRequired     = Admin
 Uninstallable          = Yes
 AppMutex               = RWanIP
 Compression            = ZIP/9
 OutputBaseFilename     = Setup
 MinVersion             = 0.0,5.0

;-- Windows 2000 & XP (Support Dialog)
 AppPublisher    = NetworkDLS
 AppPublisherURL = http://www.NetworkDLS.com/
 AppUpdatesURL   = http://www.NetworkDLS.com/
 AppVersion      = 1.0.0.4

[Tasks]
 Name: InstallService;              Description: "Install as service? (Primarily for FTP)"; GroupDescription: "Startup Options:"; Flags: exclusive
 Name: InstallService\StartService; Description: "Start the service?";                      GroupDescription: "Startup Options:";

 Name: RunAtStart;                  Description: "Run at &startup?";                        GroupDescription: "Startup Options:"; Flags: unchecked exclusive
 Name: RunAtStart\common;           Description: "For all &users";                          GroupDescription: "Startup Options:"; Flags: unchecked exclusive
 Name: RunAtStart\user;             Description: "For me &only";                            GroupDescription: "Startup Options:"; Flags: unchecked exclusive

 Name: RunManually;                 Description: "Do not auto-start?";                      GroupDescription: "Startup Options:"; Flags: unchecked exclusive

[Files]
 Source: "..\..\@AutoUpdate\Win32\AutoUpdate.Exe"; DestDir: "{app}"; Flags: RestartReplace;
 Source: "AutoUpdate.xml";                         DestDir: "{app}"; Flags: IgnoreVersion;
 Source: "..\Release\RWanIP.Exe";                  DestDir: "{app}"; Flags: IgnoreVersion;
 Source: "..\Help\Help.chm";                       DestDir: "{app}";
 Source: "Resources\Start.ico";                    DestDir: "{app}";
 Source: "Resources\Stop.ico";                     DestDir: "{app}";
 Source: "Resources\IPIDs\*.*";                    DestDir: "{app}\IPID Files";

[Icons]
 Name: "{group}\Start RWanIP";     Filename: "{app}\RWanIP.Exe"; Tasks: NOT InstallService;
 Name: "{group}\Configure RWanIP"; Filename: "{app}\RWanIP.Exe"; Parameters: "/Config";
 Name: "{group}\AutoUpdate";       Filename: "{app}\AutoUpdate.Exe";
 Name: "{group}\Documentation";    Filename: "{app}\Help.chm"; Flags: runmaximized;

 Name: "{group}\Start Service";    Filename: "{app}\RWanIP.Exe"; Parameters: "/Start"; IconFilename: "{app}\Start.ico";       Tasks: InstallService;
 Name: "{group}\Stop Service";     Filename: "{app}\RWanIP.Exe"; Parameters: "/Stop";  IconFilename: "{app}\Stop.ico";        Tasks: InstallService;

 Name: "{commonstartup}\RWanIP";   Filename: "{app}\RWanIP.Exe"; Tasks: RunAtStart\common
 Name: "{userstartup}\RWanIP";     Filename: "{app}\RWanIP.Exe"; Tasks: RunAtStart\User
 Name: "{group}\Uninstall";        Filename: "{uninstallexe}"

[Registry]
 Root: HKLM; Subkey: "Software\NetworkDLS\RWanIP"; Flags: uninsdeletekey noerror;
 
 Root: HKLM; Subkey: "Software\NetworkDLS\RWanIP"; ValueName: "Path";             ValueType: String; ValueData: "{app}";                  Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\RWanIP"; ValueName: "UpdateInterval";   ValueType: Dword;  ValueData: "10";                     Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\RWanIP"; ValueName: "UploadFileToFTP";  ValueType: Dword;  ValueData: "0";                      Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\RWanIP"; ValueName: "UsePassiveFTP";    ValueType: Dword;  ValueData: "1";                      Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\RWanIP"; ValueName: "IPHost";           ValueType: String; ValueData: "http://IpId.NTDLS.com/"; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\RWanIP"; ValueName: "LocalFile";        ValueType: String; ValueData: "{app}/LastAddress.html"; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\RWanIP"; ValueName: "FTPServer";        ValueType: String; ValueData: "SomeFTPServer.com";      Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\RWanIP"; ValueName: "FTPUsername";      ValueType: String; ValueData: "";                       Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\RWanIP"; ValueName: "FTPPassword";      ValueType: BINARY; ValueData: "";                       Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\RWanIP"; ValueName: "FTPPath";          ValueType: String; ValueData: "{computername}.html";    Flags: CreateValueIfDoesntExist;

[Run]
 Filename: "{app}\RWanIP.Exe"; Parameters: "/NonService"; Description: "Launch application"; Flags: postinstall nowait skipifsilent shellexec; Tasks: NOT InstallService;
 Filename: "{app}\RWanIP.Exe"; Parameters: "/Config";     Description: "Launch application"; Flags: postinstall nowait skipifsilent shellexec; Tasks: InstallService;
 Filename: "{app}\RWanIP.Exe"; Parameters: "/Install";    Tasks: InstallService;
 Filename: "{app}\RWanIP.Exe"; Parameters: "/Start";      Tasks: InstallService\StartService;

[UninstallRun]
 Filename: "{app}\RWanIP.Exe"; Parameters: "/Stop";   RunOnceId: "StopService";   MinVersion: 0, 4.0; Tasks: InstallService;
 Filename: "{app}\RWanIP.Exe"; Parameters: "/Delete"; RunOnceId: "DeleteService"; MinVersion: 0, 4.0; Tasks: InstallService;
