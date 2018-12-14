#include <windows.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

using namespace std;

void trimvector(vector<double> &vec, double step, double mantastep)
{
	double gap = vec[1]-vec[0];
	while (gap < step/5.) 
	{
		vec.erase(vec.begin()+1);
		gap = vec[1]-vec[0];
	}
	gap = *(vec.end()-1)-*(vec.end()-2);
	while (gap < step/5.) 
	{
		vec.erase(vec.end()-2);
		gap = *(vec.end()-1)-*(vec.end()-2);
	}
	for (vector<double>::iterator it=vec.begin()+1; it!=vec.end()-1; it++ )
	{
		// To clean out ugly decimal point tails, like---- 0.1 0.2 0.3 0.4 0.499999999999999999999 0.6 
		*it /= (mantastep/100);
		double dd=*it;
		double dd1((int)(*it+.5)), dd2((int)(*it-.5));
		if (*it>0)	*it = (int)(*it+.5);
		else if (*it<0)	*it = (int)(*it-.5);
		*it *= (mantastep/100);
	}
	double gappy = gap*2;


}

vector<double> makefixtick(double x1, double x2, int nDivRequested)
{
	vector<double> out;
	double dstep = (x2-x1) / nDivRequested; 
	double manta1 = pow(10., ceil(log10(x1))-1);
	double manta2 = pow(10., ceil(log10(x2))-1);
	double mantastep = pow(10., ceil(log10(dstep)));
	double step;
	if (x1<0)
	{ // For negative _x1, add abs(_x1) to the first and last values. Why? To make the first value positive.
	  // Leave middle values alone. Otherwise, all values would be adjusted by non-grid. 
		manta1 = pow(10., ceil(log10(-x1))-1);
		out = makefixtick(-x1, x2-2*x1, nDivRequested);
		// Assumethat the size of out >= 3
		step = (out.size()==3) ? (out.back()-out.front())/(out.size()-1) : out[2]-out[1]; 
		out[0] += 2*x1;
		out[out.size()-1] += 2*x1;
		if (manta1!=manta2)
		{
			double step_edge = out[1]-out[0];
			while (step_edge>step)
			{ // Make the first step equal or less than usual step
				for (vector<double>::iterator it=out.begin()+1; it!=out.end()-1; it++)
					(*it) -= step;
				step_edge = out[1]-out[0];
			}
			step_edge = *(out.end()-1)-*(out.end()-2);
			while (step_edge>step)
			{ // If the above while made the last step greater than equal, insert new values as needed
				out.insert(out.end()-1, *(out.end()-2)+step);
				step_edge = *(out.end()-1)-*(out.end()-2);
			}
		}
		else
		{
			for (vector<double>::iterator it=out.begin()+1; it!=out.end()-1; it++)
				*it += 2*x1;
		}
		trimvector(out, step, mantastep);
		return out;
	}
	else
	{
		int firstdigit1 = (int)(x1 / manta1);
		int firstdigit2 = (int)(x2 / manta2);
		double	xlim0 = firstdigit1*manta1;
		double	xlim1 = firstdigit2*manta2;
//		step = ((int)(dstep*60/mantastep+.5))/60.*mantastep;
		step = manta2;
		if (manta1==manta2) 
			for (int k(0); xlim0 + step*k<x2; k++) out.push_back(xlim0 + step*k);
		else
		{
			out.push_back(xlim0); 
			for (int k(1); step*k<x2; k++) out.push_back(step*k);
		}
		if (out.size() < nDivRequested * 2 / 3)
		{
			step /= 2;
			out.clear();
			out.push_back(xlim0);
			for (int k(1); step*k<x2; k++) out.push_back(step*k);
		}
		for (size_t k(0); k<out.size(); k++)
		{
			if (x1-out[k] > step/3) // if _x1 is greater than k-th generated value by more than 33% of step, major adjustment is needed (get rid of out of range value and add new one)
			{
				out.erase(out.begin()+k);
				out.push_back(out.back()+step);
				k--;
			}
			else
				break;
		}
		for (vector<double>::iterator it=out.begin(); ; it=out.begin())
		{
			if (*it < x1)	out.erase(it);
			else			break;
		} 
//		out.insert(out.begin(), x1);
//		out.push_back(x2);
		trimvector(out, step, mantastep);
		return out;
	}
}

void makefixtick_printout(FILE *fp, double a1, double a2, int count)
{
	vector<double> tick;
	tick = makefixtick(a1, a2, count);
	fprintf(fp, "makefixtick(%g, %g, %d) generates %d values with step=%g\n", a1, a2, count, tick.size(), tick[2]-tick[1]);
	int k=0;
	for (vector<double>::iterator it=tick.begin(); it!=tick.end(); it++, k++)
		fprintf(fp, "%d\t%g\n", k, *it);
}
/*
vector<double> CAxis::makefixtick(char xy, int nDivRequested)
{
	vector<double> out;
	double a1, a2, manta1(0), manta2(0);
	int sgn[2] = { 1, 1 };
	if (xy == 'x')
	{
		a1 = xlim[0];		a2 = xlim[1];
	}
	else if (xy == 'y')
	{
		a1 = ylim[0];		a2 = ylim[1];
	}
	if (a1 < 0) { sgn[0] = -1; a1 *= -1; }
	else if (a1 == 0) sgn[0] = 0;
	if (a2 < 0) { sgn[1] = -1; a2 *= -1; }
	else if (a2 == 0) sgn[1] = 0;

	if (a1 != 0)	manta1 = pow(10., ceil(log10(a1)) - 1);
	if (a2 != 0)	manta2 = pow(10., ceil(log10(a2)) - 1);
	a1 *= sgn[0];
	a2 *= sgn[1];
	double step = manta2;
	int firstdigit1 = (int)(a1 / manta1);
	int firstdigit2 = (int)(a2 / manta2);
	double	xlim0 = firstdigit1*manta1;
	double	xlim1 = firstdigit2*manta2;
	bool symm(false);
	double nDiv(nDivRequested);
	if (nDiv < 6)
		step = xlim1 / (int)(nDiv + .5);
	else
		step = xlim1 / nDiv;
	if (xlim0 + xlim1 == 0)
	{
		symm = true;
		xlim0 = 0;
		nDiv /= 2;
	}
	out.push_back(xlim0);
	while (out.back()<xlim1)
		out.push_back(out.back() + step);
	if (out.back() > xlim1) out.pop_back();
	// check if the number of grid based on initial step is OK
	while (out.size() < .67 * nDiv || out.size() >= 2 * nDiv)
	{
		if (out.size() < .67 * nDiv)
			step /= 2;
		else
			step *= 2;
		out.clear();
		out.push_back(xlim0);
		while (out.back()<xlim1)
			out.push_back(out.back() + step);
	}
	if (symm)
	{
		vector<double> tp;
		for (vector<double>::iterator it = out.begin() + 1; it != out.end(); it++)
			tp.insert(tp.begin(), -1.**it);
		for (vector<double>::iterator it = out.begin(); it != out.end(); it++)
			tp.push_back(*it);
		out = tp;
	}
	for (size_t k(0); k<out.size(); k++)
	{
		if (a1 - out[k] > step / 3) // if _x1 is greater than k-th generated value by more than 33% of step, major adjustment is needed (get rid of out of range value and add new one)
		{
			out.erase(out.begin() + k);
			out.push_back(out.back() + step);
			k--;
		}
		else
			break;
	}
	for (vector<double>::iterator it = out.begin(); ; it = out.begin())
	{
		if (*it < a1)	out.erase(it);
		else			break;
	}
	//	trimvector(out, step, mantastep);
	return out;
}*/