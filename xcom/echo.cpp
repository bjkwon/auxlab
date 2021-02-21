#include "echo.h"

string echo_object_vector::make(const CTimeSeries& obj)
{
	ostringstream out;
	echo_object::header();
	if (obj.nGroups == 1) {
		if (obj.IsLogical()) out << "(bool) ";
		out << row(obj, 0, 0);
	}
	else
	{
		if (obj.IsLogical()) out << "(bool) ";
		out << endl;
		unsigned int j;
		for (j = 0; j < min(10, obj.nGroups); j++)
			out << row(obj, obj.Len() * j, offset + 1);
		if (j == 10)
		{
			for (int m = 0; m < offset; m++) out << " ";
			out << "... (total rows) = " << obj.nGroups << endl;
		}
	}
	return out.str();
}

void echo_object_vector::print(const CTimeSeries& obj)
{
	cout << make(obj);
}

string echo_object_vector::row(const CTimeSeries& obj, unsigned int id0, int offset)
{
	string out = stream_for_echo().make(obj, id0, offset);
	out += postscript;
	out += "\n";
	return out;
}

string stream_for_echo::_complex(complex<double> cval)
{
	ostringstream out;
	double im = imag(cval);
	if (cval == 0.) { out << "0"; return out.str(); }
	if (real(cval) != 0.)	out << real(cval);
	if (im == 0.) return out.str();
	if (!out.str().empty() && im > 0) out << "+";
	if (im != 1.)	out << im;
	out << (char)161;
	return out.str();
}

string echo_object_time::tmarks(const CTimeSeries& sig, bool unit)
{
	// unit is to be used in the future 8/15/2018
	// Get the timepoints
	ostringstream out;
	streamsize org_precision = out.precision();
	out.setf(ios::fixed);
	out.precision(1);
	int kk(0), tint(sig.CountChains());
	for (const CTimeSeries* xp = &sig; kk < tint; kk++, xp = xp->chain) {
		out << "(" << xp->tmark;
		if (unit) out << "ms";
		out << "~" << xp->tmark + 1000. * xp->nSamples / xp->GetFs();
		if (unit) out << "ms";
		out << ") ";
	}
	out << endl;
	out.unsetf(ios::floatfield);
	out.precision(org_precision);
	return out.str();
}

void echo_object_audio::print(const CSignals& obj)
{
	echo_object::header();
	cout << endl;
	for (int k = 0; k < offset + 1; k++) cout << " ";
	bool show_tpoints = true;
	if (obj.IsStereo())
	{
		cout << "audio(L) ";
		if (show_tpoints) cout << echo_object_time::tmarks(obj, true);
		else cout << ".." << endl;
		for (int k = 0; k < offset + 1; k++) cout << " ";
		cout << "audio(R) ";
		if (show_tpoints) cout << echo_object_time::tmarks(*(obj.next), true);
		else cout << ".." << endl;
	}
	else
	{
		cout << "audio ";
		if (show_tpoints)
			cout << echo_object_time::tmarks(obj, true);
		else cout << ".." << endl;
	}
}

void echo_object_naudio::print(const CSignals& obj)
{
	echo_object::header();
	cout << endl;
	if (obj.next) cout << "[L] " << endl;
	cout << make(obj, obj.GetFs() > 0, offset);
	if (obj.next)
	{
		cout << "[R] " << endl;
		cout << make(*(obj.next), obj.next->GetFs() > 0, offset);
	}
}

string echo_object_naudio::make(const CTimeSeries& sig, bool unit, int offset)
{
	ostringstream out;
	streamsize org_precision = out.precision();
	out.setf(ios::fixed);
	if (unit) out.precision(1);
	else out.precision(2);
	int kk(0), tint(sig.CountChains());
	for (const CTimeSeries* xp = &sig; kk < tint; kk++, xp = xp->chain)
	{
		for (int k = 0; k < offset + 1; k++) out << " ";
		out << "(" << xp->tmark;
		if (unit) out << "ms";
		out << ") ";
		out << echo_object_vector("", offset).make(*xp);
	}
	out.unsetf(ios::floatfield);
	out.precision(org_precision);
	return out.str();
}

string stream_for_echo::make(const CSignal& var, unsigned int id0, int offset)
{
	ostringstream out;
	streamsize org_precision = out.precision();
	//	out.setf(ios::fixed);
	//	out.precision(1);
	unsigned int k = 0;
	if (var.IsComplex())
		for (; k < min(10, var.Len()); k++, out << " ")
		{
			for (int m = 0; m < offset; m++) out << " ";
			out << _complex(var.cbuf[k + id0]);
		}
	else
	{
		for (int m = 0; m < offset; m++) out << " ";
		if (var.IsLogical())
		{
			for (; k < min(10, var.Len()); k++, out << " ")
				out << var.logbuf[k + id0];
		}
		else
			for (; k < min(10, var.Len()); k++, out << " ")
				out << var.buf[k + id0];
	}
	if (var.Len() > 10) // this means nSamples is greater than 10
		out << " ... (length = " << var.Len() << ")";
	//	out.unsetf(ios::floatfield);
	//	out.precision(org_precision);
	return out.str();
}

void echo_struct::print(const CVar& obj)
{
	echo_object::header();
	cout << "[Structure] ..." << endl;
	for (map<string, CVar>::const_iterator it = obj.strut.begin(); it != obj.strut.end(); it++)
	{
		ostringstream var0;
		var0 << '.' << it->first;
		echo_object::print(var0.str(), it->second, offset + 1);
	}
	for (map<string, vector<CVar*>>::const_iterator it = obj.struts.begin(); it != obj.struts.end(); it++)
	{
		ostringstream var0;
		var0 << '.' << it->first;
		if ((*it).second.empty())
			echo_object::print(var0.str(), CVar(), offset + 1);
		else
		{
			CVar temp;
			auto j = 1;
			for (vector<CVar*>::const_iterator jt = (*it).second.begin(); jt != (*it).second.end() && j < 10; jt++)
			{
				if (!CAstSig::HandleSig(&temp, *jt)) temp.SetString("(internal error)");
				echo_object::print(var0.str(), temp, offset + 1);// , " [Handle]");
			}
		}
	}
}

void echo_cell::print(const CVar& obj)
{
	echo_object::header();
	cout << "[Cell] ..." << endl;
	auto j = 1;
	for (vector<CVar>::const_iterator it = obj.cell.begin(); it != obj.cell.end(); it++)
	{
		ostringstream _varname;
		_varname << name << '{' << j++ << '}' ;
		echo_object::print(_varname.str(), *it, offset);
	}
}

void echo_object::print(const string& name, const CVar& obj, int offset)
{
	auto type = obj.type();
	//What about TYPEBIT_STRUTS?
	if (type & TYPEBIT_STRUT)
		echo_struct(name, offset).print(obj);
	else if (type & TYPEBIT_CELL)
		echo_cell(name, offset).print(obj);
	else if (obj.IsAudio())
		echo_object_audio(name, offset).print(obj);
	else if (type & TYPEBIT_TEMPORAL)
		echo_object_naudio(name, offset).print(obj);
	else if (!type) // Don't do (type & TYPEBIT_NULL) unless you want to be funny!
		echo_object_null(name, offset).print(obj);
	else if (obj.IsString())
		echo_object_string(name, offset).print(obj);
	else if (type & 2 || type & 1)
		echo_object_vector(name, offset).print(obj);
}