using namespace std;

#include <string>
#include <complex>
#include <vector>

struct auxdata
{

};

class carray
{
public:
	union {
		vector<double> * pdata;
		vector<complex<double>> * pcbuf;
		vector<bool> * plogbuf;
		string * pstrbuf;
	};
	carray();
//	carray(const carray&);
	carray(double x, int len) { 
		type = real; 
		double *pv = new double[len];
		pdata = &vector<double>(pv, pv + len);
	};
	carray(complex<double> x, int len) { 
		type = complex; 
		//do something
	};
	carray(const char *strIn) {
		type = text; pstrbuf = new string;  *pstrbuf = strIn;
	};
	carray(char x, int len) { 
		type = logical; plogbuf = new vector<bool>;  plogbuf->reserve(len);
	};
	 ~carray();
	enum
	{
		real,
		complex,
		text,
		logical,
	}  type;
};

class caudio : public carray
{ // a single channel, single audio block, 
public:
	int fs;
	double tmark;
	caudio();
	caudio(int _fs);
//	caudio(const caudio&copy);
	caudio(int _fs, int len);
	caudio & tone(double freq, double dur, double beginPhase=0);
	~caudio();
};

class ctimesig : public caudio
{
public:
//	ctimesig *chain;
	enum
	{
		audio,
		non_audio,
	} typeTimeSig;
	ctimesig & makechainless();
	ctimesig(int _fs);
	~ctimesig();
};

class cvarsimple : public ctimesig
{
public:
	string name;
};

class cvar 
{
public:
	vector<cvarsimple> member;
};

class ccell
{
public:
	vector<cvar> member;
};

