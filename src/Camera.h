#pragma once
#ifndef _CAMERA_H
#define _CAMERA_H

#include "Globals.h"

class Camera
{
public:
	Camera()										{setToDefault();}

	void				init();
	void				shut();
	void				update();
	void				draw();
	void				onEvent(const sf::Event& event);

	void				setToDefault();

	void				setFovRad(float fovRad)		{m_fovRad = fovRad;	m_dirty=true;}
	void				setNear(float _near)		{m_near = _near;	m_dirty=true;}
	void				setFar(float _far)			{m_far = _far;		m_dirty=true;}
	
	void				setPosition(vec3 pos)		{m_position = pos;	m_dirty=true;}
	void				setFront(vec3 front)		{m_front = front;	m_dirty=true;}
	void				setUp(vec3 up)				{m_up = up;			m_dirty=true;}

	float				getFovRad()			const	{return m_fovRad;}
	float				getNear()			const	{return m_near;}
	float				getFar()			const	{return m_far;}

	vec3				getPosition()		const	{return m_position;}
	vec3				getFront()			const	{return m_front;}
	vec3				getUp()				const	{return m_up;}
	
	const mat4&			getViewToProjMtx() const;
	const mat4&			getWorldToViewMtx() const;
	const mat4&			getWorldToProjMtx() const;

	void				setFlyOver(bool isFlyOver)	{m_isFlyOver = isFlyOver;}
	bool				isFlyOver() const			{return m_isFlyOver;}

private:
	void				updateInternal();

	bool				m_isFlyOver;
	float				m_fovRad;
	float				m_near;
	float				m_far;

	vec3				m_position;
	vec3				m_front;
	vec3				m_up;

	ivec2				m_prevMousePos;

	mutable	bool		m_dirty;
	mutable	mat4		m_viewToProjMtx;
	mutable mat4		m_projToViewMtx;

	mutable	mat4		m_worldToViewMtx;
	mutable	mat4		m_worldToProjMtx;

	mutable mat4		m_viewToWorldRotMtx;
	mutable mat4		m_projToWorldRotMtx;

	mutable float		m_aspectRatio;

	/*mat4 modelViewProjMtx = camera.getWorldToProjMtx();
	modelViewProjMtx[3] = vec4(0,0,0,1);
	program->sendUniform("gLocalToProjMtx", modelViewProjMtx);
	program->sendUniform("texPerlin", 0);

	mat4 projToViewMtx = glm::inverse(camera.getViewToProjMtx());
	mat4 viewToWorldRotMtx = mat4(glm::transpose(mat3(camera.getWorldToViewMtx())));
	mat4 projToWorldRotMtx = viewToWorldRotMtx * projToViewMtx;
	program->sendUniform("gProjToWorldRotMtx", projToWorldRotMtx);
	const float fAspectRatio = ((float)gData.winSizeX) / ((float)std::max<int>(1,gData.winSizeY));
	program->sendUniform("gAspectRatio", fAspectRatio);*/
};

#endif // _CAMERA_H
