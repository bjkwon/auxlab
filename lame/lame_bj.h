//#ifdef LAME_BJ_STATIC
//#define EXT_LAME_BJ extern "C" 
//#elifdef LAME_BJ_INTERNAL
//#define EXT_LAME_BJ __declspec (dllexport) 
//#else
//#define EXT_LAME_BJ extern "C" __declspec (dllimport) 
//#endif
#ifdef __cplusplus
#define EXT_LAME_BJ extern "C" 
#else
#define EXT_LAME_BJ 
#endif 
EXT_LAME_BJ int write_mp3(int length, double *buf_l, double *buf_r, int fs, const char *filename, char *errstr);
EXT_LAME_BJ int read_mp3(int *length, double *buf_l, double *buf_r, int *fs, const char *filename, char *errstr);
EXT_LAME_BJ int	read_mp3_header(const char *filename, int *nSamples, int *nChans, int *fs, char *errstr);
EXT_LAME_BJ int	read_aiff_header(const char *filename, int *nSamples, int *nChans, int *fs, char *errstr);