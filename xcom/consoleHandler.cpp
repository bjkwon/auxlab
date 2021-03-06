// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.7075
// Date: 5/8/2021
// 
#include "graffy.h" // this should come before the rest because of wxx820
#include <windows.h> 
#include <stdio.h>
#include "consts.h"
#ifndef SIGPROC
#include "sigproc.h"
#endif
#include "audstr.h"
#include "showvar.h"
#include "histDlg.h"
#include "utils.h"
#include "xcom.h"

extern double block;
extern HANDLE hStdin, hStdout; 
extern CShowvarDlg mShowDlg;
extern CHistDlg mHistDlg;
extern xcom mainSpace;
extern HANDLE hEventLastKeyStroke2Base;

BOOL CtrlHandler( DWORD fdwCtrlType ) 
{ 
	sendtoEventLogger("in CtrlHandler");

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
		closeXcom();
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

size_t ReadLines(HANDLE hCon, char *buf, CONSOLE_SCREEN_BUFFER_INFO coninfo0, CONSOLE_SCREEN_BUFFER_INFO coninfo, size_t offset)
{
	string line;
	size_t res, last, readCount = ReadThisLine(line, hStdout, coninfo0, coninfo0.dwCursorPosition.Y, mainSpace.comPrompt.length());
	strcpy(buf, line.c_str());
	last = mainSpace.comPrompt.length() + readCount;
	// Adjust coninfo based on offset (if current cursor is moved back (or even a previous line),
	// we still need to inspect the entire input based on offset. Here, we temporarily modify coninfo.
	coninfo.dwCursorPosition.X += (short)offset;
	while (coninfo.dwCursorPosition.X >= coninfo0.dwMaximumWindowSize.X)
	{
		coninfo.dwCursorPosition.X -= coninfo0.dwMaximumWindowSize.X;
		coninfo.dwCursorPosition.Y++;
	}
	for (short k = coninfo0.dwCursorPosition.Y + 1; k < coninfo.dwCursorPosition.Y + 1; k++)
	{
		if (last != coninfo0.dwMaximumWindowSize.X)
		{
			readCount++;
			strcat(buf, "\n");
		}
		readCount += res = ReadThisLine(line, hStdout, coninfo0, k, 0);
		if (res > 0)
		{
			strcat(buf, line.c_str());
			last += res;
		}
	}
	return readCount;
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
	bool controlkeydown;
	WORD vcode;
	DEBUG_KEY retval;
	char delimLogStr[64];
	strcpy(delimLogStr, "\r\n");
	strcat(delimLogStr, EXP_AUTO_CORRECT_TAG);

try {
	while (loop)
	{		
		ReadConsoleInput(hStdin, in, INRECORD_SIZE, &nRec);
		if (xscope.size()>1)
			checkdebugkey(in, nRec);
		//nRec can be greater than one when 1) control-v is pressed (for processed input), or 2) debug command string is dispatched from OnNotify of debugDlg
		controlkeydown = false;
		for (UINT k=0; k<nRec; k++)
		{
			if (in[k].EventType !=	KEY_EVENT) 	continue;
			if (!in[k].Event.KeyEvent.bKeyDown) continue; // don't do anythin for when the key is up
			code = in[k].Event.KeyEvent.uChar.AsciiChar;			
			vcode = in[k].Event.KeyEvent.wVirtualKeyCode;
//			if (in[k].Event.KeyEvent.wVirtualKeyCode == VK_CONTROL)
				controlkeydown = !(in[k].Event.KeyEvent.bKeyDown==0);
			if (vcode==0x56 && (in[k].Event.KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED+RIGHT_CTRL_PRESSED)) && in[k].Event.KeyEvent.bKeyDown ) //control-V 
			{
				//This works when ENABLE_PROCESSED_INPUT is not set. If set, only a Debug build will work and Release build will not be caught by this. 10/7/2017 bjk
				num += paste(buf+num);
			}
			//Pasting by right mouse click (or right mouse click to invoke the Menu and choose "Paste") will only paste the first line, even if multiple lines were copied... fix it 9/21/2017 bjk
			else
			{
				read = in[k].Event.KeyEvent.uChar.AsciiChar;
				GetConsoleScreenBufferInfo(hStdout, &coninfo);
				off = (coninfo.dwCursorPosition.Y-coninfo0.dwCursorPosition.Y)*coninfo0.dwMaximumWindowSize.Y + coninfo.dwCursorPosition.X-coninfo0.dwCursorPosition.X;
				if (showscreen && num == 0) // ENTER key pressed after "light-copy" from histDlg
					num = (DWORD)ReadLines(hStdout, buf, coninfo0, coninfo, offset);
				COORD      now;
				switch(vcode)
				{ 
				case 0x98:
					// if double-clicked on histDlg, exit the loop and execute the line, as if the enter is pressed.
					loop = false;
					trimRight(buf, " ");
					strcpy(readbuffer, buf);
					strcat(readbuffer, EXP_AUTO_CORRECT_TAG);
					buf1[0] = 0, histOffset = 0;
					coninfo.dwCursorPosition.Y++;
					coninfo.dwCursorPosition.X = 0;
					SetConsoleCursorPosition(hStdout, coninfo.dwCursorPosition);
					break;
				case VK_RETURN:
					//if current line hits the end of the x limit (or goes beyond), move the cursor down one line
					if (mainSpace.comPrompt.length() + num >= coninfo.dwMaximumWindowSize.X)
					{
						coninfo.dwCursorPosition.Y++;
						SetConsoleCursorPosition(hStdout, coninfo.dwCursorPosition);
					}
					if (showscreen && !num)
						num = (DWORD)ReadLines(hStdout, buf, coninfo0, coninfo, offset);
					// When VK_RETURN appears, 
					// case 1) actual key-stroke of the enter key (nRec is 1)
					// case 2) a part of pasted input (i.e., control-v)
					if (nRec<=1)
					{ //case 1
						// if it is actual enter keystroke, usually nRec is 1 (just check also 2 in case it include key up...maybe we never need. 5/8/2021)
						//Exit the loop, set console cursor at the next line, and have xcom::getinput() return 
						loop = false;
						trimRight(buf, " ");
						strcpy(readbuffer, buf);
						strcat(readbuffer, EXP_AUTO_CORRECT_TAG);
						buf1[0] = 0, histOffset = 0;
						coninfo.dwCursorPosition.Y++;
						coninfo.dwCursorPosition.X = 0;
						SetConsoleCursorPosition(hStdout, coninfo.dwCursorPosition);
					}
					else
					{ //case 2
						buf[num++] = '\n';
						// this is the real "action" of pressing the return/enter key, exiting from the loop and return out of getinput()
						WriteConsole(hStdout, "\r\n", 2, &dw2, NULL);
					}
					break;
				case VK_CONTROL:
			GetConsoleScreenBufferInfo(hStdout, &coninfo);
					break;
				case VK_DELETE:
					if (controlkeydown)
					{
						//Do something here if you want a feature of Ctrl-Delete 
					}
					if (!offset) break;
					memcpy(buf1, buf + num - offset + 1, offset);
					memcpy(buf + num-- - offset--, buf1, offset + 1);
					if (showscreen)
					{
						WriteConsole(hStdout, buf1, (DWORD)strlen(buf1) + 1, &dw2, NULL);
						sendtoEventLogger("(VK_DELETE:buf)WriteConsole wrote %d bytes", dw2);
					}
					SetConsoleCursorPosition(hStdout, coninfo.dwCursorPosition);
					break;
				case VK_BACK:
					//don't assume that num==0 means nothing--it could be a light copy (entering from history dlg)
					if (!(num-offset))  break;
					buf1[0] = '\b';
					memcpy(buf1+1, buf+num-offset, offset+1);
					memcpy(buf+num---offset-1, buf1+1, offset+1);
					if (showscreen)
					{
						WriteConsole(hStdout, buf1, (DWORD)strlen(buf1 + 1)+2, &dw2, NULL); // normally +2 is OK for backspace, but putting more char to cover the case where the line number is reduced
					}
				case VK_LEFT:
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
						WriteConsole (hStdout, buf, res2, &dw2, NULL);
						if (res<res1) 
						{
							now.X=0; now.Y=coninfo0.dwCursorPosition.Y+res+1;
							SetConsoleCursorPosition (hStdout, now);
						}
					}
					SetConsoleCursorPosition (hStdout, coninfo0.dwCursorPosition);
					histOffset=offset=0;
					num=0;
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
					break;
				case VK_INSERT:
					replacemode = !replacemode;
					concurinf.bVisible = 1;
					if (!replacemode)	concurinf.dwSize = 70;
					else concurinf.dwSize = 25;
					SetConsoleCursorInfo (hStdout, &concurinf);
					break;
				case 75:
					// control-k
					if (read == '\v')
					{
						if (!offset) break;
						off = num - offset;
						memset(buf + off, ' ', offset);
						coninfo.dwCursorPosition.X = coninfo0.dwCursorPosition.X;
						coninfo.dwCursorPosition.Y = coninfo0.dwCursorPosition.Y;
						SetConsoleCursorPosition(hStdout, coninfo.dwCursorPosition);
						WriteConsole(hStdout, buf, (DWORD)strlen(buf), &dw2, NULL);
						num -= (int)offset;
						buf[num] = 0;
						memset(buf + off, 0, offset);
						GetConsoleScreenBufferInfo(hStdout, &coninfo);
						coninfo.dwCursorPosition.X -= (SHORT)offset;
						SetConsoleCursorPosition(hStdout, coninfo.dwCursorPosition);
						break;
					} // pass through if it's not control-k
				default:
					if (!read || (vcode>=VK_F1 && vcode<=VK_F24) ) break;
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
	// if the input is empty, that means its EXP_AUTO_CORRECT_TAG... 
	// Do this if the input is not empty
	if (strlen(readbuffer) > 0 && strcmp(readbuffer, EXP_AUTO_CORRECT_TAG))
	{
		size_t count = str2vect(tar, readbuffer, delimLogStr);
		sendtoEventLogger("Enter pressed. Trying to logging %s", tar.front().c_str());
		// if a block input is given (i.e., control-V), each line is separately saved/logged.
		LogHistory(tar);
		mHistDlg.AppendHist(tar);
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
	
