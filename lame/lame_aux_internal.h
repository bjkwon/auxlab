#ifndef LAMEAUX_H_INCLUDED
#define LAMEAUX_H_INCLUDED

typedef struct {
	int fs;
	int length;
	int nChans;
	double *buf_l;
	double *buf_r;
} csignals;

#endif
/* end of lame_aux.h */
