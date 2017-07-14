#pragma once
#ifndef _FOLLOW_CAMERA_H
#define _FOLLOW_CAMERA_H

#include "Camera.h"

class SceneObject;

class FollowCamera : public Camera
{
public:
	FollowCamera()
	{
		m_followedObject = NULL;
	}

	virtual void		init() override {Camera::init(); m_followedObject = NULL;}
	virtual void		shut() override {Camera::shut(); m_followedObject = NULL;}
	virtual void		update() override;

	void				setFollowedObject(const SceneObject* followedObject)		{m_followedObject = followedObject;}

private:
	const SceneObject*	m_followedObject;
};

#endif // _FOLLOW_CAMERA_H
