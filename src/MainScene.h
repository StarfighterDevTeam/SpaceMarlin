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

class FollowCamera;

class MainScene : public Scene
{
private:
	Skybox				m_skybox;
	Marlin				m_bob;
	std::vector<Lane>	m_lanes;
	FollowCamera*		m_camera;

	GLuint				m_sceneRefractionTexId;
	GLuint				m_sceneFboId;
	GLuint				m_sceneTexId;
	GLuint				m_sceneDepthRenderbufferId;

	FullScreenTriangle	m_postProcessTriangle;

public:
	virtual bool	init() override;
	virtual void	shut() override;
	virtual void	update() override;
	virtual void	draw() override;
	virtual void	drawAfter() override;
	virtual void	onEvent(const sf::Event& event) override;
	virtual Camera*	createCamera() const override;

private:
	void			drawScene();
	void			initSceneFBO();
	void			shutSceneFBO();
	void			updateBob();
};

#endif // _MAIN_SCENE_H
