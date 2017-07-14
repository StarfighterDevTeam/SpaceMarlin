#pragma once
#ifndef _SCENE_H
#define _SCENE_H

#include "Globals.h"
#include "Camera.h"

enum SceneId
{
	SCENE_MAIN,
	SCENE_TEST,

	NB_SCENES
};

class Scene
{
private:
	Camera*			m_camera;

public:
	virtual bool		init();
	virtual void		shut();
	virtual void		update();
	virtual void		draw();
	virtual void		onEvent(const sf::Event& event);
	virtual Camera*		createCamera() const	{return new Camera();}

	Camera*				getCamera()				{return m_camera;}
	const Camera*		getCamera() const		{return m_camera;}
};

#endif // _SCENE_H
