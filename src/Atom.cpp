#include "Atom.h"

Atom::Atom()
{
	m_speed					= 0.0f;

	//temp
	setPosition				(glm::vec3(	0.f,//laterality
										0.f,//altitude
										0.f));

	m_lastAnimationTimeSecs = -1.f;
}

const GPUProgram* Atom::getProgram() const
{
	return gData.gpuProgramMgr->getProgram(PROG_ATOM);
}

void Atom::sendUniforms(const GPUProgram* program, const Camera& camera) const
{
	ModelInstance::sendUniforms(program, camera);

	program->sendUniform("texAlbedo", 0);
}

void Atom::update()
{
	if (m_lastAnimationTimeSecs < 0.f)
		m_lastAnimationTimeSecs = gData.curFrameTime.asSeconds();

	static float ANIMATIONS_PER_SECOND = 60.0f;

	// do rendering to get ANIMATIONS_PER_SECOND
	while (m_lastAnimationTimeSecs <= gData.curFrameTime.asSeconds())
	{
		// todo : scroll on Z and follow lane on X,Y
		move(vec3(0.f, 0.f, m_speed * 1.0f / ANIMATIONS_PER_SECOND));

		m_lastAnimationTimeSecs += (1.0f / ANIMATIONS_PER_SECOND);
	}

	//printf("Pos x: %f, pos y: %f, pos z: %f\n", getPosition().x, getPosition().y, getPosition().z);
}
