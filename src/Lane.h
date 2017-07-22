#pragma once
#ifndef _LANE_H
#define _LANE_H

#include "Globals.h"
#include "GPUProgramManager.h"

#define _LANE_USES_GPU
//#define _LANE_OLD_TRANSFORMATIONS

class Camera;

class Lane
{
public:
	Lane();
	void		init(mat4 initialMtx);
	void		shut();
	void		draw(const Camera& camera, GLuint texCubemapId, GLuint refractionTexId);
	void		update();

	float		getCylinderRadius() const;	// TODO: obsolete, to be removed
	vec3		getPosition() const;
	vec3		getNormalToSurface(const vec3& worldSpacePos) const;
	float		getDistToSurface(const vec3& worldSpacePos) const;
	vec3		getGravityVector(const vec3& worldSpacePos) const;

#ifdef _LANE_OLD_TRANSFORMATIONS
	float		interpolationMethod(float a, float b, float ratio) const;
#endif

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
#ifdef _LANE_OLD_TRANSFORMATIONS
	sf::Clock					m_transformationClock;
	std::vector<mat4>			m_mtxVector;
#endif

	struct Keyframe
	{
		float	dist;	// distance between both circles of the capsule
		float	r0;		// radius of the left circle of the capsule
		float	r1;		// radius of the right circle of the capsule
		float	yaw,pitch,roll;	// rotation of the lane
		vec3	pos;	// position of the center of the capsule

		Keyframe() :
			dist(1.f),
			r0(1.f), r1(1.f),
			yaw(0.f), pitch(0.f), roll(0.f),
			pos(0.f,0.f,0.f)
		{
		}
	};

	Keyframe					m_curKeyframe;

#ifdef _USE_ANTTWEAKBAR
	TwBar*						m_debugBar;
#endif
};

#endif
