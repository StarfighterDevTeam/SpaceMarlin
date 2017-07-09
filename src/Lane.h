#pragma once
#ifndef _LANE_H
#define _LANE_H

#include "Globals.h"
#include "SharedDefines.h"

//#define _LANE_USES_GPU

class Camera;

class Lane
{
public:
	Lane();
	void		init();
	void		shut();
	void		draw(const Camera& camera, GLuint texCubemapId);
	void		update();

private:
	static void	computeNormals(VtxLane* vertices, int nbVertices, const unsigned short* indices, int nbIndices);

	std::vector<unsigned short>	m_indices;
#ifdef _LANE_USES_GPU
	GLuint						m_heightsTexId[3];
#else
	std::vector<VtxLane>		m_vertices[3];
#endif
	int							m_curBufferIdx;
	float						m_lastAnimationTimeSecs;

	GLuint						m_vertexArrayId;
	GLuint						m_indexBufferId;
	GLuint						m_vertexBufferId;
};

#endif
