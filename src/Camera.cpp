#include "Camera.h"
#include "InputManager.h"
#include <glm/gtx/euler_angles.hpp>

void Camera::init()
{
	setToDefault();
	m_prevMousePos = ivec2(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);
}

void Camera::shut()
{
}

void Camera::update()
{
	ivec2 curMousePos = ivec2(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);
	if(m_isFlyOver)
	{
		const float dTime = gData.dTime.asSeconds();
		static float gfCamRotSpeed = 0.1f;
		static float gfCamTransSpeed = 10.f;
			
		// Rotation with left click
		if(gData.window->hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Left) && curMousePos != m_prevMousePos)
		{
			const float fDeltaRotX = gfCamRotSpeed * dTime * (curMousePos.x - m_prevMousePos.x);
			const float fDeltaRotY = gfCamRotSpeed * dTime * (curMousePos.y - m_prevMousePos.y);
			
			const vec3 cameraSpaceOffset = vec3(fDeltaRotX, -fDeltaRotY, 0);
			const mat3 cameraToWorldRotMtx = glm::transpose(mat3(m_worldToViewMtx));
			const vec3 worldSpaceOffset = cameraToWorldRotMtx * cameraSpaceOffset;
			setFront(glm::normalize(m_front + worldSpaceOffset));
			const vec3 X = glm::cross(m_front, m_up);
			setUp(glm::cross(X, m_front));
		}

		// Roll (rotation with right click)
		if(sf::Mouse::isButtonPressed(sf::Mouse::Right) && curMousePos != m_prevMousePos)
		{
			const float fAngle = gfCamRotSpeed * dTime * (curMousePos.x - m_prevMousePos.x);
			const mat3 cameraSpaceRotMtx = glm::orientate3(fAngle);
			const mat3 worldToCameraRotMtx = mat3(m_worldToViewMtx);
			const mat3 cameraToWorldRotMtx = glm::transpose(worldToCameraRotMtx);
			setUp(cameraToWorldRotMtx * cameraSpaceRotMtx * worldToCameraRotMtx * m_up);
		}

		// Translation
		// - front/back
		if(gData.inputMgr->isDebugCamFrontPressed())
			setPosition(m_position + gfCamTransSpeed * dTime * m_front);
		if(gData.inputMgr->isDebugCamBackPressed())
			setPosition(m_position - gfCamTransSpeed * dTime * m_front);

		// - left/right
		const vec3 X = glm::cross(m_front, m_up);
		if(gData.inputMgr->isDebugCamLeftPressed())
			setPosition(m_position - gfCamTransSpeed * dTime * X);
		if(gData.inputMgr->isDebugCamRightPressed())
			setPosition(m_position + gfCamTransSpeed * dTime * X);

		// - up/down
		if(gData.inputMgr->isDebugCamUpPressed())
			setPosition(m_position + gfCamTransSpeed * dTime * m_up);
		if(gData.inputMgr->isDebugCamDownPressed())
			setPosition(m_position - gfCamTransSpeed * dTime * m_up);
	}
	m_prevMousePos = curMousePos;
}

void Camera::draw()
{
}

void Camera::drawAfter()
{
}

void Camera::onEvent(const sf::Event& event)
{
	if (event.type == sf::Event::Resized)
		m_dirty = true;
}

void Camera::setToDefault()
{
	m_isFlyOver = false;
	m_fovRad = 45.f;
	m_near = 0.1f;
	m_far = 100.f;

	m_position = vec3(0.f, 0.f, 6.f);
	m_front = glm::normalize(-m_position);
	m_up = vec3(0.f, 1.f, 0.f);

	m_dirty = true;
}

void Camera::updateInternal()
{
	if(m_dirty)
	{
		m_dirty = false;
		m_worldToViewMtx = glm::lookAt(m_position, m_position+m_front, m_up);
		m_viewToProjMtx = glm::perspective(m_fovRad, ((float)gData.winSizeX) / ((float)gData.winSizeY), m_near, m_far);

		m_worldToProjMtx = m_viewToProjMtx * m_worldToViewMtx;
		m_projToViewMtx = glm::inverse(m_viewToProjMtx);

		m_viewToWorldRotMtx = mat4(glm::transpose(mat3(m_worldToViewMtx)));;
		m_projToWorldRotMtx = m_viewToWorldRotMtx * m_projToViewMtx;

		m_aspectRatio = ((float)gData.winSizeX) / ((float)std::max<int>(1,gData.winSizeY));
	}
}

const mat4& Camera::getViewToProjMtx() const
{
	const_cast<Camera*>(this)->updateInternal();
	return m_viewToProjMtx;
}

const mat4& Camera::getWorldToViewMtx() const
{
	const_cast<Camera*>(this)->updateInternal();
	return m_worldToViewMtx;
}

const mat4& Camera::getWorldToProjMtx() const
{
	const_cast<Camera*>(this)->updateInternal();
	return m_worldToProjMtx;
}
