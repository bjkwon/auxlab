// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
// gengrids.cpp : Defines the entry point for the console application.
//

#include <vector>

std::vector<double> rule1(double in[], int digone[], int *digHandled = NULL);
std::vector<double> rule2(double in[], int digone[], int *digHandled = NULL);
std::vector<double> MakeGridEdges(double in[2], int *dighand = NULL);

double granularize(double in)
{
	while (in >= 7.5)
	{
		return granularize(in/10.) * 10.;
	}
	while (in <= .5)
	{
		return granularize(in * 10) / 10.;
	}
	int id = 0;
	double tp[] = {1,2,2.5,5,};
	std::vector<double> gran(tp, tp+sizeof(tp)/sizeof(double));
	std::vector<double>::iterator it = gran.begin();
	for (it = gran.begin(); ;)
	{
		if (*it == in) return in;
		if (*it>in) break;
		it++;
		if (it == gran.end())
			return gran.back();
	}
	double up = *it - in;
	double down = in;
	if (down>up)
		return *it;
	else
		return *(it - 1);
}

int getdigitone(double x)
{
	if (x == 0)
		return 0;
	if (x<0)
		return -getdigitone(-x);
	else
		return (int)(x / pow(10., (int)floor(log10(x))));
}

double tenpower(int n)
{
	int out(1);
	if (n == 0) return 1;
	else if (n<0) return 1 / tenpower(-n);
	else
	{
		while (n-->0)
			out *= 10;
		return out;
	}
}

double getmag(double x)
{
	double xx = log10(fabs(x));
	int ss = (int)floor(xx);
	double num = tenpower(ss);
	return getdigitone(x)*num;
}

int digitcnt(double x)
{ // digit count less one; characteristic of common log
	if (x == 0)
		return 0;
	if (x<0)
		return digitcnt(-x);
	else
		return (int)floor(log10(x));
}

double ceiln(double x, int n)
{
	double mult = tenpower(n);
	return ceil(x / mult) * mult;
}

double fixn(double x, int n)
{
	double mult = tenpower(n);
	return floor(x / mult) * mult;
}

std::vector<double> rule1(double in[], int digone[], int *digHandled)
{
	std::vector<double> out, out2;
	double less[2];
	memcpy(less, in, sizeof(less));
	while (digone[1] == digone[0])
	{
		for (int k = 0; k<2; k++)
		{
			less[k] -= getmag(less[k]);// strip the first digit
			digone[k] = getdigitone(less[k]);
		}
		//at this point, if less is different digit, less[0] should be zero---if all numbers are positive
		if (digitcnt(less[0])<digitcnt(less[1]))
		{	digone[0] = 0; break;	}
		else if (digitcnt(less[0])>digitcnt(less[1]))
		{	digone[1] = 0; break;	}
	}
	//at this point digone is different
	//what's left in less determines the ceiling/fixing position
	if (digitcnt(less[0]) != digitcnt(less[1]) && less[0] != in[0])
	{ // to handle in of 7067 vs 7912, less at this point should be 67 vs 912 ==> different digit count
	  // that means it has gone through trimming in the above while loop, and if rule1 was called with different digitone, 
	  // it shouldn't come in here. that's why we have less[0] != in[0]      5/7/2018
		out2 = MakeGridEdges(less);
		for (int k = 0; k<2; k++)
			out.push_back(getmag(in[k]) + out2[k]);
	}
	else
	{
		// if digit count of less are different (i.e., when rule1 was called with different digitone AND digit count of less[0] is one less than less[1]
		// digit to trim should follow that of less[0]. Even if it was set with less[1], it will be decremented in the do while loop here. 5/7/2018
		int dig2trim = digitcnt(less[0]);
		bool dig2Specified(false);
		int dig2trimThis;
		if (digHandled) dig2Specified = true, dig2trimThis = *digHandled;
		bool loop(true);
		do {
			if (dig2Specified)
				if (dig2trimThis > dig2trim)
				{	dig2trim = dig2trimThis; loop = false;	}
				else
					*digHandled = dig2trim;
			out.clear();
			out.push_back(ceiln(in[0], dig2trim));
			out.push_back(fixn(in[1], dig2trim));
			dig2trim--;
		} while (out.back() == out.front()); // if in[0] and in[1] are too close, dig2trim needs to go down.
	}
	return out;
}

