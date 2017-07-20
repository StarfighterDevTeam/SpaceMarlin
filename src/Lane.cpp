#include "Lane.h"
#include "Drawer.h"
#include "GPUProgramManager.h"
#include "Camera.h"
#include "glutil/glutil.h"

static const ivec2 gSideNbVtx(100,100);
//static const ivec2 gSideNbVtx(10,10);
//static const ivec2 gSideNbVtx(4,4);
static const float gGridSize = 10.f;

//#define _TEST_LANE_CPU_CYLINDER

#ifdef _LANE_USES_GPU
	#define _IF_LANE_USES_GPU(a, b) a
#else
	#define _IF_LANE_USES_GPU(a, b) b
#endif

Lane::Lane()
{
#ifdef _LANE_USES_GPU
	for(GLuint& texId : m_heightsTexId)
		texId = INVALID_GL_ID;
	for(GLuint& texId : m_waterFboId)
		texId = INVALID_GL_ID;
#endif
}

float Lane::getCylinderRadius() const
{
	return m_localToWorldMtx[0].x;
}

float Lane::interpolationMethod(float a, float b, float ratio) const
{
	return (1-ratio)*a + ratio*b;
}

vec3 Lane::getPosition() const
{
	return vec3(m_localToWorldMtx[3].x, m_localToWorldMtx[3].y, m_localToWorldMtx[3].z);
}

void Lane::init()
{
#ifdef _LANE_USES_GPU
	assert(m_heightsTexId[0] == INVALID_GL_ID);
#else
	assert(!m_vertices[0].size());
#endif

	m_curBufferIdx = 0;
	m_lastAnimationTimeSecs = -1.f;

	std::vector<VtxLane>* pInitVertices = NULL;
#ifdef _LANE_USES_GPU
	std::vector<VtxLane> vertices;
	pInitVertices = &vertices;

	// Create heights textures for water simulation
	for(int i=0 ; i < _countof(m_heightsTexId) ; i++)
	{
		glGenTextures(1, &m_heightsTexId[i]);
		glBindTexture(GL_TEXTURE_2D, m_heightsTexId[i]);

		// Set the filter
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Create the texture
		glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_R32F,
				gSideNbVtx.x, gSideNbVtx.y,
				0,
				GL_RED,
				GL_FLOAT,
				NULL);
	}
#else
	pInitVertices = &m_vertices[0];
#endif

	// Init vertices positions
	pInitVertices->resize(gSideNbVtx.x*gSideNbVtx.y);

#ifdef _TEST_LANE_CPU_CYLINDER
	const int nbVerticesPerRing = gSideNbVtx.x;
	const int nbRings = gSideNbVtx.y;

	assert(nbRings > 1);
	assert(nbVerticesPerRing > 1);
	const double distBetweenRings			= 1. / nbRings;
	const double angleBetweenRingVertices	= (2. * M_PI) / nbVerticesPerRing;

	const vec2 toUV = vec2(1.f/(nbRings-1), 1.f/(nbVerticesPerRing-1));
	for(int y=0 ; y < nbRings ; y++)
	{
		const float posZ = (float)(-y * distBetweenRings);
		for(int x=0 ; x < nbVerticesPerRing ; x++)
		{
			VtxLane& vtx = (*pInitVertices)[x + y*nbVerticesPerRing];
			const double angle = x * angleBetweenRingVertices - M_PI_2;
			vtx.pos.x = (float)cos(angle);
			vtx.pos.y = (float)sin(angle);
			vtx.pos.z = posZ;
			vtx.normal = vec3(vtx.pos.x, vtx.pos.y, 0);
			vtx.uv = vec2(x * toUV.x, y * toUV.y);
		}
	}
