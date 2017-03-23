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
		glDeleteProgram(program);
		return 0;
	}

	return program;

}

shader::shader()
{
}

shader::~shader()
{
	glDeleteProgram(_program);
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
	_modelID = glGetUniformLocation(_program, "MV");
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
	usingShaderNumber = 0;
	zoom = 1.f;
	scaleX = 1.f;
	scaleY = 1.f;
	rotation = 0.f;

	Model = glm::mat4(1.0f);

	GLfloat V[] =
	{
		-1.f, 1.f, 0.0f,	//screen (x,y,z)
		0.0f, 1.f,			//texture (u,v)
		-1.f, -1.f, 0.0f,
		0.0f, 0.0f,
		1.f, -1.f, 0.0f,
		1.f, 0.0f,
		1.f, 1.f, 0.0f,
		1.f, 1.f
	};

	//copy contents
	for (int i = 0; i < 20; i++)
	{
		Vertices[i] = V[i];
	}
	GLshort I[] = { 0, 1, 2 ,0, 2, 3 };
	//copy contents
	for (int i = 0; i < 6; i++)
	{
		Indices[i] = I[i];
	}

	isLoaded = false;
}



textBox::textBox(const textBox & cpy)
{

	text = cpy.text;
	bg_color = cpy.bg_color;
	text_color = cpy.text_color;

	isLoaded = cpy.isLoaded;
	usingShaderNumber = cpy.usingShaderNumber;
	_texture = cpy._texture;
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

texture::~texture()
{
	glDeleteTextures(1, &_texture);
	if (image != nullptr)
	{
		delete[] image;
	}
}



void texture::resize(int width)
{
	if (texWidth > 0)	//temp fix for resize textBox bug --think i need to store scale and do this later
	{
		float scale = (float)width / (float)texWidth;

		//needs to be a scalex scaley
		scaleX = scale;
		scaleY = scale;
	}
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
	
	//deletes if there is image loaded
	if (image != nullptr)
	{
		delete[] image;
		glDeleteTextures(1, &_texture);
	}
	//load the texture from file

	//this needs error checking

	//opengl part
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	//stb_image.h part
	loadImage(file);
	
	isLoaded = true;
}

bool tolerance(float a, float b, int t)
{
	if(a>b)
	{
		if (a - t < b)return true;
	}
	else if (a + t > b) return true;

	return false;
}

void texture::load(vec2 &size, glm::vec4 &rgba)
{


	texWidth = size.x;
	texHeight = size.y;

	//deletes if there is image loaded
	if (image != nullptr)
	{
		delete[] image;
		glDeleteTextures(1, &_texture);
	}

	//allocates image pointer
	image = new unsigned char[ 4 * size.x * size.y ];
	
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);

	//initialize pixels to rgba
	int x = 0;
	int y = 0;
	for (size_t i = 0; i < (4 * texWidth*texHeight); i += 4)
	{
		image[i] = (unsigned char)rgba.x;
		image[i+1] = (unsigned char)rgba.y;
		image[i+2] = (unsigned char)rgba.z;
		image[i+3] = (unsigned char)rgba.w;
		

		//circle needs to be saved
		//need to make drawFunc() that takes std::function<bool(x,y)>
		//which can be used in place of for loop in both functions this and below
		/*
		if(tolerance((x-400)*(x-400) + (y-400)*(y-400), 100*100, 100))
		{
			image[i] = (unsigned char)50;
			image[i + 1] = (unsigned char)50;
			image[i + 2] = (unsigned char)150;
			image[i + 3] = (unsigned char)150;
		}*/

		//confusing line, counting by 4's (rgba) for every pixel
		//so its like saying x mod width, which == 0 when x == y * width
		if ( ( (i/4)%(int)size.x ) == 0 )
		{
			x = 0;
			y++;
		}
		else x++;

	}

	isLoaded = true;
}

