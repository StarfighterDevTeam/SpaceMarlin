#pragma once
#ifndef _MAIN_SCENE_H
#define _MAIN_SCENE_H

#include "Globals.h"
#include "Scene.h"
#include "glutil\GPUProgram.h"
#include "glutil\Quad.h"
#include "Model.h"
#include "Lane.h"
#include "Skybox.h"
#include "Background.h"

class MainScene : public Scene
{
private:
	Background		m_background;
	Skybox			m_skybox;
	Model			m_bob;
	Lane			m_lane;

	float m_bobSurfaceSpeedLateral;
	float m_bobAirSpeedLateral;
	float m_bobJumpSpeedVertical;
	float m_bobOffsetX;
	float m_bobOffsetZ;
	bool m_bobIsJumping;

public:
	virtual bool init() override;
	virtual void shut() override;
	virtual void update() override;
	virtual void draw() override;
	virtual void onEvent(const sf::Event& event);
};

#endif // _MAIN_SCENE_H
