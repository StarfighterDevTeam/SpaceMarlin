#include "Camera.h"

void Camera::setToDefault()
{
	m_fovRad = 45.f;
	m_near = 0.1f;
	m_far = 100.f;

	m_position = glm::vec3(0.f, 3.f, -6.f);
	m_front = glm::normalize(-m_position);
	m_up = glm::vec3(0.f, 1.f, 0.f);

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

const glm::mat4& Camera::getProjMtx() const
{
	const_cast<Camera*>(this)->updateInternal();
	return m_projMtx;
}

const glm::mat4& Camera::getViewMtx() const
{
	const_cast<Camera*>(this)->updateInternal();
	return m_viewMtx;
}

const glm::mat4& Camera::getViewProjMtx() const
{
	const_cast<Camera*>(this)->updateInternal();
	return m_viewProjMtx;
}
