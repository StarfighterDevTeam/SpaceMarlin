#pragma once
#ifndef _MAIN_SCENE_H
#define _MAIN_SCENE_H

#include "Globals.h"
#include "Scene.h"
#include "glutil\GPUProgram.h"
#include "glutil\Quad.h"
#include "Model.h"
#include "Lane.h"
#include "Camera.h"

class MainScene : public Scene
{
private:
	Model			m_bob;
	Lane			m_lane;
	Camera			m_camera;

public:
	virtual bool init() override;
	virtual void shut() override;
	virtual void update() override;
	virtual void draw() override;
};

#endif // _MAIN_SCENE_H
