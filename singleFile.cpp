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

GLuint & shader::getProjectID()
{
	return _projectID;
}

GLuint & shader::getModelID()
{
	return _modelID;
}

GLuint & shader::getScreenWidthID()
{
	return _screenWidth;
}

GLuint & shader::getTexWidthID()
{
	return _texWidth;
}

void shader::initShader()
{
	_program = compileShaderProgram(vs, fs);
	_positionAttribute = glGetAttribLocation(_program, "aPos");
	_textureCoord = glGetAttribLocation(_program, "aCoord");
	_samplerLocation = glGetUniformLocation(_program, "uTex");	//these names need fixed, not descriptive
	_colorAttribute = glGetAttribLocation(_program, "aColor");
	_modelID = glGetUniformLocation(_program, "MVP");
	_projectID = glGetUniformLocation(_program, "P");
	_screenWidth = glGetUniformLocation(_program, "SW");
	_texWidth = glGetUniformLocation(_program, "TW");
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
texture::texture() : externalPosition(vec2((float)thisDisplay.getWidth()/2.f, (float)thisDisplay.getWidth()/2.f)), internalPosition(vec2(5.f, 5.f))
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
	scaleX = cpy.scaleX;
	scaleY = cpy.scaleY;
	rotation = cpy.rotation;
	externalPosition = cpy.externalPosition;
	internalPosition = cpy.internalPosition;

	updateVertices();

}



void texture::resize(int width)
{

	float scale = (float)width / (float)texWidth;
	
	//needs to be a scalex scaley
	scaleX = scale;
	scaleY = scale;

}

void texture::translateTexture(vec2 pos)
{
	externalPosition = pos;
}



void texture::updateVertices()
{
	//this function translates vertices
	//texture matrix, needs better comments

	//this is not aspect ratio...not sure if this is right
	//need to figure it out and rename variables
	//I think the trick may be normalize
	float aspectY = (float)texHeight; // (float)texWidth;
	float aspectX = (float)texWidth; // (float)texWidth;

	vec2 vtxNormals = glm::normalize(vec2(aspectX, aspectY));

	GLfloat x = (internalPosition.x - 5.f);
	GLfloat y = (internalPosition.y - 5.f);
	
//	vec2 aspect( zoom * aspectX, zoom * aspectY);
	vec2 aspect( vtxNormals.x, vtxNormals.y);
	
	
	GLfloat V[20] =
	{
		-aspect.x,
		aspect.y, 0.f,
		0.f+x, zoom*1.f+y,
		-aspect.x,
		-aspect.y, 0.f,
		0.f+x, 0.f+y,
		aspect.x,
		-aspect.y, 0.f,
		zoom*1.f+x, 0.f+y,
		aspect.x,
		aspect.y, 0.f,
		zoom*1.f+x, zoom*1.f+y


	};
	

	//post newly generated vtx
	for (int i = 0; i < 20; i++)
	{
		Vertices[i] = V[i];
	}

}

void texture::traverseTexture(vec2 pos)
{
	//is there any reason to limit this? 
	//eventually something would overflow
	internalPosition = pos;
	updateVertices();
}

void texture::zoomTexture(float scalar)
{

	//zooms in only
	if (scalar <= 1 && scalar > 0 )
	{
		zoom = scalar;
		updateVertices();
	}
}

void texture::rotateTexture(float rot)
{
	rot -= rotation;
	do
	{
		
		if (rot > 360.f)rot -= 360.f;
		if (rot < 0.f)rot += 360.f;

		rotation = rot;

	} while (!(rot < 360.f && rot > 0.f));
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

	glEnable(GL_DEPTH_TEST);

	//handles scale rotate and translate
	transformMatrices();
	

	//this is the actual draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, Indices);

	glPopMatrix();


}

void texture::loadImage(const string & file)
{
	int width = 0, height = 0, channels = 0;

	unsigned char * img;

	stbi_set_flip_vertically_on_load(true);
	img = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	//save to member variables
	image = img;
	texWidth = width;
	texHeight = height;

}

void texture::transformMatrices()
{
	glm::mat4 transModel;
	glm::mat4 transView;

	Model = glm::rotate(Model, glm::radians((float)rotation), glm::vec3(0.f, 0.f, -1.f));
	transModel = glm::scale(Model, glm::vec3(scaleX, scaleY, 0));


	//need to calculate and subtract dist from center
	//variables need to be used to make this all more clear
	float rx = -externalPosition.x + thisDisplay.getWidth() / 2.f;
	float ry = -externalPosition.y + thisDisplay.getHeight() / 2.f;
	//this isn't quite right, but on the right path i think
	rx *= -.15f;
	ry *= -.15f;

	transView = glm::translate(thisDisplay.cam.View, glm::vec3((externalPosition.x - rx) / (thisDisplay.getWidth()) - .5f, (externalPosition.y - ry) / (thisDisplay.getHeight()) - .5f, -.5f));

	mvp = transView*transModel;

	//set up shader model * view matrix
	glUniform1f((thisDisplay.shaderList[usingShaderNumber]->getTexWidthID()), texWidth);
	glUniformMatrix4fv((thisDisplay.shaderList[usingShaderNumber]->getModelID()), 1, GL_FALSE, &mvp[0][0]);

}


