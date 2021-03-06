#pragma once
#ifndef _MARLIN_H
#define _MARLIN_H

#include "Model.h"

enum MarlinState
{
	STATE_IDLE,
	STATE_JUMPING,
	STATE_DIVING,
};

class Lane;

class Marlin : public ModelInstance
{
protected:
	virtual const GPUProgram*	getProgram() const override;
	virtual void				sendUniforms(const GPUProgram* program, const Camera& camera) const override;

	std::vector<const Lane*>			m_lanes;

	float m_surfaceSpeedLateral;
	float m_airSpeedLateral;
	float m_jumpSpeedVertical;
	float m_diveSpeedVertical;
	float m_diveSpeedLateral;
	float m_gravityAccelerationVertical;

	vec3 m_speed;
	vec3 m_speedMoveLateral;
	float m_speedMax;

	vec3 m_tangentToGravityAtJumpTime;

	MarlinState m_state;

	float m_lastAnimationTimeSecs;

public:
	Marlin();

	bool init();
	void shut();
	void update();
	void addLane(const Lane* lane);
};

#endif // _MARLIN_H