#else
	VtxLane vtx;
	vtx.pos.y = 0.f;
	vtx.normal = vec3(0,1,0);
	for(int z=0 ; z < gSideNbVtx.y ; z++)
	{
		vtx.pos.z = (z-gSideNbVtx.y/2) * gGridSize / gSideNbVtx.y;
		for(int x=0 ; x < gSideNbVtx.x ; x++)
		{
			vtx.pos.x = (x-gSideNbVtx.x/2) * gGridSize / gSideNbVtx.x;
			vtx.uv = vec2(x/((float)gSideNbVtx.x), z/((float)gSideNbVtx.y));
			(*pInitVertices)[x + z*gSideNbVtx.x] = vtx;
		}
	}
#endif

#if !defined(_LANE_USES_GPU)
	m_vertices[1] = m_vertices[2] = m_vertices[0];
#endif

#ifdef _TEST_LANE_CPU_CYLINDER
	for(int y = 0 ; y < nbRings-1 ; y++)
	{
		int x=0;
		for( ; x < nbVerticesPerRing-1 ; x++)
		{
			m_indices.push_back((x+0) + (y+0)*nbVerticesPerRing);
			m_indices.push_back((x+0) + (y+1)*nbVerticesPerRing);
			m_indices.push_back((x+1) + (y+1)*nbVerticesPerRing);

			m_indices.push_back((x+0) + (y+0)*nbVerticesPerRing);
			m_indices.push_back((x+1) + (y+1)*nbVerticesPerRing);
			m_indices.push_back((x+1) + (y+0)*nbVerticesPerRing);
		}

		int n = 1 - nbVerticesPerRing;
		m_indices.push_back((x+0) + (y+0)*nbVerticesPerRing);
		m_indices.push_back((x+0) + (y+1)*nbVerticesPerRing);
		m_indices.push_back((x+n) + (y+1)*nbVerticesPerRing);

		m_indices.push_back((x+0) + (y+0)*nbVerticesPerRing);
		m_indices.push_back((x+n) + (y+1)*nbVerticesPerRing);
		m_indices.push_back((x+n) + (y+0)*nbVerticesPerRing);
	}
#else
	for(int z = 0 ; z < gSideNbVtx.y-1 ; z++)
	{
		for(int x = 0 ; x < gSideNbVtx.x-1 ; x++)
		{
			m_indices.push_back((x+0) + (z+0)*gSideNbVtx.x);
			m_indices.push_back((x+0) + (z+1)*gSideNbVtx.x);
			m_indices.push_back((x+1) + (z+1)*gSideNbVtx.x);

			m_indices.push_back((x+0) + (z+0)*gSideNbVtx.x);
			m_indices.push_back((x+1) + (z+1)*gSideNbVtx.x);
			m_indices.push_back((x+1) + (z+0)*gSideNbVtx.x);
		}
	}
#endif
	
	// Send to GPU
	{
		glGenVertexArrays(1, &m_vertexArrayId);
		glBindVertexArray(m_vertexArrayId);

		const GLenum vbUsage = _IF_LANE_USES_GPU(GL_STATIC_DRAW, GL_STREAM_DRAW);
		
		// Load into the VBO
		glGenBuffers(1, &m_vertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, pInitVertices->size() * sizeof(VtxLane), &(*pInitVertices)[0], vbUsage);

		// Generate a buffer for the indices as well
		glGenBuffers(1, &m_indexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned short), &m_indices[0] , GL_STATIC_DRAW);

		// Setup vertex buffer layout
		SETUP_PROGRAM_VERTEX_ATTRIB(PROG_LANE)

		glBindVertexArray(0);
	}

