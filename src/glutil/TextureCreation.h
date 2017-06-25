// TextureCreation.h

#ifndef TEXTURE_CREATION_H
#define TEXTURE_CREATION_H

//#include "glxw.h"
//#include "../Common.h"
#include "../Globals.h"

namespace glutil
{

// 2D, depth
GLuint createTextureDepth(int width, int height);

// Rect, depth
GLuint createTextureRectDepth(int width, int height);

// 2D, RGBA8
GLuint createTextureRGBA8(int width, int height, const GLubyte* data=NULL);

// Rect, RGBA8
GLuint createTextureRectRGBA8(int width, int height, const GLubyte* data=NULL);

// 2D, RED8
GLuint createTextureR8(int width, int height, const GLubyte* data=NULL);

// Rect, RED8
GLuint createTextureRectR8(int width, int height, const GLubyte* data=NULL);

// 2D, RGBA16F|RGBA32F
GLuint createTextureRGBAF(int width, int height, bool use_half_float);

// Rect, RGBA16F|RGBA32F
GLuint createTextureRectRGBAF(int width, int height, bool use_half_float);

// Renderbuffer, RGBA8
GLuint createRenderbufferRGBA8(int width, int height);

// Renderbuffer, depth
GLuint createRenderbufferDepth(int width, int height);

}

#endif // TEXTURE_CREATION_H
