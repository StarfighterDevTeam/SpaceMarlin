#include "Marlin.h"
#include "Camera.h"
#include "InputManager.h"
#include "Lane.h"

Marlin::Marlin()
{
	m_surfaceSpeedLateral				= +5.f;;
	m_airSpeedLateral					= +2.5f;
	m_jumpSpeedVertical					= +10.f;
	m_diveSpeedVertical					= -7.f;//-10.f can go through lane if cylinder radius = 2
	m_diveSpeedLateral					= +2.5f;
	m_gravityAccelerationVertical		= -30.f;
	m_archimedeAccelerationVertical		= +30;
	m_offsetX							= 0;
	m_offsetZ							= 0;
	m_speedX							= 0;
	m_speedZ							= 0;
	m_speedMax							= 20.f;

	m_mass								= 1.f;

	m_speed								= vec3(0, 0, 0);
	//temp
	setPosition				(glm::vec3(	0.f,//laterality
										4.f,//altitude
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

bool Marlin::addLane(const Lane* lane)
{
	if (lane)
	{
		m_lanes.push_back(lane);
	}

	return lane != NULL;
}

float Marlin::getAngleToLane(const Lane* lane) const
{
	const float a = getPosition().x - lane->getPosition().x;
	const float b = getPosition().y - lane->getPosition().y;

	if (a == 0 && b == 0)
		return 0.f;

	float distance_to_obj = (a * a) + (b * b);
	distance_to_obj = sqrt(distance_to_obj);

	float angle;
	angle = acos(a / distance_to_obj);

	if (b < 0)
	{
		angle = -angle;
	}

	return angle;
}

float Marlin::getDistanceSquaredToLane(const Lane* lane) const
{
	vec3 d = getPosition() - lane->getPosition();

	vec3 a = getPosition();
	vec3 b = lane->getPosition();

	float distSquared = d.x*d.x + d.y*d.y + d.z*d.z;

	//float dx = getPosition().x - lane->getPosition().x;
	//float dy = getPosition().y - lane->getPosition().y;
	//float dz = getPosition().z - lane->getPosition().z;
	//
	//float distSquared = dx*dx + dy*dy + dz*dz;

	return distSquared;
}

void Marlin::getAltitudeAndAngleToLane(const Lane* lane, float &altitude, float &angle) const
{
	const float a = getPosition().x - lane->getPosition().x;
	const float b = getPosition().y - lane->getPosition().y;

	if (a == 0 && b == 0)
	{
		altitude = 0;
		angle = 0;
		return;
	}

	float distance = (a * a) + (b * b);
	distance = sqrt(distance);

	angle = acos(a / distance);

	if (b < 0)
	{
		angle = -angle;
	}

	if (angle < 0)
	{
		printf("lol)");
	}

	altitude = distance - lane->getCylinderRadius(angle);

	
}

float Marlin::getAltitudeToLane(const Lane* lane) const
{
	return sqrt(getDistanceSquaredToLane(lane)) - lane->getCylinderRadius(getAngleToLane(lane));
}

float Marlin::getNormalizedSpeed() const
{
	return sqrt(m_speed.x*m_speed.x + m_speed.y*m_speed.y + m_speed.z*m_speed.z);
}

sf::Clock simulationStart;

void Marlin::update()
{
	//Test
	if (!m_lanes.empty() && simulationStart.getElapsedTime().asSeconds() > 6.f)
	{
		const Lane* lane = m_lanes.front();

		//gravity acceleration
		float altitude, angle;
		getAltitudeAndAngleToLane(lane, altitude, angle);

		vec3 vectorToLane = getPosition() - lane->getPosition();
		vectorToLane = normalize(vectorToLane);

		static float gravity = -25.f;
		if (altitude > 0)
		{
			m_speed += vectorToLane * gravity * gData.dTime.asSeconds();
		}
		else if (altitude < 0)
		{
			m_speed += vectorToLane * (-gravity) * gData.dTime.asSeconds();//using an inverted gravity to simulate an Archimedes' thrust
		}

		//Jump
		if (gData.inputMgr->isUpPressed())
		{
			if (altitude == 0)//Bob must be on the surface to jump
			{
				m_speed += vectorToLane * m_jumpSpeedVertical;
			}
		}

		//Dive
		if (gData.inputMgr->isDownPressed())
		{
			if (altitude == 0)//Bob must be on the surface to jump
			{
				m_speed += vectorToLane * m_diveSpeedVertical;
			}
		}

		//speed limit
		if (getNormalizedSpeed() > m_speedMax)
		{
			m_speed = normalize(m_speed) * m_speedMax;
		}

		//apply speed
		move(m_speed * gData.dTime.asSeconds());

		//moving through the lane's surface? -> loose momentum
		float altitudeNew, angleNew;
		getAltitudeAndAngleToLane(lane, altitudeNew, angleNew);

		//printf("altitude: %f", altitude);

		if ((altitude > 0 && altitudeNew <= 0)
			|| (altitude < 0 && altitudeNew >= 0))
		{
			//kill tiny oscillations by sticking the Marlin right on the lane surface
			if (getNormalizedSpeed() < abs(gravity)/20)//0.5f; the stronger the gravity, the more it's necessary
			{
				vec3 vectorToLaneNew = getPosition() - lane->getPosition();
				vectorToLaneNew = normalize(vectorToLane);

				vec3 vectorToStickToLane = vectorToLaneNew * (-altitudeNew);

				move(vectorToStickToLane);

				//printf("STICK TO LANE. ");
			}

			//loose momentum
			//m_speed = vec3(0, 0, 0);
			m_speed *= 0.5f;
			//printf("LOOSE MOMENTUM. ");
		}

		//printf("\n");
	}

	//OLD MOVE SYSTEM
	//bool bobIsJumping = m_offsetZ > 0;
	//bool bobIsDiving = m_offsetZ < 0;
	//
	////Deceleration
	//m_speedX = 0;
	//
	////Move left
	//if (gData.inputMgr->isLeftPressed())
	//{
	//	if (m_offsetZ == 0)
	//	{
	//		m_speedX -= m_surfaceSpeedLateral;
	//	}
	//	else if (m_offsetZ > 0)
	//	{
	//		m_speedX -= m_airSpeedLateral;
	//	}
	//	else//if (m_bobOffsetZ < 0)
	//	{
	//		m_speedX -= m_diveSpeedLateral;
	//	}
	//}
	//
	////Move right
	//if (gData.inputMgr->isRightPressed())
	//{
	//	if (m_offsetZ == 0)
	//	{
	//		m_speedX += m_surfaceSpeedLateral;
	//	}
	//	else if (m_offsetZ > 0)
	//	{
	//		m_speedX += m_airSpeedLateral;
	//	}
	//	else//if (m_bobOffsetZ < 0)
	//	{
	//		m_speedX += m_diveSpeedLateral;
	//	}
	//}
	//
	////Jump
	//if (gData.inputMgr->isUpTapped())
	//{
	//	if (m_offsetZ == 0)//Bob must be on the surface to jump
	//	{
	//		m_speedZ += m_jumpSpeedVertical;
	//	}
	//}
	//
	////Dive
	//if (gData.inputMgr->isDownTapped())
	//{
	//	if (m_offsetZ == 0)//Bob must be on the surface to dive
	//	{
	//		m_speedZ += m_diveSpeedVertical;
	//	}
	//}
	//
	////Speed limits
	//if (m_speedX > m_surfaceSpeedLateral) m_speedX = m_surfaceSpeedLateral;
	//if (m_speedX < -m_surfaceSpeedLateral) m_speedX = -m_surfaceSpeedLateral;
	//
	////Gravity force
	//if (m_offsetZ > 0)
	//{
	//	m_speedZ += m_gravityAccelerationVertical * gData.dTime.asSeconds();
	//}
	////Archimed force
	//if (m_offsetZ < 0)
	//{
	//	m_speedZ += m_archimedeAccelerationVertical * gData.dTime.asSeconds();
	//}
	//
	////Apply speed to offset
	//m_offsetX += m_speedX * gData.dTime.asSeconds();
	//m_offsetZ += m_speedZ * gData.dTime.asSeconds();
	//
	////Returning back to the surface from a jump or a dive
	//if ((m_offsetZ <= 0 && bobIsJumping)
	//	|| (m_offsetZ >= 0 && bobIsDiving))
	//{
	//	m_speedZ = 0;
	//	m_offsetZ = 0;
	//}
	//
	////Apply offset to position
	//setPosition(glm::vec3(m_offsetX,
	//	m_offsetZ,
	//	0.f));
}