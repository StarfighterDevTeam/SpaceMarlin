// TextureCreation.cpp

#include "TextureCreation.h"

namespace glutil
{

// 2D, depth
GLuint createTextureDepth(int width, int height)
{
	GLuint id_texture;
	glGenTextures(1, &id_texture);
	glBindTexture(GL_TEXTURE_2D, id_texture);

	// Set the filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the texture
	glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_DEPTH_COMPONENT24,
			width, height,
			0,
			GL_DEPTH_COMPONENT,
			GL_UNSIGNED_BYTE,
			NULL);

	return id_texture;
}

// Rect, depth
GLuint createTextureRectDepth(int width, int height)
{
	GLuint id_texture;
	glGenTextures(1, &id_texture);
	glBindTexture(GL_TEXTURE_RECTANGLE, id_texture);

	// Set the filter
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the texture
	glTexImage2D(
			GL_TEXTURE_RECTANGLE,
			0,
			GL_DEPTH_COMPONENT24,
			width, height,
			0,
			GL_DEPTH_COMPONENT,
			GL_UNSIGNED_BYTE,
			NULL);

	return id_texture;
}

// 2D, RGBA8
GLuint createTextureRGBA8(int width, int height, const GLubyte* data)
{
	GLuint id_texture;
	glGenTextures(1, &id_texture);
	glBindTexture(GL_TEXTURE_2D, id_texture);

	// Set the filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the texture
	glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA8,
			width, height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			(const GLvoid*)data);

	return id_texture;
}

// Rect, RGBA8
GLuint createTextureRectRGBA8(int width, int height, const GLubyte* data)
{
	GLuint id_texture;
	glGenTextures(1, &id_texture);
	glBindTexture(GL_TEXTURE_RECTANGLE, id_texture);

	// Set the filter
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the texture
	glTexImage2D(
			GL_TEXTURE_RECTANGLE,
			0,
			GL_RGBA8,
			width, height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			(const GLvoid*)data);

	return id_texture;
}

// 2D, R8
GLuint createTextureR8(int width, int height, const GLubyte* data)
{
	GLuint id_texture;
	glGenTextures(1, &id_texture);
	glBindTexture(GL_TEXTURE_2D, id_texture);

	// Set the filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the texture
	glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_R8,
			width, height,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			(const GLvoid*)data);

	return id_texture;
}

// Rect, R8
GLuint createTextureRectR8(int width, int height, const GLubyte* data)
{
	GLuint id_texture;
	glGenTextures(1, &id_texture);
	glBindTexture(GL_TEXTURE_RECTANGLE, id_texture);

	// Set the filter
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the texture
	glTexImage2D(
			GL_TEXTURE_RECTANGLE,
			0,
			GL_R8,
			width, height,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			(const GLvoid*)data);

	return id_texture;
}

// 2D, RGBA16F|RGBA32F
GLuint createTextureRGBAF(int width, int height, bool use_half_float)
{
	GLuint id_texture;
	glGenTextures(1, &id_texture);
	glBindTexture(GL_TEXTURE_2D, id_texture);

	// Set the filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the texture
	glTexImage2D(
			GL_TEXTURE_2D,
			0,
			(use_half_float ? GL_RGBA16F : GL_RGBA32F),
			width, height,
			0,
			GL_RGBA,
			GL_FLOAT,
			NULL);

	return id_texture;
}

// Rect, RGBA16F|RGBA32F
GLuint createTextureRectRGBAF(int width, int height, bool use_half_float)
{
	GLuint id_texture;
	glGenTextures(1, &id_texture);
	glBindTexture(GL_TEXTURE_RECTANGLE, id_texture);

	// Set the filter
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the texture
	glTexImage2D(
			GL_TEXTURE_RECTANGLE,
			0,
			(use_half_float ? GL_RGBA16F : GL_RGBA32F),
			width, height,
			0,
			GL_RGBA,
			GL_FLOAT,
			NULL);

	return id_texture;
}

// Renderbuffer, RGBA8
GLuint createRenderbufferRGBA8(int width, int height)
{
	GLuint id_rb;

	glGenRenderbuffers(1, &id_rb);
	glBindRenderbuffer(GL_RENDERBUFFER, id_rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);

	return id_rb;
}

// Renderbuffer, depth
GLuint createRenderbufferDepth(int width, int height)
{
	GLuint id_rb;

	glGenRenderbuffers(1, &id_rb);
	glBindRenderbuffer(GL_RENDERBUFFER, id_rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

	return id_rb;
}

}
