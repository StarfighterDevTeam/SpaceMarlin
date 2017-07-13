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

	m_speed								= vec3(0, 0, 0);
	//temp
	setPosition(glm::vec3(	0.f,//laterality
							6.f,//altitude
							0.f));
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

bool Marlin::addLane(Lane* lane)
{
	if (lane)
	{
		m_lanes.push_back(lane);
	}

	return lane;
}

float Marlin::getDistanceSquaredToLane(Lane* lane) const
{
	float dx = getPosition().x - lane->getPosition().x;
	float dy = getPosition().y - lane->getPosition().y;
	float dz = getPosition().z - lane->getPosition().z;

	float distSquared = dx*dx + dy*dy + dz*dz;

	return distSquared;
}

vec3 Marlin::getGravitationalForce(Lane* lane) const
{
	//Universal gravitational force: https://en.wikipedia.org/wiki/Newton%27s_law_of_universal_gravitation

	//float G = 0.0000000000667384f;	//G = 6.67384ee-11
	float G = 1.f;	//G = 6.67384ee-11
	float d = getDistanceSquaredToLane(lane);
	//float gravitationalForce = d != 0 ? G * m_mass * lane->getMass() / d : 0;//avoides to divide by 0
	float gravitationalForce = d != 0 ? G / d : 0;//avoides to divide by 0. Simplification of the law: ignore lane and marlin's masses because they're constant.

	//get the direction of the vector
	vec3 vector = lane->getPosition() - getPosition();
	//normalize the vector
	float n = abs(vector.x) + abs(vector.y) + abs(vector.z);
	if (n != 0)
	{
		vector = vec3(vector.x / n, vector.y / n, vector.z / n);
	}
	//glm::normalize(vector);

	float t = gData.dTime.asSeconds();
	vec3 gravity = vec3(t * vector.x * gravitationalForce,
						t * vector.y * gravitationalForce,
						t * vector.z * gravitationalForce);

	return gravity;
}

vec3 Marlin::getArchimedeThrust(Lane* lane) const
{
	//Archimedes' principle: https://en.wikipedia.org/wiki/Archimedes%27_principle

	float marlinSize = 0.4; //theorically it should be the Marlin's volume. But height is a good approximation of how much he's immerged.
	float altitude = sqrt(getDistanceSquaredToLane(lane)) - lane->getCylinderRadius();
	
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

	float P = 1.f;	//pression of the fluid
	float archimedeThrust = P * immergedQuota;

	//look for the direction of the vector
	vec3 vector = getPosition() - lane->getPosition();
	//normalize the vector
	float n = abs(vector.x) + abs(vector.y) + abs(vector.z);
	if (n != 0)
	{
		vector = vec3(vector.x / n, vector.y / n, vector.z / n);
	}
	//glm::normalize(vector);

	float t = gData.dTime.asSeconds();

	vec3 archimede = vec3(t * vector.x * archimedeThrust,
							t * vector.y * archimedeThrust,
							t * vector.z * archimedeThrust);

	return archimede;
}

void Marlin::update()
{
	//Test
	//if (!m_lanes.empty())
	//{
	//	bool splashIsPossible = sqrt(getDistanceSquaredToLane(m_lanes.front())) > m_lanes.front()->getCylinderRadius();
	//
	//	vec3 debugPos = getPosition();
	//
	//	vec3 gravity = getGravitationalForce(m_lanes.front());
	//	
	//	//move(gravity);//todo : rotate the vector according to angle
	//
	//	vec3 archi = getArchimedeThrust(m_lanes.front());
	//	//move(archi);//todo : rotate the vector according to angle
	//
	//	//move(gravity + archi);
	//
	//	m_speed += gravity + archi;
	//
	//	//have we splashed onto the lane? (back from "above" the lane)
	//	bool splashOccurred = splashIsPossible && sqrt(getDistanceSquaredToLane(m_lanes.front())) < m_lanes.front()->getCylinderRadius();
	//	if (splashOccurred)
	//	{
	//		//apply splash speed malus, due to the resistance of the fluid
	//		m_speed *= 1.0;//0.1f
	//	}
	//
	//	//normalize the vector
	//	float maxSpeed = 1 * gData.dTime.asSeconds();
	//	float n = abs(m_speed.x) + abs(m_speed.y) + abs(m_speed.z);
	//	if (n != 0)
	//	{
	//		m_speed = vec3(m_speed.x / n * maxSpeed, m_speed.y / n * maxSpeed, m_speed.z / n * maxSpeed);
	//	}
	//
	//	//apply speed
	//	vec3 debugPos2 = getPosition();
	//	move(m_speed * gData.dTime.asSeconds());
	//	debugPos2 = getPosition();
	//
	//	printf("pos      x: %f | y: %f | z: %f\ngravity  x: %f | y: %f | z: %f\narchimede x: %f | y: %f | z: %f\nsplash malus: %d\ntotal move x: %f | y: %f | z: %f\n\n", 
	//		debugPos.x, debugPos.y, debugPos.z,
	//		gravity.x, gravity.y, gravity.z,
	//		archi.x, archi.y, archi.z,
	//		(int)splashOccurred,
	//		gravity.x + archi.x, gravity.y + archi.y, gravity.z + archi.z);
	//}

	
	//OLD MOVE SYSTEM
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