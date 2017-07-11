#include "MainScene.h"
#include "glutil/glutil.h"
#include "GPUProgramManager.h"
#include "Drawer.h"
#include "InputManager.h"

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
	
	if(!m_skybox.loadFromFiles(
		cubemapFilenames[0].c_str(),
		cubemapFilenames[1].c_str(),
		cubemapFilenames[2].c_str(),
		cubemapFilenames[3].c_str(),
		cubemapFilenames[4].c_str(),
		cubemapFilenames[5].c_str()
		))
		return false;

	// Bob
	if(!m_bob.loadFromFile((gData.assetsPath + "/models/marlin/marlin.fbx").c_str()))
		return false;

	// Lane
	m_lane.init();

	m_camera.setPosition(vec3(-2, 3, 8));
	m_camera.setFront(normalize(-m_camera.getPosition()));
	m_camera.setUp(vec3(0,1,0));

	initSceneFBO();
	m_postProcessTriangle.init();

	return true;
}

void MainScene::shut()
{
	Scene::shut();

	m_skybox.unload();
	m_bob.unload();
	m_lane.shut();

	shutSceneFBO();
	m_postProcessTriangle.shut();
}

void MainScene::update()
{
	Scene::update();

	m_lane.update();

	//static float gfCurAngle = 0.f;
	//static float gfSpeed = 0.001f;
	//gfCurAngle += gfSpeed * gData.dTime.asMilliseconds();
	//m_bob.setModelMtx(glm::rotate(glm::mat4(), gfCurAngle, glm::vec3(0.f, 1.f, 0.f)));

	m_bob.update();
}

void MainScene::draw()
{
	Scene::draw();

	// Draw scene to FBO
	{
		glutil::BindFramebuffer bindSceneFbo(m_sceneFboId);

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);

		// Cull triangles which normal is not towards the camera
		glEnable(GL_CULL_FACE);
		
		m_skybox.draw(m_camera);
	
		glm::mat4 modelViewProjMtx = m_camera.getWorldToProjMtx() * m_bob.getModelMtx();
	
		m_bob.draw(m_camera);

		m_lane.draw(m_camera, m_skybox.getSkyTexId());

		// Debug model gizmo
		gData.drawer->drawLine(m_camera, glm::vec3(0,0,0), COLOR_RED,	glm::vec3(3,0,0), COLOR_RED		);
		gData.drawer->drawLine(m_camera, glm::vec3(0,0,0), COLOR_GREEN,	glm::vec3(0,3,0), COLOR_GREEN	);
		gData.drawer->drawLine(m_camera, glm::vec3(0,0,0), COLOR_BLUE,	glm::vec3(0,0,3), COLOR_BLUE	);
	}

	// Post-process
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		const GPUProgram* tonemappingProgram = gData.gpuProgramMgr->getProgram(PROG_TONEMAPPING);
		tonemappingProgram->use();
		tonemappingProgram->sendUniform("gTime", gData.curFrameTime.asSeconds());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_sceneTexId);
		tonemappingProgram->sendUniform("texScene", 0);

		m_postProcessTriangle.draw();

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}
}

void MainScene::onEvent(const sf::Event& event)
{
	Scene::onEvent(event);

	if(event.type == sf::Event::Resized)
	{
		shutSceneFBO();
		initSceneFBO();
	}
}

void MainScene::initSceneFBO()
{
	// Setup scene FBO
	m_sceneTexId = glutil::createTextureRGBAF(gData.winSizeX, gData.winSizeY, true);
	m_sceneDepthRenderbufferId = glutil::createRenderbufferDepth(gData.winSizeX, gData.winSizeY);

	glGenFramebuffers(1, &m_sceneFboId);
	glutil::BindFramebuffer fboBinding(m_sceneFboId);

	// - attach the textures:
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneTexId, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_sceneDepthRenderbufferId);

	GL_CHECK();

	// - specify the draw buffers:
	static const GLenum drawBuffers[] = {
		GL_COLOR_ATTACHMENT0
	};

	glDrawBuffers(sizeof(drawBuffers) / sizeof(GLenum), drawBuffers);

	// - check the FBO:
	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(fboStatus == GL_FRAMEBUFFER_COMPLETE)
		logSuccess("FBO creation");
	else
		logError("FBO not complete");
}

void MainScene::shutSceneFBO()
{
	glDeleteFramebuffers(1, &m_sceneFboId);					m_sceneFboId = INVALID_GL_ID;
	glDeleteTextures(1, &m_sceneTexId);						m_sceneTexId = INVALID_GL_ID;
	glDeleteRenderbuffers(1, &m_sceneDepthRenderbufferId);	m_sceneDepthRenderbufferId = INVALID_GL_ID;
}
