#include "Marlin.h"
#include "Camera.h"
#include "InputManager.h"
#include "Lane.h"

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

	m_mass								= 1.f;

	//temp
	//setPosition(glm::vec3(	0.f,//laterality
	//						2.f,//altitude
	//						0.f));
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

float Marlin::getDistanceSquaredToLane(Lane* lane) const
{
	float dx = getPosition().x - lane->getPosition().x;
	float dy = getPosition().y - lane->getPosition().y;

	float distSquared = dx*dx + dy*dy;

	return distSquared;
}

float Marlin::getGravitationalForce(Lane* lane) const
{
	//Universal gravitational force: https://en.wikipedia.org/wiki/Newton%27s_law_of_universal_gravitation

	float G = 0.0000000000667384f;	//G = 6.67384ee-11
	float gravitationalForce = G * m_mass * lane->getMass() / getDistanceSquaredToLane(lane);

	return gravitationalForce;
}

float Marlin::getArchimedeThrust(Lane* lane) const
{
	//Archimedes' principle: https://en.wikipedia.org/wiki/Archimedes%27_principle

	float marlinSize = 0.4; //height
	float altitude = getDistanceSquaredToLane(lane) - lane->getCylinderRadius();
	
	float immergedQuota;
	if (altitude < - marlinSize / 2)
	{
		immergedQuota = 1;//100% of Marlin's volume is under water
	}
	else if (altitude > marlinSize / 2)
	{ 
		immergedQuota = 0;//0% of Marlin's volume is under water
	}
	else
	{
		immergedQuota = abs(altitude) / marlinSize / 2;// between 0 and 100% is under water, with a pro rata
	}

	float P = 2.5f;	//"weight" of the lane's fluid
	float archimedeThrust = P * immergedQuota * m_mass;

	return archimedeThrust;
}

bool Marlin::addLane(Lane* lane)
{
	if (lane)
	{
		m_lanes.push_back(lane);
	}
		
	return lane;
}

void Marlin::update()
{
	//Test
	//if (!m_lanes.empty())
	//{
	//	vec3 debugPos = getPosition();
	//
	//	float gravity = getGravitationalForce(m_lanes.front());
	//	debugPos += vec3(0, -gravity * gData.dTime.asSeconds(), 0);
	//	move(vec3(0, -gravity * gData.dTime.asSeconds(), 0));//todo : rotate the vector according to angle
	//
	//	float archi = getArchimedeThrust(m_lanes.front());
	//	debugPos += vec3(0, archi * gData.dTime.asSeconds(), 0);
	//	move(vec3(0, archi * gData.dTime.asSeconds(), 0));//todo : rotate the vector according to angle
	//}

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