void texture::draw(vec2 & pos, glm::vec4 & rgba)
{

	//need to throw some kind of error in else
	if (image != nullptr)
	{

		int x = 0;
		int y = 0;
		for (size_t i = 0; i < (4 * texWidth*texHeight); i += 4)
		{

			//if (x >= 400 && y >= 400) another example for drawFunc()
			if(x==pos.x && y == pos.y)
			{
				image[i] = (unsigned char)rgba[0];
				image[i + 1] = (unsigned char)rgba[1];
				image[i + 2] = (unsigned char)rgba[2];
				image[i + 3] = (unsigned char)rgba[3];
			}

			if (((i / 4) % texWidth) == 0)
			{
				x = 0;
				y++;
			}
			else x++;

		}
	}

}

void texture::loadTexture()
{

	if (isLoaded) 
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	}
}

void texture::drawTexture()
{
					//there is a conflict with textBox - it uses GL_TEXTURE1 --
	glActiveTexture(GL_TEXTURE0);	//this could be a conflict if drawing multiple textures
									//needs to be planned for -prob not,when drawing one at a time
	glBindTexture(GL_TEXTURE_2D, _texture);

	//set up cam matrix for shader
	glUniform1f((thisDisplay.shaderList[usesShader()]->getScreenWidthID()), thisDisplay.getWidth());
	glUniformMatrix4fv((thisDisplay.shaderList[usesShader()]->getProjectID()), 1, GL_FALSE, &(thisDisplay.cam.Projection[0][0]));

	glPushMatrix();

	
	//needed to overlap sprites - otherwise transparant background draw overwrites previously drawn background
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	/////////////////////////////////////////////////////
	
	//this messes up alpha layers and reverses layers
	//glEnable(GL_DEPTH_TEST);

	//handles scale rotate and translate
	transformMatrices();
	

	//this is the actual draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, Indices);


	glPopMatrix();

}

void texture::loadImage(const string & file)
{
	int width = 0, height = 0, channels = 0;

	//deletes if there is image loaded
	if (image != nullptr)
	{
		delete[] image;
		glDeleteTextures(1, &_texture);
	}

	//need to look into stbi allocation
	stbi_set_flip_vertically_on_load(true);
	//save to member variables
	image = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

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
	float rx = externalPosition.x / ( thisDisplay.getWidth());
	float ry = externalPosition.y / ( thisDisplay.getHeight());
	//this isn't quite right, but on the right path i think

	transView = glm::translate(thisDisplay.cam.View, glm::vec3((rx) - .5f, (ry) - .5f, -.5f));
	

	mv = transView*transModel;

	//set up shader with  model * view matrix
	glUniform1f((thisDisplay.shaderList[usingShaderNumber]->getTexWidthID()), texWidth);
	glUniformMatrix4fv((thisDisplay.shaderList[usingShaderNumber]->getModelID()), 1, GL_FALSE, &mv[0][0]);

}


///////////////////////////////////////


display::display()
{

	title = "";	        				//field of view 			aspect ratio			near  far
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

	for (unsigned int i = 0; i < textureList.size(); i++)
	{
		
		if (textureList[i]->isGood()) 
		{
			textureList[i]->loadTexture();
			shaderList[textureList[i]->usesShader()]->useShader(textureList[i]->getVtx());
		}
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

	//this mousePos does not match mouse fn 
	//	mousePos = vec2(x, y);

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
	mousePos = vec2( x, winHeight-y );
}

void display::openDisplay(int * ac, char ** av)
{
	//init glut, error handling?
	glutInit(ac, av);	//at_EXIT_HACK for windows? research
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);	//is there any reason to change these
	glutInitWindowSize(winWidth, winHeight);

	glutCreateWindow(title.c_str());
	
	if (fullscreen)
	{
		glutFullScreen();	//this needs its own function that knows fullscreen size
		//update window size
		vec2 win(glutGet(GLUT_SCREEN_HEIGHT), glutGet(GLUT_SCREEN_WIDTH));
		setSize(win);
	}
						//Init GLEW
	GLenum error = glewInit();

	if (error != GLEW_OK)	//this like all other error checking
	{						//needs handled
		std::cout << "Fatal Error - Could not initiate GLEW\n";
	}


	//init glutloop functions
	glutDisplayFunc(draw);
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutTimerFunc(refresh, timerEvent, 0);


	//R   G    B    A
	glClearColor(0.f, 0.f, 0.f, 0.f);	//this needs to take variables

	glViewport(0, 0, winWidth, winHeight);

	//create default texture shader
	shaderList.push_back(make_unique<shader>());
	shaderList[0]->initShader();

	//run glut loop
	glutMainLoop();

}


