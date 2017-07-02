#pragma once
#ifndef _Background_H
#define _Background_H

#include "Globals.h"
#include "SharedDefines.h"

class Camera;

class Background
{
public:
	bool		load();
	void		unload();
	void		draw(const Camera& camera);
	void		update();

private:
	std::vector<unsigned short>	m_indices;
	std::vector<VtxBackground>	m_vertices;

	GLuint		m_vertexArrayId;
	GLuint		m_indexBufferId;
	GLuint		m_vertexBufferId;

	GLuint		m_perlinTexId;

	bool		m_loaded;
};

#endif