#ifdef _LANE_USES_GPU
	// Init VBO/VAO
	{
		glGenVertexArrays(1, &m_waterVertexArrayId);
		glBindVertexArray(m_waterVertexArrayId);

		VtxWaterSimulation vertices[] = {
			{vec3(-1,-1,0)},	{vec3(+1,-1,0)},	{vec3(+1,+1,0)},
			{vec3(-1,-1,0)},	{vec3(+1,+1,0)},	{vec3(-1,+1,0)},
		};

		// Load into the VBO
		glGenBuffers(1, &m_waterVertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_waterVertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

		// Setup vertex buffer layout
		SETUP_PROGRAM_VERTEX_ATTRIB(PROG_WATER_SIMULATION)

		glBindVertexArray(0);
	}

	// Init framebuffer objects
	for(int curIdx=0 ; curIdx < _countof(m_waterFboId) ; curIdx++)
	{
		GLuint& fboId			= m_waterFboId[curIdx];
		GLuint heightsTexId		= m_heightsTexId[curIdx];

		glGenFramebuffers(1, &fboId);
		glutil::BindFramebuffer fboBinding(fboId);

		// - attach the textures:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, heightsTexId, 0);
		
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
#endif

	//Init lane transformations
	//Mtx 1
	static vec3 debugPos = vec3(0, 0, 0);
	static vec3 debugScale = vec3(2, 2, 1);
	m_localToWorldMtx = mat4(
		vec4(debugScale.x, 0, 0, 0),
		vec4(0, debugScale.y, 0, 0),
		vec4(0, 0, debugScale.z, 0),
		vec4(debugPos, 1));

	m_mtxVector.push_back(m_localToWorldMtx);

	//Mtx 2
	static vec3 debugPos2 = vec3(0, 0, 0);
	static vec3 debugScale2 = vec3(3, 3, 1);
	mat4 mtx2 = mat4(
		vec4(debugScale2.x, 0, 0, 0),
		vec4(0, debugScale2.y, 0, 0),
		vec4(0, 0, debugScale2.z, 0),
		vec4(debugPos2, 1));

	m_mtxVector.push_back(mtx2);

	//Mtx 3
	static vec3 debugPos3 = vec3(2, 0, 0);
	static vec3 debugScale3 = vec3(4, 4, 1);
	mat4 mtx3 = mat4(
		vec4(debugScale3.x, 0, 0, 0),
		vec4(0, debugScale3.y, 0, 0),
		vec4(0, 0, debugScale3.z, 0),
		vec4(debugPos3, 1));

	m_mtxVector.push_back(mtx3);
}

void Lane::shut()
{
	assert(m_indices.size() > 0);
	m_indices.clear();

#ifdef _LANE_USES_GPU
	assert(m_heightsTexId[0] != INVALID_GL_ID);

	for(GLuint& fboId : m_waterFboId)
	{
		glDeleteFramebuffers(1, &fboId);
		fboId = INVALID_GL_ID;
	}

	for(GLuint& texId : m_heightsTexId)
	{
		glDeleteBuffers(1, &texId);
		texId = INVALID_GL_ID;
	}
#else
	assert(m_vertices[0].size());
	for(int i=0 ; i < _countof(m_vertices) ; i++)
		m_vertices[i].clear();
#endif

	glDeleteBuffers(1, &m_vertexBufferId); m_vertexBufferId = INVALID_GL_ID;
	glDeleteBuffers(1, &m_indexBufferId); m_indexBufferId = INVALID_GL_ID;
	glDeleteVertexArrays(1, &m_vertexArrayId); m_vertexArrayId = INVALID_GL_ID;
}

void Lane::draw(const Camera& camera, GLuint texCubemapId, GLuint refractionTexId)
{
	//static float gfQuadSize = 1.f;
	//const vec3 quadPos[] = {
	//	vec3(-gfQuadSize,0,-gfQuadSize),
	//	vec3(+gfQuadSize,0,-gfQuadSize),
	//	vec3(+gfQuadSize,0,+gfQuadSize),
	//	vec3(-gfQuadSize,0,+gfQuadSize),
	//};
	//gData.drawer->drawLine(camera, quadPos[0], COLOR_BLUE, quadPos[1], COLOR_BLUE);
	//gData.drawer->drawLine(camera, quadPos[1], COLOR_BLUE, quadPos[2], COLOR_BLUE);
	//gData.drawer->drawLine(camera, quadPos[2], COLOR_BLUE, quadPos[3], COLOR_BLUE);
	//gData.drawer->drawLine(camera, quadPos[3], COLOR_BLUE, quadPos[0], COLOR_BLUE);

#if defined(_LANE_USES_GPU)
	updateWaterOnGPU();
#endif

	glutil::Enable<GL_BLEND> blendState;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const GPUProgram* laneProgram = gData.gpuProgramMgr->getProgram(PROG_LANE);
	laneProgram->use();
	gData.gpuProgramMgr->sendCommonUniforms(
		laneProgram,
		camera,
		m_localToWorldMtx);

	GLint textureSlot = 0;

	assert(texCubemapId != INVALID_GL_ID);
	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texCubemapId);
	laneProgram->sendUniform("texCubemap", textureSlot);
	textureSlot++;

	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glBindTexture(GL_TEXTURE_2D, refractionTexId);
	laneProgram->sendUniform("texRefraction", textureSlot);
	textureSlot++;

#ifdef _LANE_USES_GPU
	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glBindTexture(GL_TEXTURE_2D, m_heightsTexId[m_curBufferIdx]);
	laneProgram->sendUniform("texHeights", textureSlot);
	textureSlot++;

	laneProgram->sendUniform("gTexelSize", vec2(1.f/gSideNbVtx.x, 1.f/gSideNbVtx.y));
	laneProgram->sendUniform("gDistBetweenTexels", vec2(gGridSize / gSideNbVtx.x, gGridSize / gSideNbVtx.y));
#endif

	glBindVertexArray(m_vertexArrayId);

#if !defined(_LANE_USES_GPU)
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, m_vertices[m_curBufferIdx].size() * sizeof(VtxLane), &m_vertices[m_curBufferIdx][0], GL_STREAM_DRAW);
#endif

	{
		glutil::Disable<GL_CULL_FACE> cullFaceState;

		glDrawElements(
			GL_TRIANGLES,
			(GLsizei)m_indices.size(),
			GL_UNSIGNED_SHORT,
			(void*)0
		);
	}

	glBindVertexArray(0);

	// BEGIN TEST
