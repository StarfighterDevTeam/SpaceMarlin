#include "MainScene.h"
#include "glutil/glutil.h"
#include "GPUProgramManager.h"
#include "Drawer.h"
#include "InputManager.h"
#include "FollowCamera.h"
#include "SoundManager.h"

#define _USE_HALF_FLOAT
#ifdef _USE_HALF_FLOAT
	#define _IF_USE_HALF_FLOAT(x,y) x
#else
	#define _IF_USE_HALF_FLOAT(x,y) y
#endif
#define RT_FORMAT _IF_USE_HALF_FLOAT(GL_RGBA16F,GL_RGBA32F)

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
	if(!m_bob.init())
		return false;

	// Atom resource
	if (!m_atomBlueprint.loadFromFile((gData.assetsPath + "/models/atom/atom.fbx").c_str()))
		return false;

	// Load score
	// TODO: manually created test score for now
	if(!m_score.load((gData.assetsPath + "/scores/score01.txt").c_str()))
		return false;
	
	// Lanes
	m_lanes.resize(m_score.getLaneTrackCount());
	for (int i = 0; i < (int)m_score.getLaneTrackCount() ; i++)
	{
		m_lanes[i].init(&m_score.getLaneTrack(i), i, &m_atomBlueprint);
		m_bob.addLane(&m_lanes[i]);
	}
	
	m_camera = (FollowCamera*)getCamera();
	assert(m_camera);

	m_camera->setPosition(vec3(-2, 3, 8));
	m_camera->setFront(normalize(-m_camera->getPosition()));
	m_camera->setUp(vec3(0,1,0));
	m_camera->setFollowedObject(&m_bob);
	//m_camera->setFlyOver(true);

	initSceneFBO();

	m_postProcessTriangle.init();

	//Music
	m_curScoreTime = sf::seconds(0.f);
	m_beatCount = 0;
	m_measureCount = 0;

	return true;
}

void MainScene::shut()
{
	Scene::shut();

	m_skybox.unload();
	m_bob.shut();
	for(Lane& lane : m_lanes)
		lane.shut();

	m_atoms.clear();
	m_atomBlueprint.unload();

	m_score.unload();
	
	shutSceneFBO();
	m_postProcessTriangle.shut();
}

void MainScene::update()
{
	Scene::update();

	// For now we keep repeating the score
	m_curScoreTime += gData.dTime;
	if(m_curScoreTime > m_score.getDuration())
		m_curScoreTime -= m_score.getDuration();

	size_t lanesVectorSize = m_lanes.size();
	for (size_t i = 0; i < lanesVectorSize; i++)
	{
		m_lanes[i].setCurTime(m_curScoreTime);
		m_lanes[i].update();
	}

	//static float gfCurAngle = 0.f;
	//static float gfSpeed = 0.001f;
	//gfCurAngle += gfSpeed * gData.dTime.asMilliseconds();
	//m_bob.setLocalToWorldMtx(glm::rotate(glm::mat4(), gfCurAngle, glm::vec3(0.f, 1.f, 0.f)));

	for(Atom& atom : m_atoms)
		atom.update();

	m_bob.update();

	//Music
	m_beatCount = (int)(gData.soundMgr->getCurMusic().getPlayingOffset().asSeconds() * gData.soundMgr->getBPM() / 60.f) - (m_measureCount * 4);
	while (m_beatCount > 4)
	{
		m_beatCount -= 4;
		m_measureCount++;

		//test
		//if (m_measureCount == 6)//could be every beat
		//{
			//printf("CREATION ATOM\n");
			m_atoms.push_back(Atom());
			m_lanes[0].setupAtom(&m_atoms.back());
			m_atoms.back().m_speed = 5.f;
			m_atoms.back().move(vec3(0.f, 8.f, -20.f));
		//}
	}
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
		
		m_skybox.draw(*m_camera);
	
		m_bob.draw(*m_camera);

		// Debug model gizmo
		gData.drawer->drawLine(*m_camera, vec3(0,0,0), COLOR_RED,	vec3(3,0,0), COLOR_RED		);
		gData.drawer->drawLine(*m_camera, vec3(0,0,0), COLOR_GREEN,	vec3(0,3,0), COLOR_GREEN	);
		gData.drawer->drawLine(*m_camera, vec3(0,0,0), COLOR_BLUE,	vec3(0,0,3), COLOR_BLUE		);

		// Copy scene image for refraction effect
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindTexture(GL_TEXTURE_2D, m_sceneRefractionTexId);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, RT_FORMAT, 0, 0, gData.winSizeX, gData.winSizeY, 0);

		// Draw lane
		size_t lanesVectorSize = m_lanes.size();
		for (size_t i = 0; i < lanesVectorSize; i++)
		{
			m_lanes[i].draw(*m_camera, m_skybox.getSkyTexId(), m_sceneRefractionTexId);
		}

		// Draw atoms
		for(Atom& atom : m_atoms)
			atom.draw(*m_camera);
	}
}

void MainScene::drawAfter()
{
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

	// BEGIN TEST
	vec2 debugSize = vec2(gData.winSizeX/4.f, gData.winSizeY/4.f);
	gData.drawer->draw2DTexturedQuad(m_sceneRefractionTexId, vec2(gData.winSizeX - debugSize.x - 10, 10), debugSize);
	// END TEST
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

Camera*	MainScene::createCamera() const
{
	return new FollowCamera();
}

// Setup scene FBO & refraction texture
void MainScene::initSceneFBO()
{
	m_sceneTexId = glutil::createTextureRGBAF(gData.winSizeX, gData.winSizeY, _IF_USE_HALF_FLOAT(true, false));
	m_sceneRefractionTexId = glutil::createTextureRGBAF(gData.winSizeX, gData.winSizeY, _IF_USE_HALF_FLOAT(true, false));
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
	glDeleteTextures(1, &m_sceneRefractionTexId);			m_sceneRefractionTexId = INVALID_GL_ID;
	glDeleteRenderbuffers(1, &m_sceneDepthRenderbufferId);	m_sceneDepthRenderbufferId = INVALID_GL_ID;
}
