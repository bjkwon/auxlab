#pragma once
#include "sigproc.h"
enum graffytype : char
{
	GRAFFY_no_graffy = '\0',
	GRAFFY_root = 'r',
	GRAFFY_figure = 'f',
	GRAFFY_axes = 'a',
	GRAFFY_axis = 'x',
	GRAFFY_text = 't',
	GRAFFY_patch = 'p',
	GRAFFY_line = 'l',
	GRAFFY_tick = 'k',
	GRAFFY_others = 'o',
};

enum LineStyle : unsigned _int8
{
	LineStyle_err = 255,
	LineStyle_noline = 0,
	LineStyle_solid,
	LineStyle_dash,
	LineStyle_dot,
	LineStyle_dashdot,
	LineStyle_dashdotdot,
};

graffytype GOtype(const CVar & obj);
