#pragma once
#include "sigproc.h"
#include <complex>

class echo_object
{
public:
	echo_object() {
		offset = 0;
		type = 0;
	};
	virtual ~echo_object() {};
	int offset;
	string name;
	string postscript;
	uint16_t type;
	void print(const string& name, const CVar& obj, int offset);
	void header()
	{
		if (!name.empty())
		{
			for (int k = 0; k < offset; k++) cout << " ";
			cout << name << " = ";
		}
	};

};

class echo_cell : public echo_object
{
public:
	echo_cell(const string& _name, int _offset) {
		name = _name; offset = _offset;
	};
	virtual ~echo_cell() {};
	void print(const CVar& obj);
};

class echo_struct : public echo_object
{
public:
	echo_struct(const string& _name, int _offset) {
		name = _name; offset = _offset;
	};
	virtual ~echo_struct() {};
	void print(const CVar& obj);
};

class echo_object_null : public echo_object
{
public:
	echo_object_null(const string& _name, int _offset) {
		name = _name; offset = _offset;
	};
	virtual ~echo_object_null() {};
	void print(const CVar& obj)
	{
		echo_object::header();
		cout << "[]" << postscript << endl;
	};
};

class echo_object_string : public echo_object
{
public:
	echo_object_string(const string& _name, int _offset) {
		name = _name; offset = _offset;
	};
	virtual ~echo_object_string() {};
	void print(const CVar& obj)
	{
		echo_object::header();
		cout << "\"" << obj.string() << "\"" << postscript << endl;
	};
};

class echo_object_vector : public echo_object
{
public:
	echo_object_vector(const string& _name, int _offset) {
		name = _name; offset = _offset;
	};
	virtual ~echo_object_vector() {};
	void print(const CTimeSeries& obj);
	string row(const CTimeSeries& obj, unsigned int id0, int offset);
	string make(const CTimeSeries& obj);
};

class echo_object_time : public echo_object
{
public:
	echo_object_time() {};
	virtual ~echo_object_time() {};
	void print(const CVar& obj);
	static string tmarks(const CTimeSeries& obj, bool unit);
};

class echo_object_audio : public echo_object_time
{
public:
	echo_object_audio(const string& _name, int _offset) {
		name = _name; offset = _offset;
	};
	virtual ~echo_object_audio() {};
	void print(const CSignals& obj);
};

class echo_object_naudio : public echo_object_time
{
public:
	echo_object_naudio(const string& _name, int _offset) {
		name = _name; offset = _offset;
	};
	virtual ~echo_object_naudio() {};
	string make(const CTimeSeries& sig, bool unit, int offset);
	void print(const CSignals& obj);
};

class stream_for_echo
{
public:
	stream_for_echo() {};
	virtual ~stream_for_echo() {};
	string make(const CSignal& var, unsigned int id0, int offset);
	static string _complex(complex<double> cval);
};