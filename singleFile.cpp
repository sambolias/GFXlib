//GFXlib.cpp
//Sam Erie
//2/22/17
//start of my graphics library


#include "singleFile.h"


GLuint shader::compileShader(GLenum shaderType, const string source)
{
	GLuint shader = glCreateShader(shaderType);

	const char * sourceArray[1] = { source.c_str() };	//needs to match const GLchar *
	glShaderSource(shader, 1, sourceArray, NULL);
	glCompileShader(shader);

	GLint compileResults;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResults);

	if (compileResults == 0)
	{
		std::cout << "Shader Error";
		//this means there was an error
		//need to do something here
		//see other project for how to log
	}

	return shader;
}

GLuint shader::compileShaderProgram(const string & vsSource, const string & fsSource)
{

	GLuint program = glCreateProgram();

	if (program == 0)
	{
		std::cout << "Shader Error";
		//should probably throw program creation failed error
		return 0;
	}

	GLuint vs = compileShader(GL_VERTEX_SHADER, vsSource);
	GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSource);

	if (vs == 0 || fs == 0)	//if either failed to compile
	{	//clean up mess
		glDeleteShader(vs);
		glDeleteShader(fs);
		glDeleteProgram(program);
		std::cout << "Shader Error";
		//maybe throw, but return for now
		return 0;
	}

	glAttachShader(program, vs);
	glDeleteShader(vs);	//no longer needed

	glAttachShader(program, fs);
	glDeleteShader(fs);	//same, RAII

	glLinkProgram(program);

	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);	//check similar to shaders

	if (linkStatus == 0)
	{
		std::cout << "Shader Error";
		//needs to throw or return 0
		//so that nobody trys to use bad program
		//I need to make overall exception strategy first
	}

	return program;

}

shader::shader()
{
}

void shader::initShader()
{
	_program = compileShaderProgram(vs, fs);
	_positionAttribute = glGetAttribLocation(_program, "aPos");
	_textureCoord = glGetAttribLocation(_program, "aCoord");
	_samplerLocation = glGetUniformLocation(_program, "uTex");	//these names need fixed, not descriptive
	_colorAttribute = glGetAttribLocation(_program, "aColor");
}

void shader::useShader(const GLfloat * Vertices)
{
	//error handling

	glUseProgram(_program);
	glVertexAttribPointer(_positionAttribute, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), Vertices);
	glVertexAttribPointer(_textureCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), Vertices + 3);

	glEnableVertexAttribArray(_positionAttribute);
	glEnableVertexAttribArray(_textureCoord);

	glUniform1i(_samplerLocation, 0);

}


bool texture::isGood()
{
	return isLoaded;
}

//all texture needs from display is size, but externalPosition does need updated...global statics or accessors?
texture::texture() : externalPosition(vec2(512.f / 2.f, 512.f / 2.f)), internalPosition(vec2(5.f, 5.f))
{
}

texture::texture(const texture & cpy) 
{
	isLoaded = cpy.isLoaded;
	usingShaderNumber = cpy.usingShaderNumber;
	_texture=cpy._texture;
	image = cpy.image;
	texWidth = cpy.texWidth;
	texHeight = cpy.texHeight;
	zoom = cpy.zoom;
	rotation = cpy.rotation;
	externalPosition = cpy.externalPosition;
	internalPosition = cpy.internalPosition;

	updateVertices();

}



void texture::resize(int width)
{
	float scale = (float)width / (float)texWidth;
	texWidth *= scale;
	texHeight *= scale;
	updateVertices();
}

void texture::translateTexture(vec2 pos)
{
	externalPosition = pos;
}

void texture::updateVertices()
{
	//this function translates vertices
	//texture matrix, needs better comments

	GLfloat x = (internalPosition.x - 5.f);
	GLfloat y = (internalPosition.y - 5.f);

	//fun trig - better variable names and some extra space
	//could probably elim need for tons of comments here
	//needs to keep aspect ratio on rotate
	GLfloat V[20] =
	{
		(zoom*glm::sin(glm::radians(rotation + 315 - 90))) + x,(zoom*glm::cos(glm::radians(rotation + 315 - 90))) + y, 0.f,
		0.f, 1.f,
		(zoom*glm::sin(glm::radians(rotation + 45 - 90))) + x, (zoom*glm::cos(glm::radians(rotation + 45 - 90))) + y, 0.f,
		0.f, 0.f,
		(zoom*glm::sin(glm::radians(rotation + 135 - 90))) + x, (zoom*glm::cos(glm::radians(rotation + 135 - 90))) + y, 0.f,
		1.f, 0.0f,
		(zoom*glm::sin(glm::radians(rotation + 225 - 90))) + x, (zoom*glm::cos(glm::radians(rotation + 225 - 90))) + y, 0.f,
		1.f, 1.f
	};

	//post newly generated vtx
	for (int i = 0; i < 20; i++)
	{
		Vertices[i] = V[i];
	}

}

