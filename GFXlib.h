// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//saving for xplatform uses
#include "targetver.h"

//std lib includes
#include <stdio.h>
#include <iostream>
#include <string>
using std::string;
#include <functional>
using std::function;

//OpenGL includes - see docs for licensing
#include <GL\glew.h>
#include <GL\freeglut.h>

//for loading textures - see docs for licensing
#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"



// TODO: reference additional headers your program requires here
