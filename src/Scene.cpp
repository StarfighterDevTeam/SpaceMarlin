#include "Scene.h"
#include "InputManager.h"

bool Scene::init()
{
	m_camera = createCamera();
	m_camera->init();
	return true;
}

void Scene::shut()
{
	m_camera->shut();
	delete m_camera;
	m_camera = NULL;
}

void Scene::update()
{
	m_camera->update();
}

void Scene::draw()
{
	m_camera->draw();
}

void Scene::onEvent(const sf::Event& event)
{
	if(gData.inputMgr->eventIsDebugCamReleased(event))
	{
		m_camera->setFlyOver(!m_camera->isFlyOver());
		logInfo("Toggling debug camera to: ", m_camera->isFlyOver() ? "on" : "off");
	}
	m_camera->onEvent(event);
}
