#pragma once
#ifndef _MAIN_SCENE_H
#define _MAIN_SCENE_H

#include "Globals.h"
#include "Scene.h"
#include "glutil\GPUProgram.h"
#include "glutil\FullScreenTriangle.h"
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

	GLuint			m_sceneFboId;
	GLuint			m_sceneTexId;
	GLuint			m_sceneDepthRenderbufferId;

	FullScreenTriangle	m_postProcessTriangle;

	float m_bobSurfaceSpeedLateral;
	float m_bobAirSpeedLateral;
	float m_bobJumpSpeedVertical;
	float m_bobDiveSpeedVertical;
	float m_bobDiveSpeedLateral;
	float m_bobOffsetX;
	float m_bobOffsetZ;
	float m_bobSpeedX;
	float m_bobSpeedZ;
	float m_bobGravitySpeedVertical;
	float m_bobArchimedSpeedVertical;

public:
	virtual bool	init() override;
	virtual void	shut() override;
	virtual void	update() override;
	virtual void	draw() override;
	virtual void	onEvent(const sf::Event& event) override;

private:
	void			drawScene();
	void			initSceneFBO();
	void			shutSceneFBO();
	void			updateBob();
};

#endif // _MAIN_SCENE_H
