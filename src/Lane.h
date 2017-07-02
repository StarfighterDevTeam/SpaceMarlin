#pragma once
#ifndef _LANE_H
#define _LANE_H

#include "Globals.h"
#include "SharedDefines.h"

class Camera;

class Lane
{
public:
	void		init();
	void		shut();
	void		draw(const Camera& camera);
	void		update();

private:
	void		updateVertices();

	std::vector<unsigned short>	m_indices;
	std::vector<VtxLane>		m_vertices;

	GLuint						m_vertexArrayId;
	GLuint						m_indexBufferId;
	GLuint						m_vertexBufferId;
};

#endif
