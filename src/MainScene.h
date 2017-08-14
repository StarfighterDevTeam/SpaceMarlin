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
#include "Atom.h"
#include "Score.h"

class FollowCamera;

class MainScene : public Scene
{
private:
	Skybox				m_skybox;
	Marlin				m_bob;
	Score				m_score;
	std::vector<Lane>	m_lanes;
	FollowCamera*		m_camera;
	ModelResource		m_atomBlueprint;
	std::vector<Atom>	m_atoms;

	// Score playing
	sf::Time			m_curScoreTime;
	int					m_beatCount;
	int					m_measureCount;
	float				m_beatsPerMinute;

	// Low-level rendering
	GLuint				m_sceneRefractionTexId;
	GLuint				m_sceneFboId;
	GLuint				m_sceneTexId;
	GLuint				m_sceneDepthRenderbufferId;

	FullScreenTriangle	m_postProcessTriangle;

#ifdef _USE_ANTTWEAKBAR
	// Edition
	TwBar*				m_editionBar;
	bool				m_editionMode;

	// Debug
	TwBar*				m_debugBar;
	bool				m_bDebugDrawRefraction;
#endif

public:
	virtual bool	init() override;
	virtual void	shut() override;
	virtual void	update() override;
	virtual void	draw() override;
	virtual void	drawAfter() override;
	virtual void	onEvent(const sf::Event& event) override;
	virtual Camera*	createCamera() const override;

private:
	void			initSceneFBO();
	void			shutSceneFBO();
};

#endif // _MAIN_SCENE_H
