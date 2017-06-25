#pragma once
#ifndef _SCENE_H
#define _SCENE_H

#include "Globals.h"

enum SceneId
{
	SCENE_MAIN,
	SCENE_TEST,

	NB_SCENES
};

class Scene
{
public:
	virtual bool init() = 0;
	virtual void shut() = 0;
	virtual void update() = 0;
	virtual void draw() = 0;
};

#endif // _SCENE_H
