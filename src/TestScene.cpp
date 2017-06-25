#include "TestScene.h"

#include "glutil/glutil.h"
#include <glm/gtc/matrix_transform.hpp>

#include "SharedDefines.h"

bool TestScene::init()
{
	// ------- Quad ---------
	const int sx = 80;
	const int sy = 60;
	m_quad = new glutil::Quad(sx, sy);
	glutil::Quad::Pixel* pixels = new glutil::Quad::Pixel[sx*sy];
	for(int y=0 ; y < sy ; y++)
	{
		for(int x=0 ; x < sx ; x++)
		{
			uint8_t r,g,b,a;
			r = x * 255 / sx;
			g = 0;
			b = 0;
			a = 0xff;
			pixels[x + y*sx] = (r << 24) | (r << 16) | (r << 8) | (a << 0);
		}
	}
	m_quad->setPixels(pixels);
	delete [] pixels;

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
	m_modelProgram->setUniformNames(uniformNames);

	// ------ Model ------
	if(!m_model.loadFromFile("media/models/suzanne_gltuto/suzanne.obj"))
		return false;

	// ------ VBO/VAO --------
	createVBOAndVAO();

	return true;
}

void TestScene::shut()
{
	SAFE_DELETE(m_quad);
	SAFE_DELETE(m_modelProgram);
}

void TestScene::update()
{
}

void TestScene::draw()
{
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	//m_quad->display();

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

	// Draw the VAO:
	glBindVertexArray(m_idVAO);
	glDrawArrays(GL_TRIANGLES, 0, m_nbVertices);
}

void TestScene::createVBOAndVAO()
{
	// Create the VAO:
	glGenVertexArrays(1, &m_idVAO);

	// Bind the VAO:
	glBindVertexArray(m_idVAO);

	GLsizeiptr bufferSize = 0;
	const GLvoid* bufferData = NULL;
	const GLvoid* indicesBufferData = NULL;

	m_nbVertices = 6;
	GLfloat quadBufferData[] = {	// Vertex coordinates (x, y, z)
								-0.5f, -0.5f, 0.f,	// triangle 1
								+0.5f, -0.5f, 0.f,
								+0.5f, +0.5f, 0.f,

								-0.5f, -0.5f, 0.f,	// triangle 2
								+0.5f, +0.5f, 0.f,
								-0.5f, +0.5f, 0.f,

								// Texture coordinates (u, v)
								0.f, 0.f,	// triangle 1
								1.f, 0.f,
								1.f, 1.f,
								
								0.f, 0.f,	// triangle 2
								1.f, 1.f,
								0.f, 1.f,

								// Normals (x,y,z)
								1.f, 0.f, 0.f,	// triangle 1
								0.f, 1.f, 0.f,
								0.f, 0.f, 1.f,

								1.f, 1.f, 0.f,	// triangle 2
								0.f, 1.f, 1.f,
								1.f, 0.f, 1.f,
							};
	bufferData = quadBufferData;
	bufferSize = sizeof(quadBufferData);

	// Create the buffer and put the data into it:
	glGenBuffers(1, &m_idVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_idVertexBuffer);

	glBufferData(GL_ARRAY_BUFFER, bufferSize, bufferData, GL_STATIC_DRAW);

	// - enable attributes:
	glEnableVertexAttribArray(MODEL_ATTRIB_POSITIONS);
	glEnableVertexAttribArray(MODEL_ATTRIB_UVS);
	glEnableVertexAttribArray(MODEL_ATTRIB_NORMALS);

	// - positions pointer:
	ptrdiff_t offset = NULL;
	glVertexAttribPointer(MODEL_ATTRIB_POSITIONS,  3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*m_nbVertices*3;

	// - uvs pointer:
	glVertexAttribPointer(MODEL_ATTRIB_UVS,  2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*m_nbVertices*2;

	// - normals pointer:
	glVertexAttribPointer(MODEL_ATTRIB_NORMALS,  3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*m_nbVertices*3;

	GL_CHECK();
}
