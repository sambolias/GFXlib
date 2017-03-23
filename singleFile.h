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
#include <gtc/matrix_transform.hpp>
using glm::vec2;



class shader
{
	friend class display;
	friend class texture;
	friend class textBox;

	GLuint compileShader(GLenum shaderType, const string source);

	GLuint compileShaderProgram(const string &vsSource, const string &fsSource);


	//vertex shader for loaded texture
	string vs = R"(
			attribute vec4 aPos;
			attribute vec2 aCoord;
			varying vec2 vCoord;

			uniform mat4 MV;
			uniform mat4 P;
			uniform float SW;
			uniform float TW;

			vec4 eye;
			vec4 proj;
			
			void main()
			{	
				eye = MV * vec4(aPos.x,aPos.y, 0.5, 1);
				proj = P * vec4(0.5*TW, 0.5*TW, eye.z, eye.w);

				gl_PointSize = SW* proj.x / proj.w;
				gl_Position = P * eye;
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

							
	GLuint _projectID;
	GLuint _modelID;
	GLuint _screenWidth;
	GLuint _texWidth;
	

	GLuint _positionAttribute;
	GLuint _colorAttribute;

	//add error checking

protected:
	
	GLuint & getProjectID();
	GLuint & getModelID();
	GLuint & getScreenWidthID();
	GLuint & getTexWidthID();
	void initShader();
	
	void useShader(const GLfloat *Vertices);

public:
	//needs dctor
	shader();
	~shader();

};



class texture	
{	
protected:
	friend class display;

	//private members

	//default shader program 0
	int usingShaderNumber;
	GLuint _texture;
	//texture size (stays constant after load)
	int texWidth, texHeight;
	//transformation variables
	float zoom;
	float scaleX, scaleY;
	float rotation;
  //on screen translation   //in texture traversal
	vec2 externalPosition, internalPosition;
	//array of rgb(a) pixels
	unsigned char * image=nullptr;

	
	//Model Matrix
	//These probably don't need to be stored as they are
	//recalculated every iteration
	glm::mat4 Model;
	glm::mat4 mv;

//texture input/output vtx 
	GLfloat Vertices[20];
	GLushort Indices[6];
	bool isLoaded;


public:

	bool isGood();

	texture(); 
	
	virtual ~texture();

	
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

	void load(const string &file);
	void load(vec2 &size, glm::vec4 &rgba);

	void draw(vec2 &pos, glm::vec4 &rgba);

protected:
	GLfloat * getVtx();
	int usesShader();	//this can be public if multiple shaders are supported
	//kindof confusing names, this loads it to draw, the other loads it from file
	virtual void loadTexture();
	virtual void drawTexture();


	void updateVertices();
	//loads file using stbi_image free lib...needs error checking
	void loadImage(const string &file);
	
	void transformMatrices();

};


class textBox: public texture
{

	

public:
	textBox(string t);

	void setText(string text);
	void setBackground(glm::vec4 rgba);
	void setTextColor(glm::vec4 rgba);
	void setTextSize(float size);
	void setBorderSize(vec2 size);

private:
	void makeBackground();

	const unsigned char * text;

	glm::vec4 bg_color = glm::vec4(0, 0, 0, 175);
	glm::vec3 text_color = glm::vec3(1, 1, 1);

	float scale=.25f;
	textBox(const textBox & cpy);	//this should probably be a deleted function
protected:
	virtual void drawTexture() override;
	virtual void loadTexture() override;


};

///////////////////////////////


class camera
{
public:

	camera() {}

	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model;

	//if this works out make accessor functions

};

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
	display();

	~display() = default;	//make sure there are no more resources that need deleted

	bool fullscreen = false;
	int winHeight = 512, winWidth = 512;	//need to find out how to init these for diff machines
	int refresh;	//need to make sure this does what i think it does
	string title;
	unordered_map<unsigned char, bool> keyListeners;
	vec2 mousePos;
	camera cam;


	vector<unique_ptr<shader>> shaderList;
	//consider using a map with named textures
	//find out if this could use a unique pointer
	vector<texture*> textureList;

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
	void addTexture(texture &tex);
	void setUpdate(function<void()> update);
	void addKeyListener(unsigned char key);
	void removeKeyListener(unsigned char key);
	bool checkKeyListener(unsigned char key);
	vec2 getMousePos();

	int getHeight() const;
	int getWidth() const;

	//display init (pre-open) functions
	void setTitle(string t);
	void setSize(vec2 win);
	void setFullscreen();


}thisDisplay;	//this is the only display that should ever exist



#endif // !SINGLE_FILE_H_INCLUDED