#include "Marlin.h"
#include "Camera.h"
#include "InputManager.h"

Marlin::Marlin()
{
	m_surfaceSpeedLateral				= +5.f;;
	m_airSpeedLateral					= +2.5f;
	m_jumpSpeedVertical					= +10.f;
	m_diveSpeedVertical					= -10.f;
	m_diveSpeedLateral					= +2.5f;
	m_gravityAccelerationVertical		= -30.f;
	m_archimedeAccelerationVertical		= +30;
	m_offsetX							= 0;
	m_offsetZ							= 0;
	m_speedX							= 0;
	m_speedZ							= 0;
	
}

const GPUProgram* Marlin::getProgram() const
{
	return gData.gpuProgramMgr->getProgram(PROG_MARLIN);
}

void Marlin::sendUniforms(const GPUProgram* program, const Camera& camera) const
{
	Model::sendUniforms(program, camera);

	program->sendUniform("gSpeed", vec3(m_speedX, 0.f, m_speedZ));
	//program->sendUniform("texAlbedo", 0);
	//program->sendUniform("gTime", gData.curFrameTime.asSeconds());
}

void Marlin::update()
{
	bool bobIsJumping = m_offsetZ > 0;
	bool bobIsDiving = m_offsetZ < 0;

	//Deceleration
	m_speedX = 0;

	//Move left
	if (gData.inputMgr->isLeftPressed())
	{
		if (m_offsetZ == 0)
		{
			m_speedX -= m_surfaceSpeedLateral;
		}
		else if (m_offsetZ > 0)
		{
			m_speedX -= m_airSpeedLateral;
		}
		else//if (m_bobOffsetZ < 0)
		{
			m_speedX -= m_diveSpeedLateral;
		}
	}

	//Move right
	if (gData.inputMgr->isRightPressed())
	{
		if (m_offsetZ == 0)
		{
			m_speedX += m_surfaceSpeedLateral;
		}
		else if (m_offsetZ > 0)
		{
			m_speedX += m_airSpeedLateral;
		}
		else//if (m_bobOffsetZ < 0)
		{
			m_speedX += m_diveSpeedLateral;
		}
	}

	//Jump
	if (gData.inputMgr->isUpTapped())
	{
		if (m_offsetZ == 0)//Bob must be on the surface to jump
		{
			m_speedZ += m_jumpSpeedVertical;
		}
	}

	//Dive
	if (gData.inputMgr->isDownTapped())
	{
		if (m_offsetZ == 0)//Bob must be on the surface to dive
		{
			m_speedZ += m_diveSpeedVertical;
		}
	}

	//Speed limits
	if (m_speedX > m_surfaceSpeedLateral) m_speedX = m_surfaceSpeedLateral;
	if (m_speedX < -m_surfaceSpeedLateral) m_speedX = -m_surfaceSpeedLateral;

	//Gravity force
	if (m_offsetZ > 0)
	{
		m_speedZ += m_gravityAccelerationVertical * gData.dTime.asSeconds();
	}
	//Archimed force
	if (m_offsetZ < 0)
	{
		m_speedZ += m_archimedeAccelerationVertical * gData.dTime.asSeconds();
	}

	//Apply speed to offset
	m_offsetX += m_speedX * gData.dTime.asSeconds();
	m_offsetZ += m_speedZ * gData.dTime.asSeconds();

	//Returning back to the surface from a jump or a dive
	if ((m_offsetZ <= 0 && bobIsJumping)
		|| (m_offsetZ >= 0 && bobIsDiving))
	{
		m_speedZ = 0;
		m_offsetZ = 0;
	}

	//Apply offset to position
	setPosition(glm::vec3(m_offsetX,
		m_offsetZ,
		0.f));
}