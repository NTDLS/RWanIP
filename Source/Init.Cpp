#ifndef _INIT_CPP
#define _INIT_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Init.H"
#include "Entry.H"

#include "../../@Libraries/CNASCCL (Stream)/CNASCCL.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::System;
using namespace NSWFL::Registry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InitializeApp(void)
{
	if(!gMem.Initialize())
	{
		return false;
	}
	
	LoadString(ghAppInstance, IDS_APPCOPYRIGHT, gsAppCopyRight, sizeof(gsAppCopyRight));
    LoadString(ghAppInstance, IDS_FILEVERSION, gsFileVersion, sizeof(gsFileVersion));
    LoadString(ghAppInstance, IDS_TITLECAPTION, gsTitleCaption, sizeof(gsTitleCaption));
    LoadString(ghAppInstance, IDS_REGISTRYKEY, gsRegistryKey, sizeof(gsRegistryKey));\

	GetRegistryValues();

	gService.Initialize(gsTitleCaption, &ServiceThread, &WaitOnApplicationToExit);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UninitializeApp(void)
{
	gMem.Destroy();
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetKey(char *sKeyOut, int iMaxSz)
{
	memset(sKeyOut, 0, iMaxSz);
	char sMachine[1024];
	Get_MachineName(sMachine, sizeof(sMachine));
	CNASCCL NASCCL;
	return NASCCL.HashKey(sMachine, sKeyOut, strlen(sMachine), iMaxSz - 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetRegistryValues(void)
{
	DWORD dwRegValue = 0;

	dwRegValue = sizeof(gsIPHost);
	memset(gsIPHost, 0, dwRegValue);
	Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "IPHost", gsIPHost, dwRegValue);

	dwRegValue = sizeof(gsLocalFile);
	memset(gsLocalFile, 0, dwRegValue);
	Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "LocalFile", gsLocalFile, dwRegValue);

	dwRegValue = sizeof(gsFTPServer);
	memset(gsFTPServer, 0, dwRegValue);
	Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "FTPServer", gsFTPServer, dwRegValue);

	dwRegValue = sizeof(gsFTPUsername);
	memset(gsFTPUsername, 0, dwRegValue);
	Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "FTPUsername", gsFTPUsername, dwRegValue);

	dwRegValue = sizeof(gsFTPPassword);
	memset(gsFTPPassword, 0, dwRegValue);
	if(Get_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "FTPPassword", gsFTPPassword, dwRegValue))
	{
		if(dwRegValue > 0)
		{
			char sKey[1024];
			int iKeyLength = GetKey(sKey, sizeof(sKey));
			CNASCCL NASCCL(sKey, iKeyLength);

			NASCCL.Cipher(gsFTPPassword, dwRegValue);
			NASCCL.DestroyCryptography();
		}
		else{
			strcpy_s(gsFTPPassword, sizeof(gsFTPPassword), "");
		}
	}
	else{
		strcpy_s(gsFTPPassword, sizeof(gsFTPPassword), "");
	}

	dwRegValue = sizeof(gsFTPPath);
	memset(gsFTPPath, 0, dwRegValue);
	Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "FTPPath", gsFTPPath, dwRegValue);

	dwRegValue = sizeof(gsLastTime);
	memset(gsLastTime, 0, dwRegValue);
	Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "LastTime", gsLastTime, dwRegValue);

	dwRegValue = sizeof(gsLastIP);
	memset(gsLastIP, 0, dwRegValue);
	Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "LastIP", gsLastIP, dwRegValue);

	dwRegValue = sizeof(gsLastStatus);
	memset(gsLastStatus, 0, dwRegValue);
	Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "LastStatus", gsLastStatus, dwRegValue);

	Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UpdateInterval", gdwUpdateInterval);
	gbUploadFileToFTP = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UploadFileToFTP");
	gbUsePassiveFTP = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UsePassiveFTP");

	return true;
}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SetRegistryValues(void)
{
	CreateRegistryKey(HKEY_LOCAL_MACHINE, gsRegistryKey);

	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "IPHost", gsIPHost);
	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "LocalFile", gsLocalFile);
	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "FTPServer", gsFTPServer);
	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "FTPUsername", gsFTPUsername);

	int iLength = strlen(gsFTPPassword);
	if(iLength > 0)
	{
		char sFTPPassword[255];
		char sKey[1024];
		int iKeyLength = GetKey(sKey, sizeof(sKey));
		CNASCCL NASCCL(sKey, iKeyLength);

		NASCCL.Cipher(gsFTPPassword, sFTPPassword, iLength);
		NASCCL.DestroyCryptography();

		Set_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "FTPPassword", sFTPPassword, iLength);
	}
	else{
		Set_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "FTPPassword", "", 0);
	}

	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "FTPPath", gsFTPPath);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UpdateInterval", gdwUpdateInterval);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UploadFileToFTP", gbUploadFileToFTP);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UsePassiveFTP", gbUsePassiveFTP);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

