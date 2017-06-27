#pragma once
#ifndef _MAIN_SCENE_H
#define _MAIN_SCENE_H

#include "Globals.h"
#include "Scene.h"
#include "glutil\GPUProgram.h"
#include "glutil\Quad.h"
#include "Model.h"

class MainScene : public Scene
{
private:
	Model			m_model;

public:
	virtual bool init() override;
	virtual void shut() override;
	virtual void update() override;
	virtual void draw() override;
};

#endif // _MAIN_SCENE_H
