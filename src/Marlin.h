#pragma once
#ifndef _MARLIN_H
#define _MARLIN_H

#include "Model.h"

class Lane;

class Marlin : public Model
{
protected:
	virtual const GPUProgram*	getProgram() const override;
	virtual void				sendUniforms(const GPUProgram* program, const Camera& camera) const override;

	std::vector<Lane*>			m_lanes;

	float m_surfaceSpeedLateral;
	float m_airSpeedLateral;
	float m_jumpSpeedVertical;
	float m_diveSpeedVertical;
	float m_diveSpeedLateral;
	float m_offsetX;
	float m_offsetZ;
	float m_speedX;
	float m_speedZ;
	float m_gravityAccelerationVertical;
	float m_archimedeAccelerationVertical;

	float m_mass;

public:
	Marlin();

	void update();
	float getDistanceSquaredToLane(Lane* lane) const;
	float getGravitationalForce(Lane* lane) const;
	float getArchimedeThrust(Lane* lane) const;
	bool addLane(Lane* lane);
};

#endif // _MARLIN_H
