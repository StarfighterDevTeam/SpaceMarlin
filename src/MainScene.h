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
#include "Marlin.h"

class MainScene : public Scene
{
private:
	Skybox			m_skybox;
	Marlin			m_bob;
	Lane			m_lane;

	GLuint			m_sceneRefractionTexId;
	GLuint			m_sceneFboId;
	GLuint			m_sceneTexId;
	GLuint			m_sceneDepthRenderbufferId;

	FullScreenTriangle	m_postProcessTriangle;

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
