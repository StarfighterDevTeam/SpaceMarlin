#include "Camera.h"

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
		if(curMousePos != m_prevMousePos && sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			static float gfCamRotSpeed = 0.0002f;
			const float fDeltaRotX = gfCamRotSpeed * gData.frameTime.asSeconds() * (curMousePos.x - m_prevMousePos.x);
			m_front = glm::rotate(mat4(), fDeltaRotX, vec3(0,1,0)) * vec4(m_front,1);

			const float fDeltaRotY = gfCamRotSpeed * gData.frameTime.asSeconds() * (curMousePos.y - m_prevMousePos.y);
			m_front = glm::rotate(mat4(), fDeltaRotY, vec3(-1,0,0)) * vec4(m_front,1);

			m_dirty = true;
		}
		// TODO: fix rotation, implement translation, use the pad...
		/*float cameraSpeed = 0.05f; // adjust accordingly
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraPos += cameraSpeed * cameraFront;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraPos -= cameraSpeed * cameraFront;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;*/
	}
	m_prevMousePos = curMousePos;
}

void Camera::draw()
{
}

void Camera::onEvent(const sf::Event& event)
{
}

void Camera::setToDefault()
{
	m_isFlyOver = false;
	m_fovRad = 45.f;
	m_near = 0.1f;
	m_far = 100.f;

	m_position = vec3(0.f, 3.f, 6.f);
	m_front = glm::normalize(-m_position);
	m_up = vec3(0.f, 1.f, 0.f);

	m_dirty = true;
}

void Camera::updateInternal()
{
	if(m_dirty)
	{
		m_dirty = false;
		m_viewMtx = glm::lookAt(m_position, m_position+m_front, m_up);
		m_projMtx = glm::perspective(m_fovRad, ((float)gData.winSizeX) / ((float)gData.winSizeY), m_near, m_far);
		m_viewProjMtx = m_projMtx * m_viewMtx;
	}
}

const mat4& Camera::getProjMtx() const
{
	const_cast<Camera*>(this)->updateInternal();
	return m_projMtx;
}

const mat4& Camera::getViewMtx() const
{
	const_cast<Camera*>(this)->updateInternal();
	return m_viewMtx;
}

const mat4& Camera::getViewProjMtx() const
{
	const_cast<Camera*>(this)->updateInternal();
	return m_viewProjMtx;
}
