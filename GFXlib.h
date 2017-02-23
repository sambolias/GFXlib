//GFXlib.h
//Sam Erie
//2/22/17
//includes for my graphics library

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


