#pragma once
#ifndef _CAMERA_H
#define _CAMERA_H

#include "Globals.h"

class SceneObject;
class GPUProgram;

class Camera
{
public:
	Camera()											{setToDefault();}
	virtual ~Camera()									{}

	virtual void		init();
	virtual void		shut();
	virtual void		update();
	virtual void		draw();
	virtual void		drawAfter();
	virtual void		onEvent(const sf::Event& event);

	void				setToDefault();

	void				setFovRad(float fovRad)			{m_fovRad = fovRad;	m_dirty=true;}
	void				setNear(float _near)			{m_near = _near;	m_dirty=true;}
	void				setFar(float _far)				{m_far = _far;		m_dirty=true;}
	
	void				setPosition(const vec3& pos)	{m_position = pos;	m_dirty=true;}
	void				setFront(const vec3& front)		{m_front = front;	m_dirty=true;}
	void				setUp(const vec3& up)			{m_up = up;			m_dirty=true;}

	float				getFovRad()			const		{return m_fovRad;}
	float				getNear()			const		{return m_near;}
	float				getFar()			const		{return m_far;}

	const vec3&			getPosition()		const		{return m_position;}
	const vec3&			getFront()			const		{return m_front;}
	const vec3&			getUp()				const		{return m_up;}
	
	const mat4&			getViewToProjMtx() const;
	const mat4&			getProjToViewMtx() const;
	const mat4&			getWorldToViewMtx() const;
	const mat4&			getWorldToProjMtx() const;

	const mat4&			getViewToWorldRotMtx() const;
	const mat4&			getProjToWorldRotMtx() const;

	void				setFlyOver(bool isFlyOver)		{m_isFlyOver = isFlyOver;}
	bool				isFlyOver() const				{return m_isFlyOver;}

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
};

#endif // _CAMERA_H
