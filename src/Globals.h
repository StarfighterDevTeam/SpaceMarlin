#pragma once
#ifndef _GLOBALS_H
#define _GLOBALS_H

#define SFML_STATIC
#define _USE_MATH_DEFINES
#define _USE_ANTTWEAKBAR

#pragma warning(error : 4715)	// "not all control paths return a value"

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

#ifdef _USE_ANTTWEAKBAR
	#define TW_STATIC
	#define TW_NO_LIB_PRAGMA
	#include <AntTweakBar.h>
#else
	class TwBar;
#endif

#include <SFML/Config.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::quat;

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
#define NO_ACTION(...)

#pragma warning(disable : 4068)	// "warning C4068: unknown pragma": needed for #pragma custom_preprocessor_[on|off]

template<class T>
T clamp(T a, T minVal, T maxVal)
{
	if(a > maxVal)
		a = maxVal;
	if(a < minVal)
		a = minVal;
	return a;
}

template <class TVec>
TVec safeNormalize(const TVec& v, const TVec& backup)
{
	float len = glm::length(v);
	if(len*len < 0.000001f)
		return backup;
	return v * (1.f/len);
}

// GLM and GLSL sign() returns 0 for 0, this one returns either 1 or -1
// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/sign.xhtml
inline float safeSign(float x)
{
	return x >= 0.f ? 1.f : -1.f;
}

inline sf::Time bpmToBeatDuration(float bpm)
{
	return sf::seconds(60.f / bpm);
}

inline float beatDurationToBpm(const sf::Time& beatDuration)
{
	return 60.f / beatDuration.asSeconds();
}

#ifndef _WIN32
	#define sprintf_s sprintf
	#define strncpy_s strncpy
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
class SoundManager;
class InputManager;

struct Globals
{
	Globals();
	int					winSizeX;
	int					winSizeY;
	sf::Time			dTime;
	sf::Clock			clock;
	sf::Time			curFrameTime;
	GPUProgramManager*	gpuProgramMgr;
	Drawer*				drawer;
	SoundManager*		soundMgr;
	InputManager*		inputMgr;
	std::string			exePath;
	std::string			assetsPath;
	std::string			shadersPath;
	sf::RenderWindow*	window;
};

extern Globals gData;

#endif // _GLOBALS_H

