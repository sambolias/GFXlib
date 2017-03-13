//GFXlib.h
//Sam Erie
//2/22/17
//includes for my graphics library

#pragma once

//my includes
#include "GFXdisplay.h"

class GFX
{
public:
	
	GFX() { test = new display("test"); }
	display *test;

};


