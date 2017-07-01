#pragma once
#ifndef _SKYBOX_H
#define _SKYBOX_H

#include "Globals.h"
#include "SharedDefines.h"

class Camera;

class Skybox
{
public:
	bool		loadFromFiles(	const char* cubemapFilename0,
								const char* cubemapFilename1,
								const char* cubemapFilename2,
								const char* cubemapFilename3,
								const char* cubemapFilename4,
								const char* cubemapFilename5);
	void		unload();
	void		draw(const Camera& camera);
	void		update();

private:
	GLuint		m_vertexArrayId;
	GLuint		m_vertexBufferId;

	GLuint		m_skyTexId;

	bool		m_loaded;
};

#endif
