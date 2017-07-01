#include "MainScene.h"

#include "glutil/glutil.h"

#include "SharedDefines.h"

#include "GPUProgramManager.h"
#include "Drawer.h"
#include "InputManager.h"

bool MainScene::init()
{
	// Marlin
	if(!m_bob.loadFromFile((gData.assetsPath + "/models/marlin/marlin.fbx").c_str()))
		return false;

	// Lane
	m_lane.init();

	m_camera.setPosition(glm::vec3(2.f, 3.f, -8.f));
	m_camera.setFront(glm::normalize(-m_camera.getPosition()));
	m_camera.setUp(glm::vec3(0.f, 1.f, 0.f));

	return true;
}

void MainScene::shut()
{
	m_bob.unload();
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

	//static float gfCurAngle = 0.f;
	//static float gfSpeed = 0.001f;
	//gfCurAngle += gfSpeed * gData.dTime.asMilliseconds();
	//m_bob.setModelMtx(glm::rotate(glm::mat4(), gfCurAngle, glm::vec3(0.f, 1.f, 0.f)));
	float offsetX = 0.f;
	if(gData.inputMgr->isLeftPressed())
		offsetX += gData.frameTime.asSeconds() * 0.01f;
	if(gData.inputMgr->isRightPressed())
		offsetX -= gData.frameTime.asSeconds() * 0.01f;

	m_bob.setPosition(glm::vec3(
		m_bob.getPosition().x + offsetX,
		cosf(4.f*gData.frameTime.asSeconds()),
		0.f));
	
	glm::mat4 modelViewProjMtx = m_camera.getViewProjMtx() * m_bob.getModelMtx();
	
	m_bob.draw(m_camera);

	m_lane.draw(m_camera);

	// Debug model gizmo
	gData.drawer->drawLine(m_camera, glm::vec3(0,0,0), COLOR_RED,	glm::vec3(3,0,0), COLOR_RED		);
	gData.drawer->drawLine(m_camera, glm::vec3(0,0,0), COLOR_GREEN,	glm::vec3(0,3,0), COLOR_GREEN	);
	gData.drawer->drawLine(m_camera, glm::vec3(0,0,0), COLOR_BLUE,	glm::vec3(0,0,3), COLOR_BLUE	);
}
