#include "Marlin.h"
#include "Camera.h"
#include "InputManager.h"
#include "Lane.h"

Marlin::Marlin()
{
	m_surfaceSpeedLateral					= +6.0f;//angular speed
	m_airSpeedLateral						= +6.0f;
	m_diveSpeedLateral						= +3.5f;

	m_jumpSpeedVertical						= +600.f;//acceleration
	m_diveSpeedVertical						= -150.f;//acceleration
	m_gravityAccelerationVertical			= -42.f;
	m_speedMax								= 20.f;

	m_speed									= vec3(0, 0, 0);
	m_speedMoveLateral						= vec3(0, 0, 0);
	m_tangentToGravityAtJumpTime			= vec3(0, 0, 0);

	m_state = STATE_IDLE;

	m_lastAnimationTimeSecs = -1.f;

	//temp
	setPosition				(glm::vec3(	0.f,//laterality
										2.f,//altitude
										0.f));
}

#define ALTITUDE_TO_ENTER_GRAVITY				15.f
#define ALTITUDE_TO_MOVE_ALONG_SURFACE			0.6f
#define ALTITUDE_TO_JUMP_OR_DIVE				0.4f
#define SPEED_TO_GET_STABILIZED_ON_SURFACE		5.f

bool Marlin::init()
{
	ModelResource* modelResource = new ModelResource();
	if(!modelResource->loadFromFile((gData.assetsPath + "/models/marlin/marlin.fbx").c_str()))
	{
		SAFE_DELETE(modelResource);
		return false;
	}

	m_modelResourcePtr = modelResource;
	return true;
}

void Marlin::shut()
{
	SAFE_DELETE(m_modelResourcePtr);
}

const GPUProgram* Marlin::getProgram() const
{
	return gData.gpuProgramMgr->getProgram(PROG_MARLIN);
}

void Marlin::sendUniforms(const GPUProgram* program, const Camera& camera) const
{
	ModelInstance::sendUniforms(program, camera);

	program->sendUniform("gSpeed", vec3(m_speedMoveLateral.x, 0.f, m_speedMoveLateral.y));
	//program->sendUniform("texAlbedo", 0);
	//program->sendUniform("gTime", gData.curFrameTime.asSeconds());
}

void Marlin::addLane(const Lane* lane)
{
	assert(lane);
	m_lanes.push_back(lane);
}

