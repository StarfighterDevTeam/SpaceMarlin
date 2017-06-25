#pragma once
#ifndef _TEST_SCENE_H
#define _TEST_SCENE_H

#include "Globals.h"
#include "Scene.h"
#include "glutil\GPUProgram.h"
#include "glutil\Quad.h"
#include "Model.h"

class TestScene : public Scene
{
private:
	glutil::GPUProgram* m_modelProgram;
	glutil::Quad*		m_quad;
	Model				m_model;

	int					m_nbVertices;
	GLuint				m_idVertexBuffer;
	GLuint				m_idVAO;

public:
	virtual bool init() override;
	virtual void shut() override;
	virtual void update() override;
	virtual void draw() override;

private:
	void createVBOAndVAO();
};

#endif // _TEST_SCENE_H
