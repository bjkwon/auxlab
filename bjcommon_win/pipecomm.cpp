// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: audfret
// Miscellaneous Support Library
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#include "audfret.h"


HANDLE InitPipe (char *PipeName, char *errStr)
{
	// This waits for a pipe initiation (blocks till a pipe connection or an error occurs).
	// Run this inside of a second thread.
	SECURITY_ATTRIBUTES	sa;
	SECURITY_DESCRIPTOR	sd;
	HANDLE hPipe;

	if (! InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
	{ 	strcpy (errStr, "ERROR in InitializeSecurityDescriptor()");	return NULL;	}

	if (!SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE))
	{ 	strcpy (errStr, "ERROR in SetSecurityDescriptorDacl()");	return NULL;	}

	sa.nLength = sizeof(sa);			// fill sa fields;  no inheritance needed
	sa.bInheritHandle = FALSE;
	sa.lpSecurityDescriptor = &sd;
	if ( (hPipe = CreateNamedPipe( PipeName, PIPE_ACCESS_DUPLEX, 
		PIPE_TYPE_MESSAGE  | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1, _MAX_PATH +100, _MAX_PATH +100, 5000, &sa)) == INVALID_HANDLE_VALUE)
	{	GetLastErrorStr(errStr); strcat (errStr, "  [error in CreateNamedPipe]"); return NULL;	}

	if (!ConnectNamedPipe(hPipe, NULL))	
	{	GetLastErrorStr(errStr); strcat (errStr, "  [error in ConnectNamedPipe]"); return NULL;	}
	
	return hPipe;
}

int __callPipe (HWND hDlg, char *Pipename, const char *PipeMsg2Send, char *PipeMsg2Rec, int LenRec, char *errstr)
{
	char reply[MAX_PATH];
	int res;
	DWORD nRead(9999), errcode;
	errstr[0]=0;
	res = CallNamedPipe(Pipename, (LPVOID)PipeMsg2Send, (DWORD)strlen(PipeMsg2Send)+1,
		PipeMsg2Rec, LenRec, &nRead, NMPWAIT_WAIT_FOREVER);
	while ( (res!=1) && (nRead!=0) )
	{
		if ((errcode=GetLastError())==ERROR_MORE_DATA)
		{
			wsprintf(errstr, "Pipe return message is truncated.");
			return 1;
		}
		else if (errcode==233) // No process is on other end of the pipe
		{
			// Ignore and return
			return 1;
		}
		else
		{
			GetLastErrorStr(errcode, reply);
			if (errcode==2) strcat(reply, " (Forgot to run Presenter?)");
			wsprintf (errstr, "Error in CallNamedPipe\n%s", reply);
			while (MessageBox (hDlg, errstr, Pipename, MB_RETRYCANCEL)==IDRETRY) 
			{
				if ((res = CallNamedPipe(Pipename, (LPVOID)PipeMsg2Send, (DWORD)strlen(PipeMsg2Send)+1,
						PipeMsg2Rec, LenRec, &nRead, NMPWAIT_WAIT_FOREVER))) break;
			}
			if (res!=1) nRead=0;
		}
	}
	PipeMsg2Rec[nRead]='\0';
	return res;
}

int CallPipe2 (HWND hDlg, char *remotePC, char *pipenode, const char *PipeMsg2Send, char *PipeMsg2Rec, int LenRec, char *errstr)
{
	char Pipename[MAX_PATH];
	wsprintf(Pipename, "\\\\%s\\pipe\\%s", remotePC, pipenode);
	return __callPipe(hDlg, Pipename, PipeMsg2Send, PipeMsg2Rec, LenRec, errstr);
}

int CallPipe (HWND hDlg, char *pipenode, const char *PipeMsg2Send, char *PipeMsg2Rec, int LenRec, char *errstr)
{
	char Pipename[MAX_PATH];
	wsprintf(Pipename, "\\\\.\\pipe\\%s", pipenode);
	return __callPipe(hDlg, Pipename, PipeMsg2Send, PipeMsg2Rec, LenRec, errstr);
}

#include <winsock.h>

/* Mystery behavior report
In if5base machine (Windows 7), located in the gally lab, but not part of gallaudet domain,
ping (any_name) will resolve with an IP address 54.174.111.151 
In other words, gethostbyname call results in that address and a further call such as connect() gets a timeout error.
Therefore, in this situation, PC name cannot be used. Instead, the IP address of the target machine should be used.
02/22/2017 bjk
*/

