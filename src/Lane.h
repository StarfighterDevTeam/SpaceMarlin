#pragma once
#ifndef _LANE_H
#define _LANE_H

#include "Globals.h"
#include "GPUProgramManager.h"

#define _LANE_USES_GPU

class Camera;

class Lane
{
public:
	Lane();
	void		init();
	void		shut();
	void		draw(const Camera& camera, GLuint texCubemapId, GLuint refractionTexId);
	void		update();

	float		getCylinderRadius() const;
	vec3		getPosition() const;

	float		interpolationMethod(float a, float b, float ratio) const;

private:
	static void	computeNormals(VtxLane* vertices, int nbVertices, const unsigned short* indices, int nbIndices);
	void		updateWaterOnGPU();

	std::vector<unsigned short>	m_indices;
#ifdef _LANE_USES_GPU
	// GPU resources for simulating water
	GLuint						m_heightsTexId[3];
	GLuint						m_waterFboId[3];
	GLuint						m_waterVertexArrayId;
	GLuint						m_waterVertexBufferId;
#else
	std::vector<VtxLane>		m_vertices[3];
#endif
	int							m_curBufferIdx;
	float						m_lastAnimationTimeSecs;

	mat4						m_localToWorldMtx;

	// GPU resources for drawing the lane
	GLuint						m_vertexArrayId;
	GLuint						m_indexBufferId;
	GLuint						m_vertexBufferId;

	//Lane transformation
	sf::Clock					m_transformationClock;
	std::vector<mat4>			m_mtxVector;

#ifdef _USE_ANTTWEAKBAR
	TwBar*						m_debugBar;
#endif
};

#endif
