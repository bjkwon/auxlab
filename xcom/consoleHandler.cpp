// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#include "graffy.h" // this should come before the rest because of wxx820
#include <windows.h> 
#include <stdio.h>
#include "consts.h"
#ifndef SIGPROC
#include "sigproc.h"
#endif
#include "audfret.h"
#include "audstr.h"
#include "showvar.h"
#include "histDlg.h"
#include "xcom.h"

extern double block;
extern HANDLE hStdin, hStdout; 
extern CShowvarDlg mShowDlg;
extern CHistDlg mHistDlg;
extern xcom mainSpace;
extern HANDLE hEventLastKeyStroke2Base;

void closeXcom(const char *AppPath);

BOOL CtrlHandler( DWORD fdwCtrlType ) 
{ 
	HMODULE h = HMODULE_THIS;
	char fullmoduleName[MAX_PATH], AppPath[MAX_PATH], drive[16], dir[256];
 	GetModuleFileName(h, fullmoduleName, MAX_PATH);
 	_splitpath(fullmoduleName, drive, dir, NULL, NULL);
 	sprintf (AppPath, "%s%s", drive, dir);

  switch( fdwCtrlType ) 
  { 
    // This works properly only when ReadConsole or ReadConsoleInput or any other low level functions are used. Not for highlevel such as getchar
    case CTRL_C_EVENT:
//      printf( "Ctrl-C pressed. Exiting...\n" );
// 	  LOGHISTORY("//\t<<CTRL-C Pressed>>")
      return( TRUE );
 
    // CTRL-CLOSE: confirm that the user wants to exit. 
    case CTRL_CLOSE_EVENT: 
		printf("Ctrl-Close event. Exiting..\n");
		closeXcom(AppPath);
		return( TRUE );
 
    // Pass other signals to the next handler. 
    case CTRL_BREAK_EVENT: 
//	  LOGHISTORY("//\t<<CTRL-Break Pressed>>")
      Beep( 1900, 200 ); 
      printf( "Exiting with CTRL_BREAK_EVENT" );
      return FALSE; 
 
    case CTRL_LOGOFF_EVENT: 
      Beep( 1000, 200 ); 
      printf( "Ctrl-Logoff event\n\n" );
      return FALSE; 
 
    case CTRL_SHUTDOWN_EVENT: 
      Beep( 750, 500 ); 
      printf( "Ctrl-Shutdown event\n\n" );
      return FALSE; 
 
    default: 
      return FALSE; 
  } 
} 

#define INRECORD_SIZE 8
#define CONTROLKEY  (in[k].Event.KeyEvent.wVirtualKeyCode==VK_CONTROL)

int paste(char *buf)
{ // buf is the same buf in getinput
	if (!IsClipboardFormatAvailable(CF_TEXT)) 
	{
		MessageBox(NULL, "Clipboard not available", 0, 0);
		return 0;
	}
	int res = OpenClipboard(NULL);
	HANDLE hglb = GetClipboardData(CF_TEXT); 
	char *buff = (char*)GlobalLock(hglb);
	DWORD len = (DWORD)strlen(buff);
	DWORD dw;
	res = WriteConsole(hStdout, buff, len, &dw, NULL);
//	fprintf(fp,"WriteConsole returns %d with len=%d, dw=%d\n", res, len, dw);
	strcpy(buf, buff);
	GlobalUnlock((HGLOBAL)buff);
	res = CloseClipboard();
	return (int)len;
}

bool check_CHAR_INFO_attri(CHAR_INFO c)
{
	if (c.Attributes==0xcdcd || c.Attributes==0)  return false;
	else	return true;
}

size_t CHAR_INFO2char(CHAR_INFO *chbuf, COORD sz, char* out)
{ // Assume: out was alloc'ed prior to this call
	size_t count(0);
	for (int m=0; m<sz.Y; m++)
	{
		for (int n=0; n<sz.X; n++,count++)
		{
			if (!check_CHAR_INFO_attri(chbuf[n+sz.X*m])) continue;
			out[count] = chbuf[n+sz.X*m].Char.AsciiChar;
			if (n==sz.X-1 && out[count]==' ') // if the last element is blank, put a CR
				out[count] = '\n';
		}
		char *pt = trimRight(out, " \r\n\t");
		count = strlen(pt);
	}
	return count;
}


