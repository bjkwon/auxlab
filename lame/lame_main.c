/*
 *      Command line frontend program
 *
 *      Copyright (c) 1999 Mark Taylor
 *                    2000 Takehiro TOMINAGA
 *                    2010-2017 Robert Hegemann
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

/* $Id: lame_main.c,v 1.18 2017/08/31 14:14:46 robert Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "lame_aux_internal.h"

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
#include "parse.h"
#include "lame_export.h"
#include "get_audio.h"
#include "timestatus.h"

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


static FILE *
init_files(lame_global_flags * gf, char const *inPath, char const *outPath, csignals *px)
{
    FILE   *outf;

    if (init_infile(gf, inPath, px) < 0) {
    // //   error_printf("Can't init infile '%s'\n", inPath);
    //    return NULL;
    }
    if ((outf = init_outfile(outPath, lame_get_decode_only(gf), px)) == NULL) {
      //  error_printf("Can't init outfile '%s'\n", outPath);
        return NULL;
    }
    return outf;
}



/* the simple lame decoder */

static int
lame_decoder_loop(lame_t gfp, FILE * outf, double *buf_l, double *buf_r)
{
    short int Buffer[2][1152];
    int     iread;
    unsigned long int  wavsize;
    int     tmp_num_channels = lame_get_num_channels(gfp);
    int     skip_start = samples_to_skip_at_start();
    int     skip_end = samples_to_skip_at_end();
    DecoderProgress dp = 0;

    if (global_ui_config.silent < 9) {
         switch (global_reader.input_format) {
        case sf_mp3:
        case sf_mp2:
        case sf_mp1:
            dp = decoder_progress_init(lame_get_num_samples(gfp),
                                       global_decoder.mp3input_data.framesize);
            break;
        case sf_raw:
        case sf_wave:
        case sf_aiff:
        default:
            dp = decoder_progress_init(lame_get_num_samples(gfp),
                                       lame_get_in_samplerate(gfp) < 32000 ? 576 : 1152);
            break;
        }
    }
	wavsize = 0;
    do {
        iread = get_audio16(gfp, Buffer); /* read in 'iread' samples */
        if (iread >= 0) {
			if (dp != 0) {
                decoder_progress(dp, &global_decoder.mp3input_data, iread);
            }
			put_audio_to_buf(Buffer, iread, buf_l + wavsize, buf_r ? buf_r + wavsize : NULL, tmp_num_channels);
			wavsize += iread;
		}
    } while (iread > 0);
    if (dp != 0)
        decoder_progress_finish(dp);
    return 0;
}

static int
lame_decoder(lame_t gfp, FILE * outf, double *buf_l, double *buf_r)
{
    int     ret;

	ret = lame_decoder_loop(gfp, outf, buf_l, buf_r);
    close_infile();     /* close the input file */
	return ret == 0 ? 1 : 0;
}

static int
write_xing_frame(lame_global_flags * gf, FILE * outf, size_t offset)
{
    unsigned char mp3buffer[LAME_MAXMP3BUFFER];
    size_t  imp3, owrite;

    imp3 = lame_get_lametag_frame(gf, mp3buffer, sizeof(mp3buffer));
    if (imp3 <= 0) {
        return 0;       /* nothing to do */
    }
    if (global_ui_config.silent <= 0) {
//        console_printf("Writing LAME Tag...");
    }
    if (imp3 > sizeof(mp3buffer)) {
        //error_printf
        //    ("Error writing LAME-tag frame: buffer too small: buffer size=%d  frame size=%d\n",
        //     sizeof(mp3buffer), imp3);
        return -1;
    }
    assert( offset <= LONG_MAX );
    if (fseek(outf, (long) offset, SEEK_SET) != 0) {
      //  error_printf("fatal error: can't update LAME-tag frame!\n");
        return -1;
    }
    owrite = fwrite(mp3buffer, 1, imp3, outf);
    if (owrite != imp3) {
       // error_printf("Error writing LAME-tag \n");
        return -1;
    }
    if (global_ui_config.silent <= 0) {
     //   console_printf("done\n");
    }
    assert( imp3 <= INT_MAX );
    return (int) imp3;
}


static int
write_id3v1_tag(lame_t gf, FILE * outf)
{
    unsigned char mp3buffer[128];
    size_t  imp3, owrite;

    imp3 = lame_get_id3v1_tag(gf, mp3buffer, sizeof(mp3buffer));
    if (imp3 <= 0) {
        return 0;
    }
    if (imp3 > sizeof(mp3buffer)) {
        //error_printf("Error writing ID3v1 tag: buffer too small: buffer size=%d  ID3v1 size=%d\n",
        //             sizeof(mp3buffer), imp3);
        return 0;       /* not critical */
    }
    owrite = fwrite(mp3buffer, 1, imp3, outf);
    if (owrite != imp3) {
//        error_printf("Error writing ID3v1 tag \n");
        return 1;
    }
    return 0;
}

