/*
 *	This source file contains export function definitions
 *
 *      Copyright (c) 1999 Mark Taylor
 *                    2000 Takehiro TOMINAGA
 *                    2010-2012 Robert Hegemann
 *                    2019 Bomjun Kwon
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <assert.h>
#include <stdio.h>

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char   *strchr(), *strrchr();
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef __sun__
/* woraround for SunOS 4.x, it has SEEK_* defined here */
#include <unistd.h>
#endif

#ifdef __OS2__
#include <os2.h>
#define PRTYC_IDLE 1
#define PRTYC_REGULAR 2
#define PRTYD_MINIMUM -31
#define PRTYD_MAXIMUM 31
#endif

#if defined(_WIN32)
# include <windows.h>
#endif


/*
 main.c is example code for how to use libmp3lame.a.  To use this library,
 you only need the library and lame.h.  All other .h files are private
 to the library.
*/
#include "lame.h"

#include "console.h"
#include "lame_export.h"

/* PLL 14/04/2000 */
#if macintosh
#include <console.h>
#endif

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif


/************************************************************************
*
* main
*
* PURPOSE:  MPEG-1,2 Layer III encoder with GPSYCHO
* psychoacoustic model.
*
************************************************************************/


#if defined( _WIN32 ) && !defined(__MINGW32__)
static void
set_process_affinity()
{
#if 0
    /* rh 061207
       the following fix seems to be a workaround for a problem in the
       parent process calling LAME. It would be better to fix the broken
       application => code disabled.
     */
#if defined(_WIN32)
    /* set affinity back to all CPUs.  Fix for EAC/lame on SMP systems from
       "Todd Richmond" <todd.richmond@openwave.com> */
    typedef BOOL(WINAPI * SPAMFunc) (HANDLE, DWORD_PTR);
    SPAMFunc func;
    SYSTEM_INFO si;

    if ((func = (SPAMFunc) GetProcAddress(GetModuleHandleW(L"KERNEL32.DLL"),
                                          "SetProcessAffinityMask")) != NULL) {
        GetSystemInfo(&si);
        func(GetCurrentProcess(), si.dwActiveProcessorMask);
    }
#endif
#endif
}
#endif

#if defined(WIN32)

/**
 *  Long Filename support for the WIN32 platform
 *
 */

void
dosToLongFileName(char *fn)
{
    const size_t MSIZE = PATH_MAX + 1 - 4; /*  we wanna add ".mp3" later */
    WIN32_FIND_DATAA lpFindFileData;
    HANDLE  h = FindFirstFileA(fn, &lpFindFileData);
    if (h != INVALID_HANDLE_VALUE) {
        size_t  a;
        char   *q, *p;
        FindClose(h);
        for (a = 0; a < MSIZE; a++) {
            if ('\0' == lpFindFileData.cFileName[a])
                break;
        }
        if (a >= MSIZE || a == 0)
            return;
        q = strrchr(fn, '\\');
        p = strrchr(fn, '/');
        if (p - q > 0)
            q = p;
        if (q == NULL)
            q = strrchr(fn, ':');
        if (q == NULL)
            strncpy(fn, lpFindFileData.cFileName, a);
        else {
            a += q - fn + 1;
            if (a >= MSIZE)
                return;
            strncpy(++q, lpFindFileData.cFileName, MSIZE - a);
        }
    }
}

BOOL
SetPriorityClassMacro(DWORD p)
{
    HANDLE  op = GetCurrentProcess();
    return SetPriorityClass(op, p);
}

#endif


