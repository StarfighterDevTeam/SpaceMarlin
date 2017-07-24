#include "Lane.h"
#include "Drawer.h"
#include "GPUProgramManager.h"
#include "Camera.h"
#include "glutil/glutil.h"

static const ivec2 gSideNbVtx(100,100);
static const float gGridSize = 10.f;
static bool gbDebugDrawNormals = false;
static bool gbDebugDrawDistToSurface = false;
static bool gbDebugDrawWaterBuffers = false;
static bool gbDebugMoveLane = false;

Lane::Lane()
{
	for(GLuint& texId : m_heightsTexId)
		texId = INVALID_GL_ID;
	for(GLuint& texId : m_waterFboId)
		texId = INVALID_GL_ID;
}

vec3 Lane::getPosition() const
{
	return vec3(m_localToWorldMtx[3].x, m_localToWorldMtx[3].y, m_localToWorldMtx[3].z);
}

vec3 Lane::getNormalToSurface(const vec3& worldSpacePos) const
{
	// TODO: handle Z

	vec4 localSpacePos = m_worldToLocalMtx * vec4(worldSpacePos,1);
	//localSpacePos.x /= localSpacePos.w;
	//localSpacePos.y /= localSpacePos.w;
	//localSpacePos.z /= localSpacePos.w;
	//localSpacePos.w = 1;
	vec2 p = vec2(localSpacePos.x / localSpacePos.w, localSpacePos.y / localSpacePos.w);

	const Keyframe& kf = m_curKeyframe;
	const float xOffsetOnC0 = (kf.r0 - kf.r1) * kf.r0 / kf.dist;
	const float xOffsetOnC1 = (kf.r0 - kf.r1) * kf.r1 / kf.dist;

	vec2 backupNormal = vec2(0,1);
	vec2 backupTangent = vec2(-1,0);

	vec3 localSpaceNormal;
	if(p.x > 0.5f*kf.dist + xOffsetOnC1)
	{
		// Compute normal relative to C1
		localSpaceNormal = vec3(
			safeNormalize(p - vec2(0.5f*kf.dist, 0.f), backupNormal),
			0);
	}
	else if(p.x < -0.5f*kf.dist + xOffsetOnC0)
	{
		// Compute normal relative to C2
		localSpaceNormal = vec3(
			safeNormalize(p - vec2(-0.5f*kf.dist, 0.f), backupNormal),
			0);
	}
	else if(p.y > 0.f)
	{
		// Compute normal relative to top tangent segment of capsule
		const float theta = acos( (kf.r0 - kf.r1) / kf.dist );
		const vec2 startPos = vec2(0.5*kf.dist + xOffsetOnC1, xOffsetOnC1 * tan(theta));
		const vec2 endPos = vec2(-0.5*kf.dist + xOffsetOnC0, xOffsetOnC0 * tan(theta));
		
		const vec2 tangentVector = safeNormalize(endPos - startPos, backupTangent);
		localSpaceNormal = vec3(tangentVector.y, -tangentVector.x, 0);
	}
	else //if(p.y <= 0.f)
	{
		// Compute normal relative to bottom tangent segment of capsule
		const float theta = acos( (kf.r0 - kf.r1) / kf.dist );
		const vec2 startPos = vec2(-0.5*kf.dist + xOffsetOnC0, -xOffsetOnC0 * tan(theta));
		const vec2 endPos = vec2(0.5*kf.dist + xOffsetOnC1, -xOffsetOnC1 * tan(theta));
		
		const vec2 tangentVector = safeNormalize(endPos - startPos, backupTangent);
		localSpaceNormal = vec3(tangentVector.y, -tangentVector.x, 0);
	}

	vec3 worldSpaceNormal = mat3(m_localToWorldMtx) * localSpaceNormal;
	
	return worldSpaceNormal;
}