void texture::traverseTexture(vec2 pos)
{
	internalPosition = pos;
	updateVertices();
}

void texture::zoomTexture(float scalar)
{
	zoom = scalar;
	updateVertices();
}

void texture::rotateTexture(float rot)
{
	do
	{
		
		if (rot > 360)rot -= 360;
		if (rot < 0)rot += 360;

		rotation = rot;

	} while (!(rot < 360 && rot > 0));
	updateVertices();
}

float texture::getZoom()
{
	return zoom;
}

float texture::getRotation()
{
	return rotation;
}

vec2 texture::getExternalPosition()
{
	return externalPosition;
}

vec2 texture::getInternalPosition()
{
	return internalPosition;
}

GLfloat * texture::getVtx()
{
	return Vertices;
}

int texture::usesShader()
{
	return usingShaderNumber;
}

void texture::load(const string & file)
{
	//this needs error checking

	//load the texture from file

	//opengl part
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	//stb_image.h part
	loadImage(file);

	isLoaded = true;
}

void texture::loadTexture()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
}

void texture::drawTexture()
{

	glActiveTexture(GL_TEXTURE0);	//this could be a conflict if drawing multiple textures
									//needs to be planned for -prob not,when drawing one at a time
	glBindTexture(GL_TEXTURE_2D, _texture);


	//needed to overlap sprites - otherwise transparant background draw overwrites previously drawn background
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	/////////////////////////////////////////////////////

	glPushMatrix();

	//draws image to stored external Position, center of image at externalPosition point
	glViewport((int)((float)externalPosition.x - texWidth / 2.f), (int)((float)externalPosition.y - texHeight / 2.f), texWidth, texHeight);

	//this is the actual draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, Indices);

	glPopMatrix();
}

void texture::loadImage(const string & file)
{
	int width = 0, height = 0, channels = 0;

	unsigned char * img;

	//stbi_set_flip_vertically_on_load(true);
	img = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	//save to member variables
	image = img;
	texWidth = width;
	texHeight = height;

}


///////////////////////////////////////


display::display(string t) : title(t)
{}

void defaultTimer(int r)
{
	thisDisplay.memberTimer(r);
}

void display::memberTimer(int r)
{
	if (glutGetWindow())
	{
		glutPostRedisplay();
		glutTimerFunc(r, defaultTimer, 0);
	}
}


void defaultDraw()
{
	thisDisplay.memberDraw();
}

void display::memberDraw()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

//	for(auto tex : textureList ) how to this way?
	//loop draws all textures 
	for (unsigned int i = 0; i < textureList.size(); i++)
	{
		textureList[i]->loadTexture();
		shaderList[textureList[i]->usesShader()]->useShader(textureList[i]->getVtx());
		textureList[i]->drawTexture();
	}

	
	//user control
	if(userUpdate!=nullptr)
		userUpdate();

	//reset listener needs function
	for (int i = 0; i < keyListeners.size(); i++)
	{
		keyListeners[i] = false;
	}


	glutSwapBuffers();
	glutPostRedisplay();
}


void defaultKeyboard(unsigned char key, int x, int y)
{
	thisDisplay.memberKeyboard(key, x, y);
}

void defaultMouse(int x, int y)
{
	thisDisplay.memberMouse(x, y);
}
	
void display::memberKeyboard(unsigned char k, int x, int y)
{
	//use int glutGetModifiers() to get shift + key, ctrl + key etc
	//use seperate keyboard function glutSpecialFunc for directions and such
	mousePos = vec2(x, y);

	keyListeners[k] = true;

	switch (k)
	{

	case 27:
		glutDestroyWindow(glutGetWindow());
		break;
	default:
		break;
	}
}

void display::memberMouse(int x, int y)
{
	mousePos = vec2(x, -y+winHeight);
}

