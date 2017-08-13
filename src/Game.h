#pragma once
#ifndef _GAME_H
#define _GAME_H

#include "Globals.h"
#include "Scene.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Game
{
private:
	sf::Vector2i		m_screenDimensions;
	sf::Vector2i		m_gameDimensions;
	Scene*				m_scenes[NB_SCENES];
	int					m_curScene;
	aiLogStream			m_aiLogStream;
	bool				m_wireframe;
	bool				m_slowMode;

public:
	bool init(sf::RenderWindow* window);
	void run();
	void shut();

private:
	void update();
	void draw();
};

#endif	// _GAME_H