#include <math.h>
static void
d2hei_array(const double *src, void *dest, int count, int normalize)
{
	unsigned char	*ucptr;
	short				value;
	double			normfact;

	normfact = normalize ? (1.0 * 0x7FFF) : 1.0;
	ucptr = ((unsigned char*)dest) + 4 * count;

	while (--count >= 0)
	{
		ucptr -= 4;
		value = (short)lrint(src[count] * normfact);
		ucptr[0] = 0;
		ucptr[1] = 0;
		ucptr[2] = (unsigned char)value ;
		ucptr[3] = value >> 8;
	};
} /* d2bei_array */

static void
d2bei_array(const double *src, int *dest, int count, int normalize)
{
	unsigned char	*ucptr;
	int				value;
	double			normfact;

	normfact = normalize ? (1.0 * 0x7FFFFFFF) : 1.0;
	ucptr = ((unsigned char*)dest) + 4 * count;

	while (--count >= 0)
	{
		ucptr -= 4;
		value = lrint(src[count] * normfact);
		ucptr[0] = value >> 24;
		ucptr[1] = value >> 16;
		ucptr[2] = value >> 8;
		ucptr[3] = value;
	};
} /* d2bei_array */

static int
lame_encoder_loop(lame_global_flags * gf, FILE * outf, int nogap, char *inPath, char *outPath, csignals *px, char *errstr)
{
	unsigned char mp3buffer[LAME_MAXMP3BUFFER] = {0};
    int     Buffer[2][1152];
    int     iread, imp3, owrite, in_limit=0;
    size_t  id3v2_size;

 //   encoder_progress_begin(gf, inPath, outPath);

    id3v2_size = lame_get_id3v2_tag(gf, 0, 0);
    if (id3v2_size > 0) {
        unsigned char *id3v2tag = malloc(id3v2_size);
        if (id3v2tag != 0) {
            size_t  n_bytes = lame_get_id3v2_tag(gf, id3v2tag, id3v2_size);
            size_t  written = fwrite(id3v2tag, 1, n_bytes, outf);
            free(id3v2tag);
            if (written != n_bytes) {
                encoder_progress_end(gf);
                strcpy(errstr, "Error writing ID3v2 tag");
                return 1;
            }
        }
    }
    else {
        unsigned char* id3v2tag = getOldTag(gf);
        id3v2_size = sizeOfOldTag(gf);
        if ( id3v2_size > 0 ) {
            size_t owrite = fwrite(id3v2tag, 1, id3v2_size, outf);
            if (owrite != id3v2_size) {
                encoder_progress_end(gf);
				strcpy(errstr, "Error writing ID3v2 tag");
                return 1;
            }
        }
    }
    if (global_writer.flush_write == 1) {
        fflush(outf);
    }

    /* do not feed more than in_limit PCM samples in one encode call
       otherwise the mp3buffer is likely too small
     */
    in_limit = lame_get_maximum_number_of_samples(gf, sizeof(mp3buffer));
    if (in_limit < 1)
        in_limit = 1;

    /* encode until we hit eof */
	int cum = 0;
	iread = min(1152, px->length - cum);
	do {
        /* read in 'iread' samples */
		d2hei_array(px->buf_l + cum, (void*)Buffer, iread, 1);
		if (px->buf_r)
			d2hei_array(px->buf_r+cum, (void*)&Buffer[1], iread, 1);
		cum += iread;
        if (iread >= 0) {
            const int* buffer_l = Buffer[0];
            const int* buffer_r = Buffer[1];
            int     rest = iread;
            do {
                int const chunk = rest < in_limit ? rest : in_limit;
                encoder_progress(gf);

                /* encode */
                imp3 = lame_encode_buffer_int(gf, buffer_l, buffer_r, chunk,
                                              mp3buffer, sizeof(mp3buffer));
                buffer_l += chunk;
                buffer_r += chunk;
                rest -= chunk;

                /* was our output buffer big enough? */
                if (imp3 < 0) {
                    if (imp3 == -1)
						strcpy(errstr, "mp3 buffer is not big enough...");
                    else
						sprintf(errstr, "mp3 internal error:  error code=%i\n", imp3);
                    return 1;
                }
                owrite = (int) fwrite(mp3buffer, 1, imp3, outf);
                if (owrite != imp3) {
					strcpy(errstr, "Error writing mp3 output");
                    return 1;
                }
            } while (rest > 0);
        }
        if (global_writer.flush_write == 1) {
            fflush(outf);
        }
		iread = min(1152, px->length - cum);
    } while (iread > 0);

    if (nogap)
        imp3 = lame_encode_flush_nogap(gf, mp3buffer, sizeof(mp3buffer)); /* may return one more mp3 frame */
    else
        imp3 = lame_encode_flush(gf, mp3buffer, sizeof(mp3buffer)); /* may return one more mp3 frame */

    if (imp3 < 0) {
        if (imp3 == -1)
			strcpy(errstr, "mp3 buffer is not big enough...");
        else
            sprintf(errstr, "mp3 internal error:  error code=%i", imp3);
        return 1;
    }
    encoder_progress_end(gf);
    owrite = (int) fwrite(mp3buffer, 1, imp3, outf);
    if (owrite != imp3) {
		strcpy(errstr, "Error writing mp3 output");
        return 1;
    }
    if (global_writer.flush_write == 1) {
        fflush(outf);
    }
    imp3 = write_id3v1_tag(gf, outf);
    if (global_writer.flush_write == 1) {
        fflush(outf);
    }
    if (imp3) {
		strcpy(errstr, "Error writing ID3v1 tag");
        return 1;
    }
    write_xing_frame(gf, outf, id3v2_size);
    if (global_writer.flush_write == 1) {
        fflush(outf);
    }
    return 0;
}