size_t ReadThisLine(string &linebuf, HANDLE hCon, CONSOLE_SCREEN_BUFFER_INFO coninfo0, SHORT thisline, size_t promptoffset)
{ 
	bool loop(true);
	int out;
	size_t res1;
	CHAR_INFO *chs;
	SMALL_RECT srct;
	COORD      now, sz;
	srct.Top = thisline; // coninfo0.srWindow.Top;
	srct.Bottom = thisline;// coninfo0.srWindow.Bottom;
	srct.Left = coninfo0.srWindow.Left + (SHORT)promptoffset;
	srct.Right = coninfo0.srWindow.Right;
	now.X = 0; now.Y = 0;//coninfo0.dwCursorPosition.Y;
	sz.X = coninfo0.dwSize.X-(SHORT)promptoffset;  sz.Y=1;

	chs = new CHAR_INFO[sz.X*sz.Y];
	int res = ReadConsoleOutput (hStdout, chs, sz, now, &srct);
	char *buf = new char[sz.X*sz.Y+1];
	memset(buf,0,sz.X*sz.Y+1);
	res1 = CHAR_INFO2char(chs, sz, buf); 
	delete[] chs;
	linebuf = buf;
	out = (int)strlen(buf); 
	delete[] buf;
	return out;
}


size_t ReadTheseLines(char *readbuffer, DWORD &num, HANDLE hCon, CONSOLE_SCREEN_BUFFER_INFO coninfo0, CONSOLE_SCREEN_BUFFER_INFO coninfo)
{
	string linebuf;
	size_t res, last;
	
	res = ReadThisLine(linebuf, hStdout, coninfo0, coninfo0.dwCursorPosition.Y, mainSpace.comPrompt.length());
	strcpy(readbuffer, linebuf.c_str());
	last = mainSpace.comPrompt.length()+res;
	num = (DWORD)res;
	//check when the line is continuing down to the next line.
	for (int k=coninfo0.dwCursorPosition.Y+1; k<coninfo.dwCursorPosition.Y+1; k++)
	{
		res = ReadThisLine(linebuf, hStdout, coninfo0, k, 0);
		if (res>0)
		{
			if (last%coninfo0.dwMaximumWindowSize.X)
				last=0, num++, strcat(readbuffer, "\n");
			strcat(readbuffer, linebuf.c_str());
			num += (DWORD)res;
			last += res;
		}
		else
			num++, strcat(readbuffer, "\n");
	}
	return num;
}