#if defined(__OS2__)
/* OS/2 priority functions */
static void
setProcessPriority(int Priority)
{
    int     rc;

    switch (Priority) {

    case 0:
        rc = DosSetPriority(0, /* Scope: only one process */
                            PRTYC_IDLE, /* select priority class (idle, regular, etc) */
                            0, /* set delta */
                            0); /* Assume current process */
        console_printf("==> Priority set to 0 (Low priority).\n");
        break;

    case 1:
        rc = DosSetPriority(0, /* Scope: only one process */
                            PRTYC_IDLE, /* select priority class (idle, regular, etc) */
                            PRTYD_MAXIMUM, /* set delta */
                            0); /* Assume current process */
        console_printf("==> Priority set to 1 (Medium priority).\n");
        break;

    case 2:
        rc = DosSetPriority(0, /* Scope: only one process */
                            PRTYC_REGULAR, /* select priority class (idle, regular, etc) */
                            PRTYD_MINIMUM, /* set delta */
                            0); /* Assume current process */
        console_printf("==> Priority set to 2 (Regular priority).\n");
        break;

    case 3:
        rc = DosSetPriority(0, /* Scope: only one process */
                            PRTYC_REGULAR, /* select priority class (idle, regular, etc) */
                            0, /* set delta */
                            0); /* Assume current process */
        console_printf("==> Priority set to 3 (High priority).\n");
        break;

    case 4:
        rc = DosSetPriority(0, /* Scope: only one process */
                            PRTYC_REGULAR, /* select priority class (idle, regular, etc) */
                            PRTYD_MAXIMUM, /* set delta */
                            0); /* Assume current process */
        console_printf("==> Priority set to 4 (Maximum priority). I hope you enjoy it :)\n");
        break;

    default:
        console_printf("==> Invalid priority specified! Assuming idle priority.\n");
    }
}
#endif


/***********************************************************************
*
*  Message Output
*
***********************************************************************/


#if defined( _WIN32 ) && !defined(__MINGW32__)
/* Idea for unicode support in LAME, work in progress
 * - map UTF-16 to UTF-8
 * - advantage, the rest can be kept unchanged (mostly)
 * - make sure, fprintf on console is in correct code page
 *   + normal text in source code is in ASCII anyway
 *   + ID3 tags and filenames coming from command line need attention
 * - call wfopen with UTF-16 names where needed
 *
 * why not wchar_t all the way?
 * well, that seems to be a big mess and not portable at all
 */
#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>  
#include <crtdbg.h>
#endif
#include <wchar.h>
#include <mbstring.h>

static wchar_t *mbsToUnicode(const char *mbstr, int code_page)
{
  int n = MultiByteToWideChar(code_page, 0, mbstr, -1, NULL, 0);
  wchar_t* wstr = malloc( n*sizeof(wstr[0]) );
  if ( wstr !=0 ) {
    n = MultiByteToWideChar(code_page, 0, mbstr, -1, wstr, n);
    if ( n==0 ) {
      free( wstr );
      wstr = 0;
    }
  }
  return wstr;
}

static char *unicodeToMbs(const wchar_t *wstr, int code_page)
{
  int n = 1+WideCharToMultiByte(code_page, 0, wstr, -1, 0, 0, 0, 0);
  char* mbstr = malloc( n*sizeof(mbstr[0]) );
  if ( mbstr !=0 ) {
    n = WideCharToMultiByte(code_page, 0, wstr, -1, mbstr, n, 0, 0);
    if( n == 0 ){
      free( mbstr );
      mbstr = 0;
    }
  }
  return mbstr;
}

char* mbsToMbs(const char* str, int cp_from, int cp_to)
{
  wchar_t* wstr = mbsToUnicode(str, cp_from);
  if ( wstr != 0 ) {
    char* local8bit = unicodeToMbs(wstr, cp_to);
    free( wstr );
    return local8bit;
  }
  return 0;
}

enum { cp_utf8, cp_console, cp_actual };

wchar_t *utf8ToUnicode(const char *mbstr)
{
  return mbsToUnicode(mbstr, CP_UTF8);
}

char *unicodeToUtf8(const wchar_t *wstr)
{
  return unicodeToMbs(wstr, CP_UTF8);
}

char* utf8ToLocal8Bit(const char* str)
{
  return mbsToMbs(str, CP_UTF8, CP_ACP);
}

char* utf8ToConsole8Bit(const char* str)
{
  return mbsToMbs(str, CP_UTF8, GetConsoleOutputCP());
}

char* local8BitToUtf8(const char* str)
{
  return mbsToMbs(str, CP_ACP, CP_UTF8);
}

char* console8BitToUtf8(const char* str)
{
  return mbsToMbs(str, GetConsoleOutputCP(), CP_UTF8);
}
 
char* utf8ToLatin1(char const* str)
{
  return mbsToMbs(str, CP_UTF8, 28591); /* Latin-1 is code page 28591 */
}

