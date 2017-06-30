#pragma once
#ifndef _GLOBALS_H
#define _GLOBALS_H

#define SFML_STATIC
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <limits.h>
#include <float.h>
#include <sstream>

#include <GL/glew.h>

#include <SFML/Config.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define COLOR_RED		glm::vec4(1,0,0,1)
#define COLOR_GREEN		glm::vec4(0,1,0,1)
#define COLOR_BLUE		glm::vec4(0,0,1,1)
#define COLOR_YELLOW	glm::vec4(1,1,0,1)
#define COLOR_CYAN		glm::vec4(0,1,1,1)
#define COLOR_MAGENTA	glm::vec4(1,0,1,1)
#define COLOR_WHITE		glm::vec4(1,1,1,1)
#define COLOR_BLACK		glm::vec4(0,0,0,1)
#define COLOR_GRAY		glm::vec4(0.5f,0.5f,0.5f,1)

#include "log/Log.h"

#define SAFE_DELETE(x) {delete x; x=NULL;}

template<class T>
T clamp(T a, T minVal, T maxVal)
{
	if(a > maxVal)
		a = maxVal;
	if(a < minVal)
		a = minVal;
	return a;
}

#ifndef _WIN32
	#define sprintf_s sprintf
#endif

#ifdef _WIN32
	#define DIR_SEP '\\'
	#define SDIR_SEP "\\"
#else
	#define DIR_SEP '/'
	#define SDIR_SEP "/"
#endif

class GPUProgramManager;
class Drawer;

struct Globals
{
	Globals();
	int					winSizeX;
	int					winSizeY;
	sf::Time			dTime;
	sf::Clock			clock;
	sf::Time			frameTime;
	GPUProgramManager*	gpuProgramMgr;
	Drawer*				drawer;
	std::string			exePath;
	std::string			assetsPath;
	std::string			shadersPath;
};

extern Globals gData;

#endif // _GLOBALS_H