#ifdef _LANE_USES_GPU
	for(int i=0 ; i < _countof(m_heightsTexId) ; i++)
		gData.drawer->draw2DTexturedQuad(m_heightsTexId[i], vec2(i*(10+gSideNbVtx.x),10), vec2(gSideNbVtx.x, gSideNbVtx.y));
#endif
	// END TEST

	// BEGIN debug normals
	static bool gbDebugDrawNormals = false;
	if(gbDebugDrawNormals)
	{
		mat3 localToWorldNormalMtx = glm::transpose(glm::inverse(mat3(m_localToWorldMtx)));
		//const int nbVerticesPerRing = gSideNbVtx.x;
		const int nbVerticesPerRing = 50;
		const double angleBetweenRingVertices	= (2. * M_PI) / nbVerticesPerRing;
		for(int x=0 ; x < nbVerticesPerRing ; x++)
		{
			vec3 localSpacePos;
			const double angle = x * angleBetweenRingVertices - M_PI_2;
			localSpacePos.x = (float)cos(angle);
			localSpacePos.y = (float)sin(angle);
			localSpacePos.z = 0;

			vec4 worldSpacePos = m_localToWorldMtx * vec4(localSpacePos,1);
			worldSpacePos.x /= worldSpacePos.w;
			worldSpacePos.y /= worldSpacePos.w;
			worldSpacePos.z /= worldSpacePos.w;
			worldSpacePos.w = 1;

			static float gfNormalSize = 1.f;
			vec3 localSpaceNormal = localSpacePos;
			vec3 worldSpaceNormal = glm::normalize(localToWorldNormalMtx * localSpaceNormal);
			vec3 worldSpacePosEnd = vec3(worldSpacePos) + gfNormalSize * worldSpaceNormal;
			gData.drawer->drawLine(camera, worldSpacePos, COLOR_RED, worldSpacePosEnd, COLOR_RED);
		}
	}
	// END debug normals
}

sf::Clock	laneSimulationStart;