DEBUG_KEY xcom::getinput(char* readbuffer)
{
	string linebuf;
	readbuffer[0]=0;
	char buffer[4096] = { 0 };
	char buf1[4096]={0}, buf[4096]={0};
	DWORD nRemove(0), nRec, dw2;
	size_t cumError(0), ures;
	vector<string> tar;
	int res, res1;
	INPUT_RECORD in[INRECORD_SIZE];
	CHAR read;
	SHORT delta;
	bool showscreen(true);
	size_t histOffset;
	DWORD num(0); // total count of chracters typed in
	size_t offset; // how many characters shifted left from the last char
	size_t off; // how much shift of the current cursor is from the command prompt
	int line, code;
	CONSOLE_SCREEN_BUFFER_INFO coninfo0, coninfo;
	GetConsoleScreenBufferInfo(hStdout, &coninfo0);
	bool loop(true), returndown(false);
	bool replacemode(false);
	CONSOLE_CURSOR_INFO concurinf;
	offset = histOffset = buf[0] = 0;
	bool controlkeydown(false);
	WORD vcode;
	DEBUG_KEY retval;

try {
	while (loop)
	{		
		//for (int k = 0; k<1; k++)
		//		fprintf(fp, "%d: down=%d, vcode=%d(0x%2x), Char=%s(%d), dwControlKeyState=%d\n", k, in[k].Event.KeyEvent.bKeyDown, in[k].Event.KeyEvent.wVirtualKeyCode, in[k].Event.KeyEvent.wVirtualKeyCode, buf1, in[k].Event.KeyEvent.uChar.AsciiChar, in[k].Event.KeyEvent.dwControlKeyState);
		ReadConsoleInput(hStdin, in, INRECORD_SIZE, &nRec);
		SendMessage(hLog, WM__RCI, (WPARAM)nRec, (LPARAM)0);
/* To track console inputs (keyboard, mouse, menu, etc), this use. It may work on one machine and show a different behavior on a different machine. 11/23/2017
		fp=fopen("in_record","at");
		fprintf(fp,"nRec=%d\n", nRec);
		for (int k=0; k<nRec; k++) 
			if (in[k].EventType == KEY_EVENT)
				fprintf(fp, "%d: down=%d, vcode=%d(0x%2x), Char=%s(%d), dwControlKeyState=%d\n", k, in[k].Event.KeyEvent.bKeyDown, in[k].Event.KeyEvent.wVirtualKeyCode, in[k].Event.KeyEvent.wVirtualKeyCode, buf1, in[k].Event.KeyEvent.uChar.AsciiChar, in[k].Event.KeyEvent.dwControlKeyState);
			else
				fprintf(fp, "in[].EventType = %d\n", in[k].EventType);
		fclose(fp);
*/
		if (CAstSig::vecast.size()>1)
			checkdebugkey(in, nRec);
		//nRec can be greater than one when 1) control-v is pressed (for processed input), or 2) debug command string is dispatched from OnNotify of debugDlg, or 3) maybe in other occassions
		for (UINT k=0; k<nRec; k++)
		{
			if (in[k].EventType !=	KEY_EVENT) 	continue;
			code = in[k].Event.KeyEvent.uChar.AsciiChar;			vcode = in[k].Event.KeyEvent.wVirtualKeyCode;
//			if (vcode>=0x30 && vcode<=0x5a) sprintf(buf1, "%c", vcode);
//			else strcpy(buf1, "");
//			fprintf(fp,"down=%d, vcode=%d(0x%2x), Char=%s(%d), dwControlKeyState=%d\n", in[k].Event.KeyEvent.bKeyDown, in[k].Event.KeyEvent.wVirtualKeyCode, in[k].Event.KeyEvent.wVirtualKeyCode, buf1, in[k].Event.KeyEvent.uChar.AsciiChar, in[k].Event.KeyEvent.dwControlKeyState );
			if (CONTROLKEY) 
				controlkeydown = !(in[k].Event.KeyEvent.bKeyDown==0);
			if (vcode==0x56 && (in[k].Event.KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED+RIGHT_CTRL_PRESSED)) && in[k].Event.KeyEvent.bKeyDown ) //control-V 
			{
				//This works when ENABLE_PROCESSED_INPUT is not set. If set, only a Debug build will work and Release build will not be caught by this. 10/7/2017 bjk
				num += paste(buf+num);
			}
			//Pasting by right mouse click (or right mouse click to invoke the Menu and choose "Paste") will only paste the first line, even if multiple lines were copied... fix it 9/21/2017 bjk
			else if ( in[k].Event.KeyEvent.bKeyDown)
			{
//				fprintf(fp,"down=%d, vcode=%d(0x%2x), Char=%s(%d), dwControlKeyState=%d\n", in[k].Event.KeyEvent.bKeyDown, in[k].Event.KeyEvent.wVirtualKeyCode, in[k].Event.KeyEvent.wVirtualKeyCode, buf1, in[k].Event.KeyEvent.uChar.AsciiChar, in[k].Event.KeyEvent.dwControlKeyState );
				read = in[k].Event.KeyEvent.uChar.AsciiChar;
				GetConsoleScreenBufferInfo(hStdout, &coninfo);
				off = (coninfo.dwCursorPosition.Y-coninfo0.dwCursorPosition.Y)*coninfo0.dwMaximumWindowSize.Y + coninfo.dwCursorPosition.X-coninfo0.dwCursorPosition.X;
				if (showscreen && off!=num) // This is likely ENTER key from histDlg 
					num = (DWORD)ReadTheseLines(buf, num, hStdout, coninfo0, coninfo);
				COORD      now;
				switch(vcode)
				{ 
				case VK_RETURN:
					// Is this actual return or enter key pressing, or transport of a block?
					// For pasting, i.e., control-v---> keep in the loop; otherwise, get out of the loop
					if (!controlkeydown) 
						loop=false;
					// if characters were already typed in or pasted on the console lines, they were registered in buf and num represents the count of them
					// if characters were lightly copied (pressing enter from the history window), they were not registered in num, so num is zero
//					buf[num++] = '\n'; //Without this line, when multiple lines are pasted with Control-V, Debug version will work, but Release will not (no line separation..just back to back and error will occur)
					if (showscreen && !num)
						num = (DWORD)ReadTheseLines(buf, num, hStdout, coninfo0, coninfo);
					else // Either control-V followed by enter pressing in Debug or debug command (#step #cont ....)
						buf[num++] = '\n'; 
					{
						int idd = 0;
						while (idd<11)
						{
							if (buf[idd++] != ' ') break;
						}
						if (idd==11)
							MessageBox(NULL, "empty", "", 0);
					}
					strcpy(readbuffer, buf);
					buf1[0] = 0, histOffset = 0; 
					WriteConsole (hStdout, "\r\n", 2, &dw2, NULL); // this is the real "action" of pressing the return/enter key, exiting from the loop and return out of getinput()
					break;
				case VK_CONTROL:
			GetConsoleScreenBufferInfo(hStdout, &coninfo);
//			fprintf(fp," VK_CONTROL current cursor x=%d, y=%d\n", coninfo.dwCursorPosition.X, coninfo.dwCursorPosition.Y);
					break;
				case VK_DELETE:
					if (!offset) break;
					memcpy(buf1, buf + num - offset + 1, offset);
					memcpy(buf + num-- - offset--, buf1, offset + 1);
					sprintf(buffer, "(VK_DELETE:buf1)%s", buf1);
					SendMessage(hLog, WM__LOG, (WPARAM)strlen(buf1), (LPARAM)buffer);
					sprintf(buffer, "(VK_DELETE:buf)%s", buf);
					SendMessage(hLog, WM__LOG, (WPARAM)strlen(buf), (LPARAM)buffer);
					if (showscreen)
					{
						WriteConsole(hStdout, buf1, (DWORD)strlen(buf1) + 1, &dw2, NULL);
						sprintf(buffer, "(VK_DELETE:buf)WriteConsole wrote %d bytes", dw2);
						SendMessage(hLog, WM__LOG, -1, (LPARAM)buffer);
					}
					SetConsoleCursorPosition(hStdout, coninfo.dwCursorPosition);
					break;
				case VK_BACK:
					//don't assume that num==0 means nothing--it could be a light copy (entering from history dlg)
					if (!(num-offset))  break;
					buf1[0] = '\b';
					memcpy(buf1+1, buf+num-offset, offset+1);
					memcpy(buf+num---offset-1, buf1+1, offset+1);
					sprintf(buffer, "(VK_BACK:buf1)%s", buf1);
					SendMessage(hLog, WM__LOG, (WPARAM)strlen(buf1), (LPARAM)buffer);
					sprintf(buffer, "(VK_BACK:buf)%s", buf);
					SendMessage(hLog, WM__LOG, (WPARAM)strlen(buf), (LPARAM)buffer);
					if (showscreen)
					{
						WriteConsole(hStdout, buf1, (DWORD)strlen(buf1 + 1)+2, &dw2, NULL); // normally +2 is OK for backspace, but putting more char to cover the case where the line number is reduced
						sprintf(buffer, "(VK_BACK:buf)WriteConsole wrote %d bytes", dw2);
						SendMessage(hLog, WM__LOG, -1, (LPARAM)buffer);
					}
				case VK_LEFT:
//					fprintf(fp,"wVirtualKeyCode=%d,num=%d,offset=%d\n",vcode, num,offset);
//					if (num) fprintf(fp,"num:%s\n",buf);
					if (vcode==VK_LEFT && !(num-offset)) break;
					if (in[k].Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED || in[k].Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED)
						delta = (SHORT)( strlen(buf) - ctrlshiftleft(buf, (DWORD)offset) - offset );
					else
						delta = 1;
					for (int pp=0; pp<delta; pp++)
					{
						if ( (coninfo.dwCursorPosition.Y-coninfo0.dwCursorPosition.Y) && !coninfo.dwCursorPosition.X)
						{
							coninfo.dwCursorPosition.X = coninfo0.dwMaximumWindowSize.X-1; 
							coninfo.dwCursorPosition.Y--;
						}
						else
							coninfo.dwCursorPosition.X--;
					}
					SetConsoleCursorPosition (hStdout, coninfo.dwCursorPosition);
					if (vcode==VK_LEFT) offset += delta;
					break;
				case VK_RIGHT:
					if (in[k].Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED || in[k].Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED)
						delta = (SHORT)(ctrlshiftright(buf, (DWORD)offset) - off);
					else
						delta = 1;
					//first determine if current location is inside the range of num, if not break
					for (int pp=0; pp<delta; pp++)
						if (off<num)
						{
							coninfo.dwCursorPosition.X++;
							if (coninfo.dwCursorPosition.X>coninfo.dwMaximumWindowSize.X)  coninfo.dwMaximumWindowSize.Y++, coninfo.dwCursorPosition.X++;
							// if the current cursor is on the bottom, the whole screen should be scrolled---do this later.
							SetConsoleCursorPosition (hStdout, coninfo.dwCursorPosition);
							offset--;
						}
					break;
				case VK_SHIFT:
				case VK_MENU:
				case VK_PAUSE:
				case VK_CAPITAL:
				case VK_HANGUL:
				case 0x16:
				case 0x17:
				case 0x18:
				case 0x19:
				case 0x1c:
				case VK_SNAPSHOT:
					break;
				case VK_ESCAPE:
					SetConsoleCursorPosition (hStdout, coninfo0.dwCursorPosition);
					if (num>0)
					{
						memset(buf, ' ', num);
						memset(buf+num, 0, 1);
					}
					else
					{
						memset(buf, ' ', coninfo0.dwMaximumWindowSize.X);
						memset(buf+coninfo0.dwMaximumWindowSize.X, 0, 1);
					}
					res1 = coninfo.dwCursorPosition.Y-coninfo0.dwCursorPosition.Y;
					for (res=0; res<=res1; res++)
					{
						DWORD res2;
						if (num==0)
							res2 = (DWORD)ReadThisLine(linebuf, hStdout, coninfo0, res, res==0? mainSpace.comPrompt.size(): 0 );
						else
							res2 = num;
						WriteConsole (hStdout, buf, min(res2,(DWORD)coninfo0.dwMaximumWindowSize.X), &dw2, NULL);
						if (res<res1) 
						{
							now.X=0; now.Y=coninfo0.dwCursorPosition.Y+res+1;
							SetConsoleCursorPosition (hStdout, now);
						}
					}
					SetConsoleCursorPosition (hStdout, coninfo0.dwCursorPosition);
					histOffset=offset=0;
					num=0;
					sprintf(buffer, "(VK_ESCAPE)Cursor moved to %d", coninfo.dwCursorPosition.X);
					break;

				case VK_UP:
				case VK_DOWN:
					if (vcode==VK_UP)
					{
						if (comid==histOffset) break;
						histOffset++;
						if (history.size()>comid-histOffset+1)
							nRemove=(DWORD)history[comid-histOffset+1].size();
						else
							nRemove = num;
					}
					else
					{
						if (histOffset==0) break;
						histOffset--;
						nRemove=(DWORD)history[comid-histOffset-1].size();
					}
					memset(buf, 0, nRemove);
					if (histOffset==0) buf[0]=0;
					else strcpy(buf, history[comid-histOffset].c_str());
					SetConsoleCursorPosition (hStdout, coninfo0.dwCursorPosition);
					num = (DWORD)strlen(buf);
					if (showscreen) WriteConsole (hStdout, buf, max(nRemove, num), &dw2, NULL);
					coninfo.dwCursorPosition.X = coninfo0.dwCursorPosition.X + (SHORT)num;
					coninfo.dwCursorPosition.Y = coninfo0.dwCursorPosition.Y;
					SetConsoleCursorPosition (hStdout, coninfo.dwCursorPosition);
					break;
				case VK_HOME:
					coninfo = coninfo0;
					res = SetConsoleCursorPosition (hStdout, coninfo.dwCursorPosition);
					offset = num;
					if (res)
						sprintf(buffer, "(VK_HOME)Cursor moved to %d", coninfo.dwCursorPosition.X);
					else
						sprintf(buffer, "(VK_HOME)SetConsoleCursorPosition failed");
					SendMessage(hLog, WM__LOG, -1, (LPARAM)buffer);
					break;
				case VK_END:
					line=0;
					ures = ReadThisLine(linebuf, hStdout, coninfo0, coninfo.dwCursorPosition.Y, mainSpace.comPrompt.size()) + mainSpace.comPrompt.size();
					while (ures==coninfo.dwMaximumWindowSize.X)
					{
						line++;
						ures = ReadThisLine(linebuf, hStdout, coninfo0, coninfo.dwCursorPosition.Y+line, 0);
					}
					coninfo.dwCursorPosition.X = mod((SHORT)ures, coninfo.dwMaximumWindowSize.X);
					coninfo.dwCursorPosition.Y += line;
					res = SetConsoleCursorPosition (hStdout, coninfo.dwCursorPosition);
					offset = 0;
					if (res)
						sprintf(buffer, "(VK_END)Cursor moved to %d", coninfo.dwCursorPosition.X);
					else
						sprintf(buffer, "(VK_END)SetConsoleCursorPosition failed");
					SendMessage(hLog, WM__LOG, -1, (LPARAM)buffer);
					break;
				case VK_INSERT:
					replacemode = !replacemode;
					concurinf.bVisible = 1;
					if (!replacemode)	concurinf.dwSize = 70;
					else concurinf.dwSize = 25;
					SetConsoleCursorInfo (hStdout, &concurinf);
					break;
				default:
					if (!read || (vcode>=VK_F1 && vcode<=VK_F24) ) break;
					//This line hopefully cleans the weird character before it appears on the cursor location--do this only when num is zero. --Now, the problem is handled in histDlg.cpp and this can be deleted 9/28/2017
//					if (num==0) SetConsoleCursorPosition (hStdout, coninfo0.dwCursorPosition);
					//default is replace mode (not insert mode)
					// if cursor is in the middle
					if (showscreen) res = WriteConsole (hStdout, &read, 1, &dw2, NULL);
					if (replacemode && offset>0)
						buf[num-offset--] = read;
					else 
					{
						if (offset)
						{
							GetConsoleScreenBufferInfo(hStdout, &coninfo);
							strcpy(buf1, &buf[num-offset]);
							buf[num++-offset] = read;
							strcpy(buf+num-offset, buf1);
							if (showscreen) res = WriteConsole (hStdout, &buf1, (DWORD)strlen(buf1), &dw2, NULL);
							GetConsoleScreenBufferInfo(hStdout, &coninfo);
							coninfo.dwCursorPosition.X -= (SHORT)dw2;
							if (coninfo.dwCursorPosition.X<0) //remainder string continues across line break
							{
								coninfo.dwCursorPosition.Y--;
								coninfo.dwCursorPosition.X += coninfo.dwMaximumWindowSize.X;
							}
							if (showscreen) SetConsoleCursorPosition (hStdout, coninfo.dwCursorPosition);
						}
						else
						{
							buf[num++-offset] = read;
						}
					}
					break;
				}
				if (!loop) k=nRec+1, strcat(buf, "\n");
			}
		}
	}
	// if a block input is given (i.e., control-V), each line is separately saved/logged.
	if (strlen(readbuffer) > 0)
	{
		size_t count = str2vect(tar, readbuffer, "\r\n");
		LogHistory(tar);
		mHistDlg.AppendHist(tar);
//		if (tar.size()>2)
//			MessageBox(NULL, "multiline", tar.front().c_str(),0);
		for (size_t k = 0; k < tar.size(); k++)
		{
			history.push_back(tar[k].c_str());
			comid++;
		}
	}
	retval=non_debug;
}
catch (DEBUG_KEY code)
{
	//without this, it would keep adding K> to the prompt
	coninfo0.dwCursorPosition.X -= (SHORT)strlen(DEBUG_PROMPT);
	SetConsoleCursorPosition (hStdout, coninfo0.dwCursorPosition);
	retval = code;
}
//	fclose(fp);
//	fp=NULL;
//	res = ResetEvent(hThEvent);
	res = SetEvent(hEventLastKeyStroke2Base);
	return retval;
}
	
