#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

int     usage(FILE * const fp, const char *ProgramName);
int     short_help(const lame_global_flags * gfp, FILE * const fp, const char *ProgramName);
int     long_help(const lame_global_flags * gfp, FILE * const fp, const char *ProgramName,
                  int lessmode);
int     display_bitrates(FILE * const fp);

int     parse_args(lame_t gfp, const char *const inPath, const char *const outPath);

void    parse_close();

#if defined(__cplusplus)
}
#endif

#endif
/* end of parse.h */