unsigned short* utf8ToUtf16(char const* mbstr) /* additional Byte-Order-Marker */
{
  int n = MultiByteToWideChar(CP_UTF8, 0, mbstr, -1, NULL, 0);
  wchar_t* wstr = malloc( (n+1)*sizeof(wstr[0]) );
  if ( wstr !=0 ) {
    wstr[0] = 0xfeff; /* BOM */
    n = MultiByteToWideChar(CP_UTF8, 0, mbstr, -1, wstr+1, n);
    if ( n==0 ) {
      free( wstr );
      wstr = 0;
    }
  }
  return wstr;
}

static
void setDebugMode()
{
#ifndef NDEBUG
    if ( IsDebuggerPresent() ) {
        // Get current flag  
        int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
        //tmpFlag |= _CRTDBG_DELAY_FREE_MEM_DF;  
        tmpFlag |= _CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF;
        // Set flag to the new value.  
        _CrtSetDbgFlag( tmpFlag );
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    }
#endif
}


FILE* lame_fopen(char const* file, char const* mode)
{
    FILE* fh = 0;
    wchar_t* wfile = utf8ToUnicode(file);
    wchar_t* wmode = utf8ToUnicode(mode);
    if (wfile != 0 && wmode != 0) {
        fh = _wfopen(wfile, wmode);
    }
    else {
        fh = fopen(file, mode);
    }
    free(wfile);
    free(wmode);
    return fh;
}

char* lame_getenv(char const* var)
{
    char* str = 0;
    wchar_t* wvar = utf8ToUnicode(var);
    if (wvar != 0) {
        wchar_t* wstr = _wgetenv(wvar);
        if (wstr != 0) {
            str = unicodeToUtf8(wstr);
        }
    }
    free(wvar);
    return str;
}

#endif

#include "lame_bj.h"

EXT_LAME_BJ int write_mp3(int length, double *buf_l, double *buf_r, int fs, const char *filename, char *errstr)
{
	lame_t  gf;
	gf = lame_init(); /* initialize libmp3lame */
	csignals_mp3_aiff x;
	x.fs = fs;
	x.buf_l = buf_l;
	x.buf_r = buf_r;
	x.length = length;
	int ret = lame_bj_encode(gf, filename, &x, errstr);
	lame_close(gf);
	return ret;
}

EXT_LAME_BJ int read_mp3(int *length, double *buf_l, double *buf_r, int *fs, const char *filename, char *errstr)
{
	lame_t  gf;
	gf = lame_init(); /* initialize libmp3lame */
	csignals_mp3_aiff x;
	lame_set_decode_only(gf, 1);
	x.buf_l = buf_l;
	x.buf_r = buf_r;
	int ret = lame_bj_decode(gf, filename, &x, errstr);
	lame_close(gf);
	if (!ret) return ret;
	*fs = x.fs;
	x.buf_r = NULL;
	*length = x.length;
	return x.length;
}


EXT_LAME_BJ int	read_mp3_header(const char *filename, int *nSamples, int *nChans, int *fs, char *errstr)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) return -1;
	int     enc_delay = 0, enc_padding = 0;
	int input_format = global_reader.input_format;
	global_reader.input_format = sf_mp123;
	int res = lame_decode_initfile(fp, &global_decoder.mp3input_data, &enc_delay, &enc_padding);
	*nChans = global_decoder.mp3input_data.stereo;
	*fs = global_decoder.mp3input_data.samplerate;
	*nSamples = global_decoder.mp3input_data.nsamp;
	fclose(fp);
	global_reader.input_format = input_format;
	return 0;
}

EXT_LAME_BJ int	read_aiff_header(const char *filename, int *nSamples, int *nChans, int *fs, char *errstr)
{
	lame_t  gf;
	gf = lame_init(); /* initialize libmp3lame */
	csignals_mp3_aiff x;
	int input_format = global_reader.input_format;
	global_reader.input_format = sf_aiff;
	init_infile(gf, filename, &x);
	*nChans = lame_get_num_channels(gf);
	*fs = lame_get_in_samplerate(gf);
	*nSamples = lame_get_num_samples(gf);
	global_reader.input_format = input_format;
	return 0;
}