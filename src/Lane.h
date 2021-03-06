#pragma once
#ifndef _LANE_H
#define _LANE_H

#include "Globals.h"
#include "GPUProgramManager.h"

class Atom;
class Camera;
class ModelResource;
struct LaneTrack;

struct GPULaneKeyframe;

struct LaneKeyframe
{
	float		beat;	// beat at which the keyframe becomes the current one
	float		dist;	// distance between both circles of the capsule
	float		r0;		// radius of the left circle of the capsule
	float		r1;		// radius of the right circle of the capsule
	float		yaw,pitch,roll;	// rotation of the lane
	vec2		offset;	// 2D position of the center of the capsule

	// Precomputed values
	struct PrecomputedData
	{
		float	halfDist;
		float	theta;
		float	tanTheta;
		float	capsulePerimeter;

		float	threshold0;
		float	threshold1;
		float	threshold2;
		float	threshold3;
		float	threshold0to1;
		float	threshold2to3;

		float	xOffsetOnC0;
		float	xOffsetOnC1;

		float	yOffsetOnC0;
		float	yOffsetOnC1;

		vec2	topLeftPos;
		vec2	topRightPos;
		vec2	topTangentVector;

		vec2	bottomLeftPos;
		vec2	bottomRightPos;
		vec2	bottomTangentVector;

		mat4	localToWorldMtx;
		mat4	worldToLocalMtx;

		quat	qLocalToWorld;
		quat	qWorldToLocal;

	private:
		void update(float& dist, float& r0, float& r1, float& yaw, float& pitch, float& roll, vec2& offset);
	public:
		friend struct LaneKeyframe;
	};
	PrecomputedData	precomp;

	LaneKeyframe() :
		dist(1.f),
		r0(1.f), r1(1.f),
		yaw(0.f), pitch(0.f), roll(0.f),
		offset(0.f,0.f)
	{
		updatePrecomputedData();
	}

	void	setFromKeyframes(const LaneKeyframe& kf0, const LaneKeyframe& kf1, float newBeat);
	void	updatePrecomputedData() {precomp.update(dist, r0, r1, yaw, pitch, roll, offset);}
	void	toGPULaneKeyframe(GPULaneKeyframe& gpuKeyframe) const;
};

class Lane
{
public:
	Lane() : m_id(-1) {}

	void		init(LaneTrack* track, int id, ModelResource* atomBlueprint, TwBar* editionBar, TwBar* debugBar);
	void		shut();
	void		draw(const Camera& camera, GLuint texCubemapId, GLuint refractionTexId);
	void		update();

	vec3		getPosition() const;

	vec3		getNormalToSurface(const vec3& worldSpacePos) const;
	void		getCoordinateSystem(const vec3& worldSpacePos, vec3& worldSpaceRight, vec3& worldSpaceNormal, vec3& worldSpaceBack) const;
	float		getDistToSurface(const vec3& worldSpacePos) const;
	vec3        getGravityVector(const vec3& worldSpacePos) const;

	void		setEditionMode(bool editionMode)	{m_editionMode = editionMode;}

	void		setCurBeat(float curBeat);

	void		setupAtom(Atom* pAtom);

	const char* getName() const	{return m_name.c_str();}

private:
	void		getLocalSpaceCoordinateSystem(const vec3& localSpacePos, vec3& localSpaceRight, vec3& localSpaceNormal, vec3& localSpaceBack) const;
	void		debugDraw(const Camera& camera);
	void		uploadTrackKeyframesToGPU();
	void		updateWaterOnGPU();
	
	std::vector<unsigned short>	m_indices;

	ModelResource*				m_atomBlueprint;
	LaneTrack*					m_track;
	int							m_id;

	// GPU resources for simulating water
	GLuint						m_heightsTexId[3];
	GLuint						m_waterFboId[3];
	GLuint						m_waterVertexArrayId;
	GLuint						m_waterVertexBufferId;

	// GPU resources for water normals computation
	GLuint						m_waterNormalsTexId;
	GLuint						m_waterNormalsFboId;
	GLuint						m_waterNormalsVertexArrayId;
	GLuint						m_waterNormalsVertexBufferId;

	// GPU resources for keyframing
	GLuint						m_keyframesBufferId;
	GLuint						m_keyframesTexId;

	int							m_curBufferIdx;
	float						m_lastAnimationTimeSecs;

	// GPU resources for drawing the lane
	GLuint						m_vertexArrayId;
	GLuint						m_indexBufferId;
	GLuint						m_vertexBufferId;

	// Edition
	std::string					m_name;
	bool						m_prevEditionMode;
	bool						m_editionMode;
	LaneKeyframe				m_editionLaneKeyframe;
	int							m_prevEditedBeat;
	int							m_editedBeat;
	LaneKeyframe				m_curKeyframe;

	// Debug
	bool						m_bDebugDrawNormals;
	bool						m_bDebugDrawCoordinateSystems;
	bool						m_bDebugDrawDistToSurface;
	bool						m_bDebugDrawWaterBuffers;
};

#endif