static int
lame_encoder(lame_global_flags * gf, FILE * outf, int nogap, char *inPath, char *outPath, csignals *px, char *errstr)
{
    int     ret;

    ret = lame_encoder_loop(gf, outf, nogap, inPath, outPath, px, errstr);
    fclose(outf);       /* close the output file */
    close_infile();     /* close the input file */
    return ret==0?1:0;
}

int lame_bj_encode(lame_t gf, int argc, char **argv, csignals *px, char *errstr)
{
	char    inPath[PATH_MAX + 1] = { 0 };
    char    outPath[PATH_MAX + 1];
    FILE   *outf = NULL;

	lame_set_msgf(gf, NULL);
	lame_set_errorf(gf, NULL);
	lame_set_debugf(gf, NULL);

	strcpy(outPath, argv[argc-1]);

	lame_set_VBR(gf, vbr_default);
	lame_set_VBR_quality(gf, (float)2.);
	lame_set_findReplayGain(gf, 1);

    if (global_ui_config.update_interval < 0.)
        global_ui_config.update_interval = 2.;

	strcpy(inPath, "");
	outf = fopen(outPath, "w+b");
    if (outf == NULL) 
	{
		strcpy(errstr, "Cannot open the file for writing");
		return 0;
	}
	if (lame_set_num_channels(gf, 1+(px->buf_r?1:0)))
		return 0;
	if (lame_set_in_samplerate(gf, px->fs))
		return 0;
	if (lame_set_num_samples(gf,px->length))
		return 0;
	lame_set_write_id3tag_automatic(gf, 0);
    if (lame_init_params(gf)) 
	{
		strcpy(errstr, "fatal error during initializing LAME");
		fclose(outf);
		return 0;
	}
    return lame_encoder(gf, outf, 0, inPath, outPath, px, errstr);
}

int lame_bj_decode(lame_t gf, const char *filename, csignals *px, char *errstr)
{
	char    inPath[PATH_MAX + 1];
	char    outPath[PATH_MAX + 1] = { 0 };
	int     ret;
	FILE   *outf = NULL;

	lame_set_msgf(gf, NULL);
	lame_set_errorf(gf, NULL);
	lame_set_debugf(gf, NULL);

	strcpy(inPath, filename);
	ret = parse_args(gf, filename, outPath);
	if (ret < 0) {
		return ret == -2 ? 0 : 1;
	}
	if (global_ui_config.update_interval < 0.)
		global_ui_config.update_interval = 2.;
	
	if (init_infile(gf, inPath, px) < 0)
	{
		strcpy(errstr, "File reading error (or invalid format)");
		return 0;
	}
	lame_set_write_id3tag_automatic(gf, 0);
	if (lame_init_params(gf))
	{
		strcpy(errstr, "fatal error during initializing LAME");
		close_infile();
		return 0;
	}
	ret = lame_decoder(gf, outf, px->buf_l, px->buf_r);
	px->fs = lame_get_in_samplerate(gf);
	px->length = lame_get_num_samples(gf);
	return ret;
}
