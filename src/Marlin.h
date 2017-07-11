#pragma once
#ifndef _MARLIN_H
#define _MARLIN_H

#include "Model.h"

class Marlin : public Model
{
protected:
	const GPUProgram*	getProgram() const override;
	void				sendUniforms(const GPUProgram* program, const Camera& camera) const override;

public:
	Marlin();

	void update();

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

};

#endif // _MARLIN_H