///////////////////////////////////////


display::display()
{

	title = "default";					//field of view 			aspect ratio			near  far
	cam.Projection = glm::perspective(glm::radians(45.f), (float)winWidth/(float)winHeight, 0.1f, 10.f);
								       //eye			  center 			    up
	cam.View = glm::lookAt(glm::vec3(0, 0, .5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

}

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
	
	

//	for(auto tex : textureList ) how to this way?
	//loop draws all textures 
	for (unsigned int i = 0; i < textureList.size(); i++)
	{
		
		//set up cam matrix for shader
		glUniform1f((shaderList[textureList[i]->usesShader()]->getScreenWidthID()), winWidth);
		glUniformMatrix4fv((shaderList[textureList[i]->usesShader()]->getProjectID()), 1, GL_FALSE, &(cam.Projection[0][0]));

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
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);	//is there any reason to change these
	glutInitWindowSize(winWidth, winHeight);

	glutCreateWindow(title.c_str());
	//glutFullScreen();	//this needs its own function
						//Init GLEW
	GLenum error = glewInit();

	if (error != GLEW_OK)	//this like all other error checking
	{						//needs handled
		std::cout << "Found the problem\n";
	}


	//init glutloop functions
	glutDisplayFunc(draw);
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutTimerFunc(refresh, timerEvent, 0);


	//R   G    B    A
	glClearColor(0.f, 0.f, 0.f, 0.f);	//this needs to take variables

	//create default texture shader
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
	//tex.load("kim.jpg");
	tex.load("test2.bmp");
	tex.resize(800);
	//test multiple textures
	texture tex2;
	tex2.load("test.jpg");
	//tex2.load("test2.bmp");
	tex2.resize(200);
	
	//test display init functions
	thisDisplay.addTexture(std::make_shared<texture>(tex));	//this should make pointer in function
	thisDisplay.addTexture(std::make_shared<texture>(tex2));	//need to come up with layering method
	thisDisplay.setSize(vec2(1000, 1000));
	
	//find out why these don't need to be added...more convenient if they don't
//	thisDisplay.addKeyListener('w');
//	thisDisplay.addKeyListener('s');

	auto t1 = thisDisplay.textureList[0];
	auto t2 = thisDisplay.textureList[1];
	
	float rot = t1->getRotation();

	//test update function
	//and other features
	thisDisplay.setUpdate([&]() {
	
		t1 = thisDisplay.textureList[0];
		
		rot = t1->getRotation();

		//test texture manipulation in update loop
		t1->rotateTexture(rot + .1f);
	
		
		//test random movement
		//t1->translateTexture(vec2(t1->getExternalPosition().x+ (((int)glm::radians(rot) % 3)?1:-1)*(.25f*cos(glm::radians(rot))), t1->getExternalPosition().y+ (((int)glm::radians(rot) % 3)?-1:1)*(.25f*cos(glm::radians(rot)))));

		//test keyboard movement
		//test texture translations/traversals
		if(thisDisplay.checkKeyListener('w'))
		{
			t2->traverseTexture(vec2(t2->getInternalPosition().x, t2->getInternalPosition().y+.1));
		//	t2->translateTexture(vec2(t2->getExternalPosition().x, t2->getExternalPosition().y+1));
		}
		if (thisDisplay.checkKeyListener('s'))
		{
			t2->traverseTexture(vec2(t2->getInternalPosition().x, t2->getInternalPosition().y - .1));
		//	t2->translateTexture(vec2(t2->getExternalPosition().x, t2->getExternalPosition().y - 1));
		}
		if (thisDisplay.checkKeyListener('d'))
		{
			t2->traverseTexture(vec2(t2->getInternalPosition().x+.1, t2->getInternalPosition().y));
			//	t2->translateTexture(vec2(t2->getExternalPosition().x +1, t2->getExternalPosition().y));
		}
		if (thisDisplay.checkKeyListener('a'))
		{
			t2->traverseTexture(vec2(t2->getInternalPosition().x-.1, t2->getInternalPosition().y ));
			//	t2->translateTexture(vec2(t2->getExternalPosition().x-1, t2->getExternalPosition().y ));
		}
		//test zoom
		if (thisDisplay.checkKeyListener('1'))
		{
			t2->zoomTexture(t2->getZoom() + .1);
		}
		if (thisDisplay.checkKeyListener('2'))
		{
			t2->zoomTexture(t2->getZoom() - .1);
		}
		
		//test mouse movement
		//keyboard input messes this up, not sure why yet		--TODO--
		vec2 m = thisDisplay.getMousePos();
	//	std::cout << m.x << " " << m.y << "\n";
		t1->translateTexture(m);


	});

	
	//run display
	thisDisplay.openDisplay(&argc, argv);
}
