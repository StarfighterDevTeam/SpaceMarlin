#include "MainScene.h"

#include "glutil/glutil.h"
#include <glm/gtc/matrix_transform.hpp>

#include "SharedDefines.h"

bool MainScene::init()
{
	// ------ Model program -----
	m_modelProgram = new glutil::GPUProgram("media/shaders/model.vert", "media/shaders/model.frag");
	if(!m_modelProgram->compileAndAttach())
		return false;
	m_modelProgram->bindAttribLocation(MODEL_ATTRIB_POSITIONS,	"pos");
	m_modelProgram->bindAttribLocation(MODEL_ATTRIB_UVS,		"uv");
	m_modelProgram->bindAttribLocation(MODEL_ATTRIB_NORMALS,	"normal");

	if(!m_modelProgram->link())
		return false;

	std::list<std::string> uniformNames;
	uniformNames.push_back("gModelViewProjMtx");
	uniformNames.push_back("texAlbedo");
	m_modelProgram->setUniformNames(uniformNames);

	// ------ Model ------
	//if(!m_model.loadFromFile("media/models/suzanne_gltuto/suzanne.obj"))
	if(!m_model.loadFromFile("media/models/marlin/marlin.fbx"))
	//if(!m_model.loadFromFile("media/models/marlin/marlin.obj"))
		return false;

	return true;
}

void MainScene::shut()
{
	SAFE_DELETE(m_modelProgram);
	m_model.unload();
}

void MainScene::update()
{
}

void MainScene::draw()
{
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	m_modelProgram->use();

	const float fNear = 0.1f;
	const float fFar = 100.f;
	glm::mat4 gModelViewProjMtx;
	static float gfCurAngle = 0.f;
	static float gfSpeed = 0.001f;
	gfCurAngle += gfSpeed * gData.dTime.asMilliseconds();
	glm::mat4 modelMtx = glm::rotate(glm::mat4(), gfCurAngle, glm::vec3(0.f, 1.f, 0.f));

	glm::mat4 projMtx = glm::perspective(glm::radians(45.0f), ((float)gData.winSizeX) / ((float)gData.winSizeY), fNear, fFar); 
	
	glm::mat4 viewMtx = glm::lookAt(glm::vec3(0.f, 3.f, -6.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	gModelViewProjMtx = projMtx * viewMtx * modelMtx;
	m_modelProgram->sendUniform("gModelViewProjMtx", gModelViewProjMtx);

	m_modelProgram->sendUniform("texAlbedo", 0);

	// Texture:
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_RECTANGLE, id_texture);
//
//	if(!gpu_transfert_done)
//	{
//		glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
//
//		GL_CHECK();
//		gpu_transfert_done = true;
//	}

	// Setup shader and its uniforms:
//	glUseProgram(id_program);
//	glUniform1i(uniform_texunit, 0);

	m_model.draw();
}
