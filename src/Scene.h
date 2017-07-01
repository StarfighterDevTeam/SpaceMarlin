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
protected:
	Camera			m_camera;

public:
	virtual bool init();
	virtual void shut();
	virtual void update();
	virtual void draw();
	virtual void onEvent(const sf::Event& event);
};

#endif // _SCENE_H