float Lane::getDistToSurface(const vec3& worldSpacePos) const
{
	// TODO: handle Z

	vec4 localSpacePos = m_worldToLocalMtx * vec4(worldSpacePos,1);
	//localSpacePos.x /= localSpacePos.w;
	//localSpacePos.y /= localSpacePos.w;
	//localSpacePos.z /= localSpacePos.w;
	//localSpacePos.w = 1;
	vec2 p = vec2(localSpacePos.x / localSpacePos.w, localSpacePos.y / localSpacePos.w);

	const Keyframe& kf = m_curKeyframe;
	const float xOffsetOnC0 = (kf.r0 - kf.r1) * kf.r0 / kf.dist;
	const float xOffsetOnC1 = (kf.r0 - kf.r1) * kf.r1 / kf.dist;
	const float theta = acos( (kf.r0 - kf.r1) / kf.dist );
	const float yOffsetOnC0 = xOffsetOnC0 * tan(theta);
	const float yOffsetOnC1 = xOffsetOnC1 * tan(theta);
	
	const vec2 segments[4] = {
		// top segment
		vec2(-0.5*kf.dist + xOffsetOnC0, yOffsetOnC0),	// left (C0)
		vec2(0.5*kf.dist + xOffsetOnC1, yOffsetOnC1),	// right (C1)

		// bottom segment
		vec2(-0.5*kf.dist + xOffsetOnC0, -yOffsetOnC0),	// left (C0)
		vec2(0.5*kf.dist + xOffsetOnC1, -yOffsetOnC1),	// right (C1)
	};

	const vec2& segStart	= p.y >= 0 ? segments[0] : segments[2];
	const vec2& segEnd		= p.y >= 0 ? segments[1] : segments[3];
	const vec2 segment		= segEnd - segStart;
	const float u			= glm::dot(p - segStart, segment) / glm::dot(segment, segment);
	if(u < 0)
	{
		const vec2 c0Center = vec2(-0.5f*kf.dist, 0);
		return glm::length(p - c0Center) - kf.r0;	// signed distance to C0
	}
	else if(u > 1)
	{
		const vec2 c1Center = vec2(0.5f*kf.dist, 0);
		return glm::length(p - c1Center) - kf.r1;	// signed distance to C1
	}
	else
	{
		// signed distance to the segment
		const vec2	h		= segStart + u * segment;
		const float s		=	-safeSign(p.y) *
								safeSign(glm::determinant(mat2(p - segStart, segment)));
		return s * glm::length(p - h);
	}
}

vec3 Lane::getGravityVector(const vec3& worldSpacePos) const
{
	// TODO: have a different gravity vector when the distance between the cylinders is > threshold
	return getNormalToSurface(worldSpacePos);
}

void Lane::init(mat4 initialMtx)
{
	assert(m_heightsTexId[0] == INVALID_GL_ID);

	m_curBufferIdx = 0;
	m_lastAnimationTimeSecs = -1.f;

	std::vector<VtxLane>* pInitVertices = NULL;

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

	// Init vertices positions
	pInitVertices->resize(gSideNbVtx.x*gSideNbVtx.y);

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
			const double angle = x * angleBetweenRingVertices;
			vtx.pos.x = (float)cos(angle);
			vtx.pos.y = (float)sin(angle);
			vtx.pos.z = posZ;
			vtx.normal = vec3(vtx.pos.x, vtx.pos.y, 0);
			vtx.uv = vec2(x * toUV.x, y * toUV.y);
		}
	}

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
	
	// Send to GPU
	{
		glGenVertexArrays(1, &m_vertexArrayId);
		glBindVertexArray(m_vertexArrayId);

		const GLenum vbUsage = GL_STATIC_DRAW;
		
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

	m_localToWorldMtx = initialMtx;
	m_worldToLocalMtx = glm::inverse(m_localToWorldMtx);

#ifdef _USE_ANTTWEAKBAR
	m_debugBar = TwNewBar("Lane");
	TwAddVarRW(m_debugBar, "PosX", TW_TYPE_FLOAT, &m_localToWorldMtx[3].x,"");
	TwAddVarRW(m_debugBar, "PosY", TW_TYPE_FLOAT, &m_localToWorldMtx[3].y,"");
	TwAddVarRW(m_debugBar, "PosZ", TW_TYPE_FLOAT, &m_localToWorldMtx[3].z,"");
	TwAddVarRW(m_debugBar, "Draw SDF", TW_TYPE_BOOLCPP, &gbDebugDrawDistToSurface,"");
	TwAddVarRW(m_debugBar, "Draw normals", TW_TYPE_BOOLCPP, &gbDebugDrawNormals,"");
	TwAddVarRW(m_debugBar, "Draw water buffers", TW_TYPE_BOOLCPP, &gbDebugDrawWaterBuffers,"");
	TwAddVarRW(m_debugBar, "Move lane", TW_TYPE_BOOLCPP, &gbDebugMoveLane,"");
#endif
}