//need a custome deleter or something I don't know about...

void display::addTexture(texture &tex)
{
	//textureList.push_back(make_unique<texture>(tex));
	textureList.push_back(&tex);
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

void display::setTitle(string t)
{
	title = t;
}

void display::setSize(vec2 win)
{
	
	for (int i = 0; i < textureList.size(); i++)
	{
		//scale texture pos to new screen size
		textureList[i]->translateTexture(vec2( 
					(int)( (float)win.x / (float)winWidth * textureList[i]->getExternalPosition().x ),
					(int)( (float)win.y / (float)winHeight * textureList[i]->getExternalPosition().y )
		
		));
	}

	//change screen size members
	winWidth = win.x;
	winHeight = win.y;

}


void display::setFullscreen()
{
	fullscreen = true;
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
	tex.load("kim.jpg");
	//tex.load("test2.bmp");
	tex.resize(400);
	//test multiple textures
	texture tex2;
	tex2.load("test.jpg");
	//tex2.load("test2.bmp");
	tex2.resize(100);
	
	texture tex3;

	tex3.load(vec2(800.f, 800.f), glm::vec4( 255.f, 255.f, 255.f, 255.f));
	tex3.resize(100);
	//test display init functions
	unsigned char  t[] = { "HEY YOU GUYS!!" };
	textBox tex4("HEY YOU GUYS!!");
//	tex4.resize(1);	//do not resize textBox bug
	//tex4.load(vec2(glutStrokeLengthf(GLUT_STROKE_ROMAN, t), glutStrokeHeight(GLUT_STROKE_ROMAN)), glm::vec4(255, 0, 0, 220));
	//tex4.setBackground(glm::vec4(255, 0, 0, 220));

	texture tex5;
	tex5.load("test.jpg");
	tex5.resize(80);

	thisDisplay.addTexture(tex2);
	thisDisplay.addTexture(tex);		
	thisDisplay.addTexture(tex3);
	thisDisplay.addTexture(tex4);
	thisDisplay.addTexture(tex5);
	

	//textBox t("test");
	//thisDisplay.text.push_back(std::make_shared<textBox>(t));

	//aspect ratio problem - textures gets warped for widescreen ex 1500,1000
	thisDisplay.setSize(vec2(1000, 1000));
	thisDisplay.setTitle("Test Display - GFXlib");
	//thisDisplay.setFullscreen();
	
	//find out why these don't need to be added...more convenient if they don't
	//the keylistener automatically adds hit keys, this method may be needed for 
	//multikey presses still though, not supported yet
//	thisDisplay.addKeyListener('w');
//	thisDisplay.addKeyListener('s');
	
	auto & t1 = thisDisplay.textureList[1];
	auto & t2 = thisDisplay.textureList[0];
	
	float rot = t1->getRotation();

	glm::vec4 color(155, 255, 115, 115);
	int inc = 1;

	//test update function
	//and other features
	thisDisplay.setUpdate([&]() {
	
		
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

		
		//testing to set up texture::draw()

		if (thisDisplay.checkKeyListener('0'))
		{
			inc *= -1;
		}

		if (thisDisplay.checkKeyListener('3'))
		{
			color[0] += inc;
		}
		if (thisDisplay.checkKeyListener('4'))
		{
			color[1] += inc;
		}
		if (thisDisplay.checkKeyListener('5'))
		{
			color[2] += inc;
		}
		if (thisDisplay.checkKeyListener('6'))
		{
			color[3] += inc;
		}
		thisDisplay.textureList[2]->draw(vec2(0, 0), color);

		//test mouse movement
		vec2 m = thisDisplay.getMousePos();
	//	std::cout << m.x << " " << m.y << "\n";
		t1->translateTexture(m);

		if (thisDisplay.checkKeyListener('l'))
		{
			tex4.setText("I can say different things \n on different lines");
		}
		tex4.load(vec2(glutStrokeLengthf(GLUT_STROKE_ROMAN, t), glutStrokeHeight(GLUT_STROKE_ROMAN)), glm::vec4(255, 0, 0, 220));
		tex5.translateTexture(vec2(150, 150));
	});

	
	//run display
	thisDisplay.openDisplay(&argc, argv);
}


//now that i figured it out fix, should only be one box behind text string
void textBox::drawTexture()
{
		vec2 size = vec2(glutStrokeLengthf(GLUT_STROKE_ROMAN, text), glutStrokeHeight(GLUT_STROKE_ROMAN));
		
		if (!isLoaded)
		{
			load(size, bg_color);
		
			loadTexture();
			thisDisplay.shaderList[usesShader()]->useShader(Vertices);
		
		}
	
		//this is to draw text background...might be better to have a separate
		//texture draw function, because this is the exact same as the original 
		//with the text draw appended
		//also i need to declare override

		//except for having to use a different active texture
		//there is a lot available, do I need to use more than 2?

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _texture);

		//set up cam matrix for shader
		glUniform1f((thisDisplay.shaderList[usesShader()]->getScreenWidthID()), thisDisplay.getWidth());
		glUniformMatrix4fv((thisDisplay.shaderList[usesShader()]->getProjectID()), 1, GL_FALSE, &(thisDisplay.cam.Projection[0][0]));

		glPushMatrix();

		//needed to overlap sprites - otherwise transparant background draw overwrites previously drawn background
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		/////////////////////////////////////////////////////
		//handles scale rotate and translate
		transformMatrices();

		//this is the actual draw
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, Indices);

		glPopMatrix();
	
		
	//draw text last

	glPushMatrix();

	//this unloads default shader program
	//so that gltransforms can be used
	glUseProgram(NULL);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, thisDisplay.getWidth(), 0, thisDisplay.getHeight(), .1, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(text_color.x, text_color.y, text_color.z);
	//scale needs member variable so it can be adjusted, /8.f and .25 coincide
	glTranslatef(externalPosition.x-size.x*scale/2.f, externalPosition.y-size.y*scale/2.f, -.4);
	glScalef(scale, scale, 0);

	glutStrokeString(GLUT_STROKE_ROMAN, text);
	glPopMatrix();

	//reset active texture 
	glActiveTexture(GL_TEXTURE0);

}

