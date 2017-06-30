#pragma once
#ifndef _CAMERA_H
#define _CAMERA_H

#include "Globals.h"

class Camera
{
public:
	Camera()										{setToDefault();}

	void				setToDefault();

	void				setFovRad(float fovRad)		{m_fovRad = fovRad;	m_dirty=true;}
	void				setNear(float _near)		{m_near = _near;		m_dirty=true;}
	void				setFar(float _far)			{m_far = _far;		m_dirty=true;}
	
	void				setPos(glm::vec3 pos)		{m_pos = pos;		m_dirty=true;}
	void				setFront(glm::vec3 front)	{m_front = front;	m_dirty=true;}
	void				setUp(glm::vec3 up)			{m_up = up;			m_dirty=true;}
	
	const glm::mat4&	getProjMtx() const;
	const glm::mat4&	getViewMtx() const;
	const glm::mat4&	getViewProjMtx() const;

private:
	void				updateInternal();

	float				m_fovRad;
	float				m_near;
	float				m_far;

	glm::vec3			m_pos;
	glm::vec3			m_front;
	glm::vec3			m_up;

	mutable	bool		m_dirty;
	mutable	glm::mat4	m_projMtx;
	mutable	glm::mat4	m_viewMtx;
	mutable	glm::mat4	m_viewProjMtx;
};

#endif // _CAMERA_H