void Lane::update()
{
	// BEGIN TEST
	static bool gbDebugTest = false;
	if(gbDebugTest)
	{
		static vec3 debugPos = vec3(0, 0, 0);
		static vec3 debugScale = vec3(4, 0.3, 1);
		static float gfDebugAngle = 0.f;
		static bool gbDebugUseTime = false;
		if(gbDebugUseTime)
		{
			static float gfSpeed = 0.001f;
			gfDebugAngle += gfSpeed * gData.dTime.asMilliseconds();
		}
		m_localToWorldMtx = mat4(
			vec4(debugScale.x, 0, 0, 0),
			vec4(0, debugScale.y, 0, 0),
			vec4(0, 0, debugScale.z, 0),
			vec4(debugPos, 1));

		static vec3 gvDebugRotAxis = vec3(0,0,1);
		mat4 laneRotMtx = glm::rotate(mat4(), gfDebugAngle, gvDebugRotAxis);
		m_localToWorldMtx = laneRotMtx * m_localToWorldMtx;
	}
	// END TEST

	//sf::sleep(sf::seconds(1.f));
	static float transformationTime = 3.f;
	if (laneSimulationStart.getElapsedTime().asSeconds() > 6.f)
	{
		if (m_mtxVector.size() > 1 && m_transformationClock.getElapsedTime().asSeconds() > transformationTime)
		{
			m_transformationClock.restart();
			m_mtxVector.erase(m_mtxVector.begin());
		}
			
		if (m_mtxVector.size() > 1)//checking again because we might just have erased a matrix
		{
			assert(m_transformationClock.getElapsedTime().asSeconds() < transformationTime);

			vec3 pos1 = vec3(m_mtxVector[0][3].x, m_mtxVector[0][3].y, m_mtxVector[0][3].z);
			vec3 scale1 = vec3(m_mtxVector[0][0].x, m_mtxVector[0][1].y, m_mtxVector[0][2].z);

			vec3 pos2 = vec3(m_mtxVector[1][3].x, m_mtxVector[1][3].y, m_mtxVector[1][3].z);
			vec3 scale2 = vec3(m_mtxVector[1][0].x, m_mtxVector[1][1].y, m_mtxVector[1][2].z);

			float t = m_transformationClock.getElapsedTime().asSeconds() / transformationTime;

			float a = interpolationMethod(pos1.x, pos2.x, t);
			float b = interpolationMethod(pos1.y, pos2.y, t);
			float c = interpolationMethod(pos1.z, pos2.z, t);

			float d = interpolationMethod(scale1.x, scale2.x, t);
			float e = interpolationMethod(scale1.y, scale2.y, t);
			float f = interpolationMethod(scale1.z, scale2.z, t);

			//if (laneSimulationStart.getElapsedTime().asSeconds() > 7)
			//{
			//	printf("\nsimulation clock: %f | transformation clock : %f\n", laneSimulationStart.getElapsedTime().asSeconds(), m_transformationClock.getElapsedTime().asSeconds());
			//	printf("stop");
			//}

			m_localToWorldMtx = mat4(
				vec4(d, 0, 0, 0),
				vec4(0, e, 0, 0),
				vec4(0, 0, f, 0),
				vec4(a, b, c, 1));

			//printf("a: %f, b:%f, c:%f, d:%f, e:%f, f:%f, t:%f\n\n", a, b, c, d, e, f, t);
		}
	}
	else
	{
		m_transformationClock.restart();
	}

	// BEGIN TEST
	//static vec3 debugPos = vec3(0, 0, 0);
	//static vec3 debugScale = vec3(4, 2, 2);
	//m_localToWorldMtx = mat4(
	//	vec4(debugScale.x, 0, 0, 0),
	//	vec4(0, debugScale.y, 0, 0),
	//	vec4(0, 0, debugScale.z, 0),
	//	vec4(debugPos, 1));

	// END TEST

	

#if !defined(_LANE_USES_GPU)
	// BEGIN TEST
	{
		static float gfLastTimeSecs = 0.f;
		if(gData.curFrameTime.asSeconds() - gfLastTimeSecs > 5.f)
		{
			gfLastTimeSecs = gData.curFrameTime.asSeconds();
			m_vertices[m_curBufferIdx][gSideNbVtx.x/2 + (gSideNbVtx.y/2)*gSideNbVtx.x].pos.y = 1.f;
		}
	}
	// END TEST

	// BEGIN TEST
	{
		//const float gGridSize = 10.f;
		//for(VtxLane& vtx : m_vertices[m_curBufferIdx])
		//{
		//	const float dx = vtx.pos.x / gGridSize;
		//	const float dy = vtx.pos.z / gGridSize;
		//	//const float dist = sqrtf(dx*dx + dy*dy);
		//	vtx.pos.y = sin(dx*M_PI) + sin(dy*M_PI);
		//}
		//computeNormals(&m_vertices[m_curBufferIdx][0], (int)m_vertices[m_curBufferIdx].size(), &m_indices[0], (int)m_indices.size());
		//return;
	}
	// END TEST

	if(m_lastAnimationTimeSecs < 0.f)
		m_lastAnimationTimeSecs = gData.curFrameTime.asSeconds();

	static float PARAM_C = 0.3f ; // ripple speed
	static float PARAM_D = 0.4f ; // distance
	static float PARAM_U = 0.05f ; // viscosity
	static float PARAM_T = 0.13f ; // time

	static float ANIMATIONS_PER_SECOND = 100.0f;

	// do rendering to get ANIMATIONS_PER_SECOND
	while(m_lastAnimationTimeSecs <= gData.curFrameTime.asSeconds())
	{
		// switch buffer numbers
		m_curBufferIdx = (m_curBufferIdx + 1) % 3 ;
		VtxLane* buf = &m_vertices[m_curBufferIdx][0];
		VtxLane* buf1 = &m_vertices[(m_curBufferIdx+2)%3][0];
		VtxLane* buf2 = &m_vertices[(m_curBufferIdx+1)%3][0];

		// we use an algorithm from
		// http://collective.valve-erc.com/index.php?go=water_simulation
		// The params could be dynamically changed every frame of course

		float C = PARAM_C; // ripple speed
		float D = PARAM_D; // distance
		float U = PARAM_U; // viscosity
		float T = PARAM_T; // time
		float TERM1 = ( 4.0f - 8.0f*C*C*T*T/(D*D) ) / (U*T+2) ;
		float TERM2 = ( U*T-2.0f ) / (U*T+2.0f) ;
		float TERM3 = ( 2.0f * C*C*T*T/(D*D) ) / (U*T+2) ;
		for(int y=1 ; y<gSideNbVtx.y-1 ; y++)	// don't do anything with border values
		{
			VtxLane *row = buf + y*(gSideNbVtx.x);
			VtxLane *row1 = buf1 + y*(gSideNbVtx.x);
			VtxLane *row1up = buf1 + (y-1)*(gSideNbVtx.x);
			VtxLane *row1down = buf1 + (y+1)*(gSideNbVtx.x);
			VtxLane *row2 = buf2 + y*(gSideNbVtx.x);
			for(int x=1;x<gSideNbVtx.x-1;x++)
			{
				row[x].pos.y = TERM1 * row1[x].pos.y
					+ TERM2 * row2[x].pos.y
					+ TERM3 * ( row1[x-1].pos.y + row1[x+1].pos.y + row1up[x].pos.y + row1down[x].pos.y );
			}
		}

		m_lastAnimationTimeSecs += (1.0f / ANIMATIONS_PER_SECOND);
	}
	computeNormals(&m_vertices[m_curBufferIdx][0], (int)m_vertices[m_curBufferIdx].size(), &m_indices[0], (int)m_indices.size());
#endif
}

