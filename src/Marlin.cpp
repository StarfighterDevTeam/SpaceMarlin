#include "Marlin.h"
#include "Camera.h"
#include "InputManager.h"
#include "Lane.h"

Marlin::Marlin()
{
	m_surfaceSpeedLateral					= +2.0f;//angular speed
	m_airSpeedLateral						= +4.0f;
	m_diveSpeedLateral						= +2.5f;

	m_jumpSpeedVertical						= +60.f;//acceleration
	m_diveSpeedVertical						= -40.f;//acceleration
	m_gravityAccelerationVertical			= -30.f;
	m_archimedeAccelerationVertical			= +30;
	m_offsetX								= 0;
	m_offsetZ								= 0;
	m_speedX								= 0;
	m_speedZ								= 0;
	m_speedMax								= 20.f;

	m_speed									= vec3(0, 0, 0);
	m_speedMoveLateral						= vec3(0, 0, 0);
	m_vectorPerpendicularToLaneAtJumpTime	= vec3(0, 0, 0);

	m_state = STATE_IDLE;

	m_lastAnimationTimeSecs = -1.f;

	//temp
	setPosition				(glm::vec3(	0.f,//laterality
										2.f,//altitude
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

void Marlin::addLane(const Lane* lane)
{
	assert(lane);
	m_lanes.push_back(lane);
}

float Marlin::getDistanceSquaredToLane(const Lane* lane) const
{
	vec3 d = getPosition() - lane->getPosition();

	float distSquared = d.x*d.x + d.y*d.y + d.z*d.z;

	return distSquared;
}

void Marlin::getAltitudeAndAngleToLane(const Lane* lane, float &altitude, float &angle) const
{
	const float a = getPosition().x - lane->getPosition().x;
	const float b = getPosition().y - lane->getPosition().y;

	float e = getPosition().x;
	float f = getPosition().y;
	float g = lane->getPosition().x;
	float h = lane->getPosition().y;

	if (a == 0 && b == 0)
	{
		altitude = 0;
		angle = 0;
		return;
	}

	float distance = (a * a) + (b * b);
	distance = sqrt(distance);

	//angle = acos(a / distance);
	//
	//if (b < 0)
	//{
	//	angle = -angle;
	//}

	angle = atan2(b, a);

	altitude = distance - lane->getCylinderRadius(angle);
}

float Marlin::getNormalizedSpeed() const
{
	return sqrt(m_speed.x*m_speed.x + m_speed.y*m_speed.y + m_speed.z*m_speed.z);
}

#define ALTITUDE_TO_ENTER_GRAVITY				8.f
#define ALTITUDE_TO_MOVE_ALONG_SURFACE			1.f//below 1.f, lateral movement won't be smooth, because it sometimes go to altitude=0.68 just with left/right inputs
#define SPEED_TO_GET_STABILIZED_ON_SURFACE		5.f

sf::Clock simulationStart;

void Marlin::update()
{
	if (m_lastAnimationTimeSecs < 0.f)
		m_lastAnimationTimeSecs = gData.curFrameTime.asSeconds();

	static float ANIMATIONS_PER_SECOND = 60.0f;

	// do rendering to get ANIMATIONS_PER_SECOND
	while (m_lastAnimationTimeSecs <= gData.curFrameTime.asSeconds())
	{
		m_speedMoveLateral = vec3(0, 0, 0);
		m_state = STATE_IDLE;

		//Test
		if (!m_lanes.empty() && simulationStart.getElapsedTime().asSeconds() > 6.f)
		{
			size_t laneVectorSize = m_lanes.size();
			for (size_t i = 0; i < laneVectorSize; i++)
			{
				const Lane* lane = m_lanes[i];

				//gravity acceleration
				float altitude, angle;
				getAltitudeAndAngleToLane(lane, altitude, angle);

				if (altitude < ALTITUDE_TO_ENTER_GRAVITY)
				{
					vec3 vectorToLane = getPosition() - lane->getPosition();
					vectorToLane = normalize(vectorToLane);

					vec3 vectorPerpendicularToLane = vec3(vectorToLane.x * cos(-3.1415 / 2) - vectorToLane.y * sin(-3.1415 / 2),
						vectorToLane.x * sin(-3.1415 / 2) - vectorToLane.y * cos(-3.1415 / 2),
						vectorToLane.z);

					//Jump
					if (gData.inputMgr->isUpPressed())
					{
						if (abs(altitude) < ALTITUDE_TO_MOVE_ALONG_SURFACE)//Bob must be on the surface to jump
						{
							m_speed += vectorToLane * m_jumpSpeedVertical * gData.dTime.asSeconds();
							m_state = STATE_JUMPING;
							m_vectorPerpendicularToLaneAtJumpTime = vectorPerpendicularToLane;//save the system at the moment of jump to keep moving in this system while in the air
						}
					}

					//Dive
					if (gData.inputMgr->isDownPressed())
					{
						if (abs(altitude) < ALTITUDE_TO_MOVE_ALONG_SURFACE)//Bob must be on the surface to jump
						{
							m_speed += vectorToLane * m_diveSpeedVertical * gData.dTime.asSeconds();
							m_state = STATE_DIVING;
						}
					}

					//Gravity
					static float gravity = -25.f;//-25.f;
					if (altitude > 0)
					{
						m_speed += vectorToLane * gravity * gData.dTime.asSeconds();
					}
					else if (altitude < 0)
					{
						m_speed += vectorToLane * (-gravity) * gData.dTime.asSeconds();//using an inverted gravity to simulate an Archimedes' thrust
					}

					//Move left
					if (gData.inputMgr->isLeftPressed())
					{
						if (abs(altitude) < ALTITUDE_TO_MOVE_ALONG_SURFACE)//bob is near the surface
						{
							float angleAfterMove = angle + m_surfaceSpeedLateral * gData.dTime.asSeconds();

							float speedX = lane->getCylinderRadius(0)			* (cos(angleAfterMove) - cos(angle));
							float speedY = lane->getCylinderRadius(3.1415f / 2)	* (sin(angleAfterMove) - sin(angle));

							m_speedMoveLateral = vec3(speedX, speedY, 0);
						}
						else if (altitude > 0)//bob is in the air
						{
							m_speedMoveLateral -= m_airSpeedLateral * m_vectorPerpendicularToLaneAtJumpTime * gData.dTime.asSeconds();
						}
						else//if (altitude < 0)//bob is underwater
						{
							m_speedMoveLateral.x += m_diveSpeedLateral * cos(angle) * gData.dTime.asSeconds();
							m_speedMoveLateral.y += m_diveSpeedLateral * sin(angle) * gData.dTime.asSeconds();
						}
					}

					//Move right
					if (gData.inputMgr->isRightPressed())
					{
						if (abs(altitude) < ALTITUDE_TO_MOVE_ALONG_SURFACE)//bob is near the surface
						{
							float angleAfterMove = angle - m_surfaceSpeedLateral * gData.dTime.asSeconds();

							float speedX = lane->getCylinderRadius(0)			* (cos(angleAfterMove) - cos(angle));
							float speedY = lane->getCylinderRadius(3.1415f / 2)	* (sin(angleAfterMove) - sin(angle));

							m_speedMoveLateral = vec3(speedX, speedY, 0);
						}
						else if (altitude > 0)//bob is in the air
						{
							m_speedMoveLateral += m_airSpeedLateral * m_vectorPerpendicularToLaneAtJumpTime * gData.dTime.asSeconds();
						}
						else//if (altitude < 0)//bob is underwater
						{
							m_speedMoveLateral.x -= m_diveSpeedLateral * cos(angle) * gData.dTime.asSeconds();
							m_speedMoveLateral.y -= m_diveSpeedLateral * sin(angle) * gData.dTime.asSeconds();
						}
					}

					//speed limit
					if (getNormalizedSpeed() > m_speedMax)
					{
						m_speed = normalize(m_speed) * m_speedMax;
					}

					//apply speed
					move((m_speed * gData.dTime.asSeconds())
						+ m_speedMoveLateral);

					//moving through the lane's surface? -> loose momentum
					float altitudeNew, angleNew;
					getAltitudeAndAngleToLane(lane, altitudeNew, angleNew);

					printf("altitude: %f | angle:%f\n", altitude, angle);

					if ((altitude > 0 && altitudeNew <= 0)//moving through lane's surface?
						|| (altitude < 0 && altitudeNew >= 0))
					{
						if (!((altitude < 0 && m_state == STATE_JUMPING) || (altitude > 0 && m_state == STATE_DIVING)))//not jumping from below the surface or diving from below the surface?
						{
							//kill tiny oscillations by sticking the Marlin right on the lane surface
							if (getNormalizedSpeed() < SPEED_TO_GET_STABILIZED_ON_SURFACE)//the stronger the gravity, the higher the value must be
							{
								vec3 vectorToLaneNew = getPosition() - lane->getPosition();
								vectorToLaneNew = normalize(vectorToLane);

								vec3 vectorToStickToLane = vectorToLaneNew * (-altitudeNew);

								move(vectorToStickToLane);

								m_speed = vec3(0, 0, 0);

								//printf("STICK TO LANE. ");
							}

							//loose momentum
							//m_speed = vec3(0, 0, 0);
							m_speed *= 0.5f;
						}

						//printf("LOOSE MOMENTUM. ");
					}

					//printf("\n");
				}
			}
		}

		m_lastAnimationTimeSecs += (1.0f / ANIMATIONS_PER_SECOND);
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