void display::openDisplay(int * ac, char ** av)
{
	//init glut, error handling?
	glutInit(ac, av);	//at_EXIT_HACK for windows? research
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);	//is there any reason to change these
	glutInitWindowSize(winWidth, winHeight);

	glutCreateWindow(title.c_str());
	//glutFullScreen();	//this needs its own function
						//Init GLEW
	GLenum error = glewInit();

	if (error != GLEW_OK)	//this like all other error checking
	{						//needs handled
		std::cout << "Found the problem\n";
	}


	glutDisplayFunc(draw);
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutTimerFunc(refresh, timerEvent, 0);

	//how much harder would it to be to access OpenGL without freeglut and glew?
	//if I'm piggybacking on these libraries I have to accept all glutxxxxFunc binds


	//some of these calls are not necessary for 2d
	//need to research

	//R   G    B    A
	glClearColor(0.f, 0.f, 0.f, 0.f);	//this needs to take variables
										//	glViewport(0, 0, winWidth, winHeight);	//this needs more customizable, maybe...prob not for 2d

	glMatrixMode(GL_PROJECTION);	//again prob good for 2d
	glLoadIdentity();

	//field of view  - aspect ratio  - near distance- far distance
	//	gluPerspective(60.f, (GLfloat)winWidth / winHeight, 0.f, 30.f);

	//this was in ctor
	shaderList.push_back(make_unique<shader>());
	shaderList[0]->initShader();

	//run glut loop
	glutMainLoop();

}

void display::addTexture(std::shared_ptr<texture> &tex)
{
	textureList.push_back(tex);
}

void display::setUpdate(function<void()> update)
{
	userUpdate = update;
}

void display::addKeyListener(unsigned char key)
{
	keyListeners[key] = false;
}

void display::removeKeyListener(unsigned char key)
{
	keyListeners.erase(key);
}

bool display::checkKeyListener(unsigned char key)
{
	return keyListeners[key];
}

vec2 display::getMousePos()
{
	return mousePos;
}

void display::setSize(vec2 win)
{
	
	for (int i = 0; i < textureList.size(); i++)
	{
		textureList[i]->translateTexture(vec2( 
					(int)((float)win.x/(float)winWidth * textureList[i]->getExternalPosition().x ),
					(int)((float)win.y / (float)winHeight * textureList[i]->getExternalPosition().y )
		));
	}

	winWidth = win.x;
	winHeight = win.y;

}

int display::getHeight() const
{
	return winHeight;
}

int display::getWidth() const
{
	return winWidth;
}



//testing
int main(int argc, char **argv)
{

	//create texture
	texture tex;
	tex.load("test.jpg");

	//test multiple textures
	texture tex2;
	tex2.load("test.jpg");


	//test display init functions
	thisDisplay.addTexture(std::make_shared<texture>(tex));	//this should make pointer in function
	thisDisplay.addTexture(std::make_shared<texture>(tex2));	//need to come up with layering method
	thisDisplay.setSize(vec2(1000, 1000));
	
	thisDisplay.addKeyListener('w');
	thisDisplay.addKeyListener('s');

	auto t1 = thisDisplay.textureList[0];
	auto t2 = thisDisplay.textureList[1];
	
	float rot = t1->getRotation();
	//test update function
	thisDisplay.setUpdate([&]() {
	
		t1 = thisDisplay.textureList[0];
		
		rot = t1->getRotation();

		//test texture manipulation in update loop
		t1->rotateTexture(rot + .1f);
	
		//test random movement
		//t1->translateTexture(vec2(t1->getExternalPosition().x+ (((int)glm::radians(rot) % 3)?1:-1)*(.25f*cos(glm::radians(rot))), t1->getExternalPosition().y+ (((int)glm::radians(rot) % 3)?-1:1)*(.25f*cos(glm::radians(rot)))));

		//test keyboard movement
		if(thisDisplay.checkKeyListener('w'))
		{
			t2->translateTexture(vec2(t2->getExternalPosition().x, t2->getExternalPosition().y+10));
		}
		if (thisDisplay.checkKeyListener('s'))
		{
			t2->translateTexture(vec2(t2->getExternalPosition().x, t2->getExternalPosition().y - 10));
		}
		
		//test mouse movement
		//keyboard input messes this up, not sure why yet
		t1->translateTexture(thisDisplay.getMousePos());


	});

	
	//run display
	thisDisplay.openDisplay(&argc, argv);
}