void Marlin::update()
{
	if (m_lastAnimationTimeSecs < 0.f)
		m_lastAnimationTimeSecs = gData.curFrameTime.asSeconds();

	static float ANIMATIONS_PER_SECOND = 60.0f;

	// do rendering to get ANIMATIONS_PER_SECOND
	while (m_lastAnimationTimeSecs <= gData.curFrameTime.asSeconds())
	{
		//printf("position Marlin: %f, %f, %f\n", getPosition().x, getPosition().y, getPosition().z);

		m_speedMoveLateral = vec3(0, 0, 0);
		m_state = STATE_IDLE;

		if (!m_lanes.empty())
		{
			//find the closest lane (size of the normal vector to lane)
			int laneIndex = -1;
			float minAltitude = ALTITUDE_TO_ENTER_GRAVITY;
			float altitude;

			const int laneVectorSize = (int)m_lanes.size();
			for (int i = 0; i < laneVectorSize; i++)
			{	
				altitude = m_lanes[i]->getDistToSurface(getPosition());

				if (abs(altitude) < minAltitude)
				{
					minAltitude = abs(altitude);
					laneIndex = i;
				}
			}

			//interact only with the closest lane, if close enough to enter gravity
			if (laneIndex >= 0)
			{
				const Lane* lane = m_lanes[laneIndex];
				altitude = lane->getDistToSurface(getPosition());

				//const vec3 gravityVector = lane->getGravityVector(getPosition());
				//const vec3 tangentToGravity = vec3(gravityVector.y, -gravityVector.x, 0);

				vec3 gravityVector, tangentToGravity, uselessVector;
				lane->getCoordinateSystem(getPosition(), tangentToGravity, gravityVector, uselessVector);
				//test
				gravityVector = vec3(0, 1, 0);
				tangentToGravity = vec3(1, 0, 0);

				//Jump
				if (gData.inputMgr->isUpPressed() && m_state != STATE_JUMPING)
				{
					if (abs(altitude) < ALTITUDE_TO_JUMP_OR_DIVE)//Bob must be on the surface to jump
					{
						m_speed += gravityVector * m_jumpSpeedVertical * 1.0f / ANIMATIONS_PER_SECOND;
						m_tangentToGravityAtJumpTime = tangentToGravity;//save the system at the moment of jump to keep moving in this system while in the air
					}
				}

				//Dive
				if (gData.inputMgr->isDownPressed())
				{
					if (abs(altitude) < ALTITUDE_TO_JUMP_OR_DIVE)//Bob must be on the surface to jump
					{
						m_speed += gravityVector * m_diveSpeedVertical * 1.0f / ANIMATIONS_PER_SECOND;
						m_state = STATE_DIVING;
					}
				}

				//Gravity
				if (altitude > 0)
				{
					m_speed += gravityVector * m_gravityAccelerationVertical * 1.0f / ANIMATIONS_PER_SECOND;
				}
				else if (altitude < 0)
				{
					m_speed += gravityVector * (-m_gravityAccelerationVertical) * 1.0f / ANIMATIONS_PER_SECOND;//using an inverted gravity to simulate an Archimedes' thrust
				}

				//Move left
				if (gData.inputMgr->isLeftPressed())
				{
					if (abs(altitude) < ALTITUDE_TO_MOVE_ALONG_SURFACE)//bob is near the surface
					{
						m_speedMoveLateral -= m_surfaceSpeedLateral * tangentToGravity * 1.0f / ANIMATIONS_PER_SECOND;
					}
					else if (altitude > 0)//bob is in the air
					{
						m_speedMoveLateral -= m_airSpeedLateral * tangentToGravity * 1.0f / ANIMATIONS_PER_SECOND;
					}
					else//if (altitude < 0)//bob is underwater
					{
						m_speedMoveLateral -= m_diveSpeedLateral * tangentToGravity * 1.0f / ANIMATIONS_PER_SECOND;
					}
				}

				//Move right
				if (gData.inputMgr->isRightPressed())
				{
					if (abs(altitude) < ALTITUDE_TO_MOVE_ALONG_SURFACE)//bob is near the surface
					{
						m_speedMoveLateral += m_surfaceSpeedLateral * tangentToGravity * 1.0f / ANIMATIONS_PER_SECOND;
					}
					else if (altitude > 0)//bob is in the air
					{
						m_speedMoveLateral += m_airSpeedLateral * tangentToGravity * 1.0f / ANIMATIONS_PER_SECOND;
					}
					else//if (altitude < 0)//bob is underwater
					{
						m_speedMoveLateral += m_diveSpeedLateral * tangentToGravity * 1.0f / ANIMATIONS_PER_SECOND;
					}
				}

				//speed limit
				if (dot(m_speed,m_speed) > m_speedMax*m_speedMax)
				{
					m_speed = normalize(m_speed) * m_speedMax;
				}

				//apply speed
				move((m_speed * 1.0f / ANIMATIONS_PER_SECOND)
					+ m_speedMoveLateral);

				//moving through the lane's surface? -> loose momentum
				//float altitudeNew, angleNew;
				//getAltitudeAndAngleToLane(lane, altitudeNew, angleNew);
				float altitudeNew = lane->getDistToSurface(getPosition());

				//printf("altitude: %f", altitude);
				//const vec3 gravityVectorNew = normalize(getPosition() - lane->getPosition());
				vec3 gravityVectorNew = lane->getNormalToSurface(getPosition());
				//test
				gravityVectorNew = vec3(0, 1, 0);

				if ((altitude > 0 && altitudeNew <= 0)//moving through lane's surface?
					|| (altitude < 0 && altitudeNew >= 0))
				{
					if (!((altitude < 0 && m_state == STATE_JUMPING) || (altitude > 0 && m_state == STATE_DIVING)))//not jumping from below the surface or diving from below the surface?
					{
						//kill tiny oscillations by sticking the Marlin right on the lane surface
						if (glm::dot(m_speed,m_speed) < SPEED_TO_GET_STABILIZED_ON_SURFACE*SPEED_TO_GET_STABILIZED_ON_SURFACE)//the stronger the gravity, the higher the value must be
						{
							const vec3 vectorToStickToLane = gravityVectorNew * (-altitudeNew);

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

				// Reset up vector according to gravity to lane
				const vec3 oldUpVector = vec3(m_localToWorldMtx[1]);
				const vec3 newUpVector = glm::normalize(oldUpVector + (gravityVectorNew - oldUpVector) * 3.f / ANIMATIONS_PER_SECOND);
				
				m_localToWorldMtx[1] = vec4(newUpVector,0);
				m_localToWorldMtx[0] = vec4(cross(newUpVector, vec3(m_localToWorldMtx[2])), 0);
			}

			//Death & respawn
			if (getPosition().y < -10.f)
			{
				setPosition(vec3(0.f, 4.f, 0.f));
				m_speed = vec3(0.f, 0.f, 0.f);
			}
		}

		m_lastAnimationTimeSecs += (1.0f / ANIMATIONS_PER_SECOND);
	}
}