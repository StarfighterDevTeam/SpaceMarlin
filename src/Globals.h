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

class GPUProgramManager;

struct Globals
{
	Globals();
	int					winSizeX;
	int					winSizeY;
	sf::Time			dTime;
	GPUProgramManager*	gpuProgramMgr;
};

extern Globals gData;

#endif // _GLOBALS_H

