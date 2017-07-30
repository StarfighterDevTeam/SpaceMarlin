#pragma once
#ifndef _ATOM_H
#define _ATOM_H

#include "Model.h"

class Atom : public Model
{
protected:
	virtual const GPUProgram*	getProgram() const override;
	virtual void				sendUniforms(const GPUProgram* program, const Camera& camera) const override;

	float m_lastAnimationTimeSecs;

public:
	Atom();

	Atom* Clone();

	void update();

	float			m_speed;
};

#endif // _ATOM_H
