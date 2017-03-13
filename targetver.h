#pragma once

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>


//std lib includes
#include <stdio.h>
#include <iostream>
#include <string>
using std::string;
#include <functional>
using std::function;
#include <algorithm>
using std::copy;

//OpenGL includes - see docs for licensing
#include <GL\glew.h>
#include <GL\freeglut.h>



//glm opengl math lib -see docs for licensing
#include <glm.hpp>
using glm::vec2;
