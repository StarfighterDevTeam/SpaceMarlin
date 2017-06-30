#include "MainScene.h"

#include "glutil/glutil.h"

#include "SharedDefines.h"

#include "GPUProgramManager.h"
#include "Drawer.h"

bool MainScene::init()
{
	// Marlin
	if(!m_marlin.loadFromFile("media/models/marlin/marlin.fbx"))
		return false;

	// Lane
	m_lane.init();

	m_camera.setToDefault();

	return true;
}

void MainScene::shut()
{
	m_marlin.unload();
	m_lane.shut();
}

void MainScene::update()
{
	m_lane.update();
}

void MainScene::draw()
{
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	const GPUProgram* modelProgram = gData.gpuProgramMgr->getProgram(PROG_MODEL);
	modelProgram->use();

	glm::mat4 modelViewProjMtx;
	static float gfCurAngle = 0.f;
	static float gfSpeed = 0.001f;
	gfCurAngle += gfSpeed * gData.dTime.asMilliseconds();
	glm::mat4 modelMtx = glm::rotate(glm::mat4(), gfCurAngle, glm::vec3(0.f, 1.f, 0.f));
	
	modelViewProjMtx = m_camera.getViewProjMtx() * modelMtx;
	modelProgram->sendUniform("gModelViewProjMtx", modelViewProjMtx);

	modelProgram->sendUniform("texAlbedo", 0);
	modelProgram->sendUniform("gTime", gData.frameTime.asSeconds());

	m_marlin.draw();

	m_lane.draw(modelViewProjMtx);

	// Debug model gizmo
	gData.drawer->drawLine(modelViewProjMtx, glm::vec3(0,0,0), COLOR_RED,	glm::vec3(3,0,0), COLOR_RED		);
	gData.drawer->drawLine(modelViewProjMtx, glm::vec3(0,0,0), COLOR_GREEN,	glm::vec3(0,3,0), COLOR_GREEN	);
	gData.drawer->drawLine(modelViewProjMtx, glm::vec3(0,0,0), COLOR_BLUE,	glm::vec3(0,0,3), COLOR_BLUE	);
}
