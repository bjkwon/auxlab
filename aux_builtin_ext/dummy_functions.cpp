#if defined(_WIN64) 
typedef __int64 INT_PTR;
#else 
typedef int INT_PTR;
#endif // _WIN64

#include <windows.h>
extern "C"
{
	int design_iir(double *num, double *den, int fs, int kind, int type, int n, double *freqs, double dbr, double dbd)
	{
		return 1;
	}
	typedef	struct SNDFILE_tag	SNDFILE;
	typedef __int64		sf_count_t;
	struct SF_INFO
	{
		sf_count_t	frames;
		int			samplerate;
		int			channels;
		int			format;
		int			sections;
		int			seekable;
	};

	SNDFILE* 	sf_open(const char *path, int mode, SF_INFO *sfinfo)
	{
		return nullptr;
	}
	sf_count_t	sf_writef_double(SNDFILE *sndfile, const double *ptr, sf_count_t frames)
	{
		return 0;
	}

	sf_count_t	sf_read_double(SNDFILE *sndfile, double *ptr, sf_count_t items)
	{
		return 0;
	}

	int		sf_close(SNDFILE *sndfile)
	{
		return 0;
	}

	typedef	 int	fftw_plan;
	typedef double fftw_complex;
	fftw_plan dummy;
	fftw_plan fftw_plan_dft_1d(int n, fftw_complex *in, fftw_complex *out, int sign, unsigned flags)
	{
		return 0;
	}
	fftw_plan fftw_plan_dft_r2c_1d(int fftsize, double* in, fftw_complex *out, int opt)
	{
		return 0;
	}
	fftw_plan fftw_plan_dft_c2r_1d(int fftsize, fftw_complex* in, double *out, int opt)
	{
		return 0;
	}
	void fftw_execute(fftw_plan p)
	{
	}
	void fftw_destroy_plan(fftw_plan p)
	{
	}
	void fftw_malloc(size_t n)
	{
	}
	void fftw_free(void* p)
	{
	}
}
bool StopPlay(INT_PTR pWavePlay, bool quick)
{
	return 0;
}
bool PauseResumePlay(INT_PTR pWavePlay, bool fOnOff)
{
	return 0;
}
HWND GetHWND_WAVPLAY()
{
	return nullptr;
}
INT_PTR PlayBufAsynch16(unsigned int DevID, short *dataBuffer, int length, int nChan, int fs, unsigned int userDefinedMsgID, HWND hApplWnd, int nProgReport, int loop, char* errstr)
{
	return 0;
}
INT_PTR QueuePlay(INT_PTR pWP, unsigned int DevID, short *dataBuffer, int length, int nChan, unsigned int userDefinedMsgID, int nProgReport, char *errstr, int loop)
{
	return 0;
}

#include "sigproc.h"

void _figure(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{}
void _axes(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{}
void _text(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{}
void _plot(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{}
void _line(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{}
void _delete_graffy(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{}
void _replicate(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{}
