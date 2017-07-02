#include "MainScene.h"
#include "glutil/glutil.h"
#include "SharedDefines.h"
#include "GPUProgramManager.h"
#include "Drawer.h"
#include "InputManager.h"

//#define _USE_SKYBOX

bool MainScene::init()
{
	if(!Scene::init())
		return false;

	// Skybox
	std::string cubemapFilenames[6] = {
		gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "purple-nebula-complex_left2.png",
		gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "purple-nebula-complex_right1.png",
		gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "purple-nebula-complex_top3.png",
		gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "purple-nebula-complex_bottom4.png",
		gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "purple-nebula-complex_front5.png",
		gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "purple-nebula-complex_back6.png",
	};

	//std::string cubemapFilenames[6] = {
	//	gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "debug_left.png",
	//	gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "debug_right.png",
	//	gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "debug_top.png",
	//	gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "debug_bottom.png",
	//	gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "debug_front.png",
	//	gData.assetsPath + SDIR_SEP "textures" SDIR_SEP "debug_back.png",
	//};
	
#ifdef _USE_SKYBOX
	if(!m_skybox.loadFromFiles(
		cubemapFilenames[0].c_str(),
		cubemapFilenames[1].c_str(),
		cubemapFilenames[2].c_str(),
		cubemapFilenames[3].c_str(),
		cubemapFilenames[4].c_str(),
		cubemapFilenames[5].c_str()
		))
		return false;
#else
	if(!m_background.load())
		return false;
#endif

	// Bob
	if(!m_bob.loadFromFile((gData.assetsPath + "/models/marlin/marlin.fbx").c_str()))
		return false;

	// Lane
	m_lane.init();

	m_camera.setPosition(vec3(-2, 3, 8));
	m_camera.setFront(normalize(-m_camera.getPosition()));
	m_camera.setUp(vec3(0,1,0));

	return true;
}

void MainScene::shut()
{
	Scene::shut();

	m_background.unload();
	m_skybox.unload();
	m_bob.unload();
	m_lane.shut();
}

void MainScene::update()
{
	Scene::update();

	m_lane.update();

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
}

void MainScene::draw()
{
	Scene::draw();

	glEnable(GL_DEPTH_TEST);

	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

#ifdef _USE_SKYBOX
	m_skybox.draw(m_camera);
#else
	m_background.draw(m_camera);
#endif

	glm::mat4 modelViewProjMtx = m_camera.getViewProjMtx() * m_bob.getModelMtx();
	
	m_bob.draw(m_camera);

	m_lane.draw(m_camera);

	// Debug model gizmo
	gData.drawer->drawLine(m_camera, glm::vec3(0,0,0), COLOR_RED,	glm::vec3(3,0,0), COLOR_RED		);
	gData.drawer->drawLine(m_camera, glm::vec3(0,0,0), COLOR_GREEN,	glm::vec3(0,3,0), COLOR_GREEN	);
	gData.drawer->drawLine(m_camera, glm::vec3(0,0,0), COLOR_BLUE,	glm::vec3(0,0,3), COLOR_BLUE	);
}

void MainScene::onEvent(const sf::Event& event)
{
	Scene::onEvent(event);
}