void textBox::loadTexture()
{
	 if (isLoaded)
	{
		updateVertices();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	}
}



textBox::textBox(string t) : texture()//, text(reinterpret_cast<const unsigned char *>(t.c_str()))
{
	
	//roundabout way to cast unsigned char * from string -- above method compiles but not correct string

	unsigned char * temp = new unsigned char[t.length()];
	int i = 0;

	setText(t);

	texWidth = 1;
	texHeight = 1;

	scaleX = .25;
	scaleY = .25;

}

void textBox::setText(string t)
{
	unsigned char * temp = new unsigned char[t.length()];
	int i = 0;

	for (auto c : t)
	{
		temp[i] = c;
		i++;
	}
	temp[i] = '\0';
	text = temp;

	//delete[] temp;	does this need managed?
}

void textBox::setBackground(glm::vec4 rgba)
{
	bg_color = rgba;
	makeBackground();
}

void textBox::setTextColor(glm::vec4 rgba)
{
	text_color = rgba;
}

void textBox::setTextSize(float size)
{
	scale = size;
}

void textBox::setBorderSize(vec2 size)
{
	texWidth = size.x;
	texHeight = size.y;

	makeBackground();
}

void textBox::makeBackground()
{
	load(vec2(texWidth,texHeight), bg_color);
}


