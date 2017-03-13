#ifndef SINGLE_FILE_H_INCLUDED
#define SINGLE_FILE_H_INCLUDED


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
#include <memory>
using std::make_unique;
using std::unique_ptr;
#include <vector>
using std::vector;
#include <unordered_map>
using std::unordered_map;

//OpenGL includes - see docs for licensing
#include <GL\glew.h>
#include <GL\freeglut.h>


#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"


//glm opengl math lib -see docs for licensing
#include <glm.hpp>
using glm::vec2;


class shader
{


	GLuint compileShader(GLenum shaderType, const string source);

	GLuint compileShaderProgram(const string &vsSource, const string &fsSource);


	//vertex shader for loaded texture
	string vs = R"(
			attribute vec4 aPos;
			attribute vec2 aCoord;
			varying vec2 vCoord;

			void main()
			{	
				gl_Position = aPos;
				vCoord = aCoord;
			}
		)";

	//fragment shader for loaded texture
	string fs = R"(
			precision mediump float;
			varying vec2 vCoord;
			uniform sampler2D uTex;

			void main()
			{
				gl_FragColor = texture2D(uTex, vCoord);	
			}	
		)";


	GLuint _program;
	GLuint _samplerLocation;
	GLuint _textureCoord;	//location or coordinates?

	GLuint _positionAttribute;
	GLuint _colorAttribute;

	//add error checking

public:
	//needs dctor
	shader();
	

	void initShader();
	
	void useShader(const GLfloat *Vertices);


};




//class shader;

//both classes badly need refactor
//need destructors, memory leaks aplenty!
class texture	//this will load textures that can be drawn to display
{				//should this hold shader program or display?

				//shader program functions
//	shader _program;
	//default shader program
	int usingShaderNumber = 0;
	GLuint _texture;
	int texWidth, texHeight;
	float zoom = 1.f;
	float rotation = 0.f;
	vec2 externalPosition, internalPosition;
	unsigned char * image;
	//heres where the magic happens

	GLfloat Vertices[20] =
	{
		-1.f, 1.f, 0.0f,
		0.0f, 1.f,
		-1.f, -1.f, 0.0f,
		0.0f, 0.0f,
		1.f, -1.f, 0.0f,
		1.f, 0.0f,
		1.f, 1.f, 0.0f,
		1.f, 1.f
	};

	GLushort Indices[6] = { 0, 1, 2 ,0, 2, 3 };
	bool isLoaded = false;


public:

	bool isGood();

	//need to use variables to ctor
	texture(); 
//	texture(display *canvas);
	texture(const texture & cpy);


	void updateVertices();
	
	//scalar resize, should also have scalarx and scalary transforms, but with better names
	void resize(int width);
	void translateTexture(vec2 pos);
	void traverseTexture(vec2 pos);
	void zoomTexture(float scalar);
	void rotateTexture(float rot);

	float getZoom();
	float getRotation();
	vec2 getExternalPosition();
	vec2 getInternalPosition();

	GLfloat * getVtx();

	int usesShader();

	//and layer info
	//then maybe animated sprites

	void load(const string &file);
	
	//kindof confusing names, this loads it to draw, the other loads it from file
	void loadTexture();
	void drawTexture();
private:


	//loads file using stbi_image free lib...needs error checking
	void loadImage(const string &file);
	
};

///////////////////////////////




//defaults for glutLoop
//take member functions in display class
void defaultTimer(int r);
void defaultDraw();
void defaultKeyboard(unsigned char k, int x, int y);
void defaultMouse(int x, int y);

class display	//this will create display and start glut loop
{				//must figure out how to pass functions to display



public:
	//need dctor stuff and error handling
//	display() {}
	display(string t="");

	
	int winHeight = 512, winWidth = 512;	//need to find out how to init these for diff machines
	int refresh;	//need to make sure this does what i think it does
	string title;
	unordered_map<unsigned char, bool> keyListeners;
	vec2 mousePos;

	vector<unique_ptr<shader>> shaderList;
	//consider using a map with named textures
	vector<std::shared_ptr<texture>> textureList;

	//to be called every iteration of glutLoop
	function<void()> userUpdate;

	//default functions passed globally above this class
	void memberTimer(int r);
	void memberDraw();
	void memberKeyboard(unsigned char k, int x, int y);
	void memberMouse(int x, int y);

	//C function pointers for glutLoop
	void(*timerEvent)(int) = &defaultTimer;
	void(*draw)() = &defaultDraw;
	void(*keyboard)(unsigned char, int, int) = &defaultKeyboard;
	void(*mouse)(int, int) = &defaultMouse;	//this is just motion
	//need mouseClick and mouseDrag which are glutMouseFunc and glutMotion
	

	//this is how you run an initialized display
	void openDisplay(int *ac, char ** av);	//need to find out what these command line params are used for

	//member accessor functions
	void addTexture(std::shared_ptr<texture> &tex);
	void setUpdate(function<void()> update);
	void addKeyListener(unsigned char key);
	void removeKeyListener(unsigned char key);
	bool checkKeyListener(unsigned char key);
	vec2 getMousePos();

	void setSize(vec2 win);

	int getHeight() const;
	int getWidth() const;


}thisDisplay;


#endif // !SINGLE_FILE_H_INCLUDED