void Lane::shut()
{
	assert(m_indices.size() > 0);
	m_indices.clear();

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

	glDeleteBuffers(1, &m_vertexBufferId); m_vertexBufferId = INVALID_GL_ID;
	glDeleteBuffers(1, &m_indexBufferId); m_indexBufferId = INVALID_GL_ID;
	glDeleteVertexArrays(1, &m_vertexArrayId); m_vertexArrayId = INVALID_GL_ID;

#ifdef _USE_ANTTWEAKBAR
	TwDeleteBar(m_debugBar);
	m_debugBar = NULL;
#endif
}

void Lane::draw(const Camera& camera, GLuint texCubemapId, GLuint refractionTexId)
{
	updateWaterOnGPU();

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

	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glBindTexture(GL_TEXTURE_2D, m_heightsTexId[m_curBufferIdx]);
	laneProgram->sendUniform("texHeights", textureSlot);
	textureSlot++;

	laneProgram->sendUniform("gTexelSize", vec2(1.f/gSideNbVtx.x, 1.f/gSideNbVtx.y));
	laneProgram->sendUniform("gDistBetweenTexels", vec2(gGridSize / gSideNbVtx.x, gGridSize / gSideNbVtx.y));

	// Send test keyframe to shader
	m_curKeyframe.dist = 4.f;
	m_curKeyframe.r0 = 2.f;
	m_curKeyframe.r1 = 0.8f;
	
	m_localToWorldMtx = glm::translate(glm::yawPitchRoll(m_curKeyframe.yaw, m_curKeyframe.pitch, m_curKeyframe.roll), m_curKeyframe.pos);
	m_worldToLocalMtx = glm::inverse(m_localToWorldMtx);

	laneProgram->sendUniform("gKeyframeDist", m_curKeyframe.dist);
	laneProgram->sendUniform("gKeyframeR0", m_curKeyframe.r0);
	laneProgram->sendUniform("gKeyframeR1", m_curKeyframe.r1);
	laneProgram->sendUniform("gKeyframeLocalToWorldMtx", m_localToWorldMtx, false, Hash::AT_RUNTIME);

	glBindVertexArray(m_vertexArrayId);

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

	debugDraw(camera);
}

void Lane::debugDraw(const Camera& camera)
{
	if(gbDebugDrawWaterBuffers)
	{
		for(int i=0 ; i < _countof(m_heightsTexId) ; i++)
			gData.drawer->draw2DTexturedQuad(m_heightsTexId[i], vec2(i*(10+gSideNbVtx.x),10), vec2(gSideNbVtx.x, gSideNbVtx.y));
	}

	if(gbDebugDrawNormals)
	{
		for(float y=-5.f ; y <= 5.f; y += 0.25f)
		{
			for(float x=-5.f ; x <= 5.f; x += 0.25f)
			{
				const vec3 worldSpacePos = vec3(x,y,0);
				const vec3 worldSpaceNormal = getNormalToSurface(worldSpacePos);
				static float gfDebugNormalSize = 0.1f;
				gData.drawer->drawLine(camera, worldSpacePos, COLOR_RED, worldSpacePos + gfDebugNormalSize * worldSpaceNormal, COLOR_WHITE);
			}
		}
	}

	if(gbDebugDrawDistToSurface)
	{
		for(float y=-5.f ; y <= 5.f; y += 0.25f)
		{
			for(float x=-5.f ; x <= 5.f; x += 0.25f)
			{
				const vec3 worldSpacePos = vec3(x,y,0);
				const float distToSurface = getDistToSurface(worldSpacePos);
				static float gfCrossSize = 0.1f;
				static float gfColorScale = 1.f;
				const vec4 color = distToSurface > 0.f ? vec4(0, distToSurface*gfColorScale, 0, 1) : vec4(-distToSurface*gfColorScale, 0, 0, 1);
				gData.drawer->drawCross(camera, worldSpacePos, color, gfCrossSize);
			}
		}
	}
}

void Lane::update()
{
	if(gbDebugMoveLane)
	{
		static vec3 debugPos = vec3(0, 0, 0);
		static vec3 debugScale = vec3(4, 0.3, 1);
		static float gfDebugAngle = 0.f;
		static bool gbDebugUseTime = true;
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
		m_worldToLocalMtx = glm::inverse(m_localToWorldMtx);
	}
}

void Lane::updateWaterOnGPU()
{
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
}