std::vector<double> rule2(double in[], int digone[], int *digHandled)
{// condition: both elements of in must be the same sign.
	std::vector<double> out;
	double less, res;
	int dig2trim;
	if (in[0] + in[1]>0)
	{
		out.push_back(0);
		less = in[1];
		if (in[0]!=0 && in[1] - in[0]<tenpower(digitcnt(in[1])) * 3)
			less -= getmag(in[1]);
		dig2trim = digitcnt(less);
		res = fixn(in[1], dig2trim);
		out.push_back(res);
	}
	else
	{
		out.push_back(0);
		less = in[0];
		if (in[1] - in[0]<tenpower(digitcnt(in[0])) * 3)
			less -= getmag(in[0]); // check this
		dig2trim = digitcnt(less);
		res = fixn(in[0], dig2trim);
		out.insert(out.begin(), res);
	}
	if (digHandled) *digHandled = dig2trim;
	return out;
}

std::vector<double> MakeGridEdges(double in[2], int *dighand)
{
	int digcount[2]; // characteristic (integer part of the common log)
	int digone[2];
	std::vector<double> out;
	for (int k = 0; k<2; k++)
	{
		digcount[k] = digitcnt(in[k]);
		digone[k] = getdigitone(in[k]);
	}
	if (in[1] * in[0] == 0.) // return value of digitcnt(0) is undefined and not used because it goes directly to rule2
		out = rule2(in, digone, dighand);
	else if (in[0] * in[1]<0) // different sign
	{
		double in2[2];
		double val1, val2;
		int digh1, digh2, digh;
		in2[0] = in[0];
		in2[1] = 0;
		out = MakeGridEdges(in2, &digh1);
		val1 = out.front();
		in2[0] = 0;
		in2[1] = in[1];
		out = MakeGridEdges(in2, &digh2);
		val2 = out.back();
		if (digh1<digh2)
		{
			if (val1<0)	val1 = fixn(val1, digh2);
			else	val1 = ceiln(val1, digh2);
			digh = digh2;
		}
		else if (digh1>digh2)
		{
			if (val2<0)	val2 = ceiln(val2, digh1);
			else	val2 = fixn(val2, digh1);
			digh = digh1;
		}
		else
			digh = digh1;
		out.clear();
		out.push_back(val1);
		out.push_back(val2);
		if (dighand) *dighand = digh;
	}
	else if (digcount[1] == digcount[0])
		out = rule1(in, digone, dighand);
	else if (digcount[1] - digcount[0]>1)
		out = rule2(in, digone, dighand);
	else  // when the difference is one
	{
		double ratio = in[1] / in[0];
		if (ratio<1) ratio = 1. / ratio;
		if (ratio>5.)
			out = rule2(in, digone, dighand);
		else
			out = rule1(in, digone, dighand);
	}
	return out;
}

std::vector<double> makeseq(double lim[], double anch, double step)
{
	std::vector<double> out;
	double p = anch;
	do {
		out.push_back(p);
		p += step;
	} while (p <= lim[1]);
	p = out.front() - step;
	while (p>lim[0])
	{
		out.insert(out.begin(), p);
		p -= step;
	}
	return out;
}
 
std::vector<double> gengrids(double lim[], int nDv, int *pdigh)
{ // nDv positive means strict division count; tenpow and width ignored
// nDv negative means loose division count; based on the grid edges from MakeGridEdges(), it tries to divide into nDv segments, 
// but the priority is given that the step size is granularized by 10%, 20%, 25% or 50% of grid edge width; therefore,
// it may not end up exactly nDv segments, but it should be close enough with one of those available steps
	std::vector<double> out, grids;
	bool dig2Specified(false);
	int dig2trimThis;
	if (pdigh) dig2Specified = true, dig2trimThis = *pdigh;
	grids = MakeGridEdges(lim, pdigh);
	double delta(grids.back() - grids.front());
	double delta0(lim[1] - lim[0]);
	double step(delta / nDv);
	double step0(delta0 / nDv);
	int copynDV(nDv);
	if (nDv < 0)
	{
		nDv *= -1;
		step0 *= -1;
		step *= -1;
		if (dig2Specified)
		{
			if (tenpower(dig2trimThis) > step)
				step = tenpower(dig2trimThis);
			else
				step = granularize(step);
		}
		else
			step = granularize(step);
	}
	out = makeseq(lim, grids.front(), step);
	while ((double)out.size() / nDv > 1.5)
	{
		if (copynDV < 0)
		{
			if (dig2Specified)
			{
				if (tenpower(dig2trimThis) > step)
					step0 = tenpower(dig2trimThis);
				else
					step0 = granularize(step0);
			}
			else
				step0 = granularize(step0);
		}
		out = makeseq(lim, grids.front(), step0);
	}
	return out;
}