void Lane::computeNormals(VtxLane* vertices, int nbVertices, const unsigned short* indices, int nbIndices)
{
	// Zero normals
	for(int i=0 ; i < nbVertices ; i++)
		vertices[i].normal = vec3(0,0,0);

	// Compute face normals, weighted by the face sizes (cross product) and add them to the vertex normals
	for(int i=0 ; i < nbIndices ; i+=3)
	{
		const unsigned short idx0 = indices[i+0];
		const unsigned short idx1 = indices[i+1];
		const unsigned short idx2 = indices[i+2];
		VtxLane& vtx0 = vertices[idx0];
		VtxLane& vtx1 = vertices[idx1];
		VtxLane& vtx2 = vertices[idx2];
		const vec3 diff1 = vtx1.pos - vtx0.pos;
		const vec3 diff2 = vtx2.pos - vtx0.pos;
		const vec3 faceNormal = glm::cross(diff1, diff2);
		vtx0.normal += faceNormal;
		vtx1.normal += faceNormal;
		vtx2.normal += faceNormal;
	}

	// Final normalization
	for(int i=0 ; i < nbVertices ; i++)
		vertices[i].normal = glm::normalize(vertices[i].normal);
}

void Lane::updateWaterOnGPU()
{
#ifdef _LANE_USES_GPU
	glutil::Disable<GL_CULL_FACE> cullFaceState;
	glutil::Disable<GL_DEPTH_TEST> depthTestState;

	GLint savedVp[4];
	glGetIntegerv(GL_VIEWPORT, savedVp);
	glViewport(0, 0, gSideNbVtx.x, gSideNbVtx.y);

	glBindVertexArray(m_waterVertexArrayId);

	const GPUProgram* waterSimulationProgram = gData.gpuProgramMgr->getProgram(PROG_WATER_SIMULATION);
	waterSimulationProgram->use();

	if(m_lastAnimationTimeSecs < 0.f)
		m_lastAnimationTimeSecs = gData.curFrameTime.asSeconds();

	static float PARAM_C = 0.8f ; // ripple speed
	static float PARAM_D = 0.4f ; // distance
	static float PARAM_U = 0.05f ; // viscosity
	static float PARAM_T = 0.13f ; // time

	static float ANIMATIONS_PER_SECOND = 100.0f;

	// do rendering to get ANIMATIONS_PER_SECOND
	while(m_lastAnimationTimeSecs <= gData.curFrameTime.asSeconds())
	{
		// switch buffer numbers
		m_curBufferIdx = (m_curBufferIdx + 1) % 3 ;

		glutil::BindFramebuffer fboBinding(m_waterFboId[m_curBufferIdx]);
		GLuint idTexHeights1 = m_heightsTexId[(m_curBufferIdx+2)%3];
		GLuint idTexHeights2 = m_heightsTexId[(m_curBufferIdx+1)%3];

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, idTexHeights1);
		waterSimulationProgram->sendUniform("texHeights1", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, idTexHeights2);
		waterSimulationProgram->sendUniform("texHeights2", 1);

		waterSimulationProgram->sendUniform("gTexelSize", vec2(1.f/gSideNbVtx.x, 1.f/gSideNbVtx.y));
		waterSimulationProgram->sendUniform("gTime", gData.curFrameTime.asSeconds());

		// we use an algorithm from
		// http://collective.valve-erc.com/index.php?go=water_simulation
		// The params could be dynamically changed every frame of course

		float C = PARAM_C; // ripple speed
		float D = PARAM_D; // distance
		float U = PARAM_U; // viscosity
		float T = PARAM_T; // time
		float TERM1 = ( 4.0f - 8.0f*C*C*T*T/(D*D) ) / (U*T+2) ;
		float TERM2 = ( U*T-2.0f ) / (U*T+2.0f) ;
		float TERM3 = ( 2.0f * C*C*T*T/(D*D) ) / (U*T+2) ;

		waterSimulationProgram->sendUniform("gTerm1", TERM1);
		waterSimulationProgram->sendUniform("gTerm2", TERM2);
		waterSimulationProgram->sendUniform("gTerm3", TERM3);

		glDrawArrays(GL_TRIANGLES, 0, 3*2);

		m_lastAnimationTimeSecs += (1.0f / ANIMATIONS_PER_SECOND);
	}

	// Restore viewport
	glViewport(savedVp[0], savedVp[1], savedVp[2], savedVp[3]);

	glBindVertexArray(0);
#endif
}
