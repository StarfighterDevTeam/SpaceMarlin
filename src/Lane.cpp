#include "Lane.h"
#include "Drawer.h"
#include "GPUProgramManager.h"
#include "Camera.h"
#include "glutil/glutil.h"
#include "Atom.h"
#include "Score.h"

static const ivec2 gSideNbVtx(100,100);
static bool gbDebugDrawNormals = false;
static bool gbDebugDrawCoordinateSystems = false;
static bool gbDebugDrawDistToSurface = false;
static bool gbDebugDrawWaterBuffers = false;
static bool gbDebugMoveLane = false;
#ifdef _USE_ANTTWEAKBAR
static TwBar*	gDebugBar = NULL;
static int		gDebugNbLaneInstances = 0;
LaneKeyframe	gDebugKeyframe;
static const char* getLaneNameForDebugBar(int laneId)
{
	static char str[256];
	sprintf_s(str, "Lane #%d", laneId);
	return str;
}
#endif

void LaneKeyframe::PrecomputedData::update(float& dist, float& r0, float& r1, float& yaw, float& pitch, float& roll, vec3& pos)
{
	// Avoid stupid distances between C0 and C1
	const float minDist			= std::max(abs(r0-r1), 0.0001f);
	if(dist < minDist)
		dist = minDist;

	constexpr float _2pi		= 2*(float)M_PI;
	halfDist					= dist*0.5f;
	theta						= acos( (r0 - r1) / dist );
	tanTheta					= tan(theta);
	const float sinTheta		= sin(theta);

	const float lengthOnC0		= ((float)M_PI - theta) * r0;
	const float lengthOnC1		= theta * r1;
	const float lengthTangent	= tanTheta * (r0 - r1);
	capsulePerimeter			= 2 * (lengthOnC1 + lengthTangent + lengthOnC0);

	threshold0		= lengthOnC1;
	threshold1		= threshold0 + lengthTangent;
	threshold2		= threshold1 + 2*lengthOnC0;
	threshold3		= threshold2 + lengthTangent;
	threshold0to1	= threshold1 - threshold0;
	threshold2to3	= threshold3 - threshold2;

	xOffsetOnC0 = (r0 - r1) * r0 / dist;
	xOffsetOnC1 = (r0 - r1) * r1 / dist;

	yOffsetOnC0 = sinTheta * r0;
	yOffsetOnC1 = sinTheta * r1;

	topLeftPos			= vec2(-halfDist + xOffsetOnC0, yOffsetOnC0);
	topRightPos			= vec2(halfDist + xOffsetOnC1, yOffsetOnC1);
	topTangentVector	= glm::normalize(topLeftPos - topRightPos);

	bottomLeftPos		= vec2(-halfDist + xOffsetOnC0, -yOffsetOnC0);
	bottomRightPos		= vec2(halfDist + xOffsetOnC1, -yOffsetOnC1);
	bottomTangentVector	= glm::normalize(bottomRightPos - bottomLeftPos);

	localToWorldMtx = glm::yawPitchRoll(yaw, pitch, roll);
	localToWorldMtx[3].x = pos.x;
	localToWorldMtx[3].y = pos.y;
	localToWorldMtx[3].z = pos.z;
	worldToLocalMtx = glm::inverse(localToWorldMtx);	// TODO: overkill...
}

struct GPULaneKeyframe
{
#define HANDLE_UNIFORM_DECLARE_LANE_KEYFRAME_MEMBER(type, varName)	\
		type varName;

	FOREACH_LANE_KEYFRAME_MEMBER(HANDLE_UNIFORM_DECLARE_LANE_KEYFRAME_MEMBER)

	void sendUniforms(const GPUProgram* program)
	{
	#define HANDLE_UNIFORM_SEND_UNIFORM(type, varName)	\
		program->sendUniform(#varName, varName, Hash::AT_RUNTIME);

	FOREACH_LANE_KEYFRAME_MEMBER(HANDLE_UNIFORM_SEND_UNIFORM)
	}
};

void LaneKeyframe::toGPULaneKeyframe(GPULaneKeyframe& gpuKeyframe) const
{
	gpuKeyframe.gKeyframeR0						= r0;
	gpuKeyframe.gKeyframeR1						= r1;
	gpuKeyframe.gKeyframeHalfDist				= precomp.halfDist;
	gpuKeyframe.gKeyframeTheta					= precomp.theta;
	gpuKeyframe.gKeyframeCapsulePerimeter		= precomp.capsulePerimeter;
	gpuKeyframe.gKeyframeThreshold0				= precomp.threshold0;
	gpuKeyframe.gKeyframeThreshold1				= precomp.threshold1;
	gpuKeyframe.gKeyframeThreshold2				= precomp.threshold2;
	gpuKeyframe.gKeyframeThreshold3				= precomp.threshold3;
	gpuKeyframe.gKeyframeThreshold0to1			= precomp.threshold0to1;
	gpuKeyframe.gKeyframeThreshold2to3			= precomp.threshold2to3;

	gpuKeyframe.gKeyframeTopRightPos			= precomp.topRightPos;
	gpuKeyframe.gKeyframeTopLeftPos				= precomp.topLeftPos;
	gpuKeyframe.gKeyframeBottomLeftPos			= precomp.bottomLeftPos;
	gpuKeyframe.gKeyframeBottomRightPos			= precomp.bottomRightPos;
	gpuKeyframe.gKeyframeTopTangentVector		= precomp.topTangentVector;
	gpuKeyframe.gKeyframeBottomTangentVector	= precomp.bottomTangentVector;

	quat qRot(precomp.localToWorldMtx);
	gpuKeyframe.gKeyframeRot.x					= qRot.x;
	gpuKeyframe.gKeyframeRot.y					= qRot.y;
	gpuKeyframe.gKeyframeRot.z					= qRot.z;
	gpuKeyframe.gKeyframeRot.w					= qRot.w;
	gpuKeyframe.gKeyframeTrans					= vec3(precomp.localToWorldMtx[3]);
}

Lane::Lane()
{
	for(GLuint& texId : m_heightsTexId)
		texId = INVALID_GL_ID;
	for(GLuint& fboId : m_waterFboId)
		fboId = INVALID_GL_ID;
	m_waterNormalsTexId = INVALID_GL_ID;
	m_waterNormalsFboId = INVALID_GL_ID;
	m_keyframesBufferId = INVALID_GL_ID;

	m_atomBlueprint = NULL;
	m_id = -1;
}

void Lane::init(const LaneTrack* track, int id, ModelResource* atomBlueprint)
{
	assert(m_heightsTexId[0] == INVALID_GL_ID);

	m_track = track;
	m_id = id;
	m_atomBlueprint = atomBlueprint;

	m_curBufferIdx = 0;
	m_lastAnimationTimeSecs = -1.f;

	std::vector<VtxLane> vertices;

	// Create heights textures for water simulation
	for(int i=0 ; i < _countof(m_heightsTexId) ; i++)
	{
		glGenTextures(1, &m_heightsTexId[i]);
		glBindTexture(GL_TEXTURE_2D, m_heightsTexId[i]);

		// Set the filter
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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

	// Create water normals texture
	{
		glGenTextures(1, &m_waterNormalsTexId);
		glBindTexture(GL_TEXTURE_2D, m_waterNormalsTexId);

		// Set the filter
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Create the texture
		glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA32F,
				gSideNbVtx.x, gSideNbVtx.y,
				0,
				GL_RGBA,
				GL_FLOAT,
				NULL);
	}

	// Init vertices positions
	vertices.resize(gSideNbVtx.x*gSideNbVtx.y);

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
			VtxLane& vtx = vertices[x + y*nbVerticesPerRing];
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
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VtxLane), &vertices[0], vbUsage);

		// Generate a buffer for the indices as well
		glGenBuffers(1, &m_indexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned short), &m_indices[0] , GL_STATIC_DRAW);

		// Setup vertex buffer layout
		SETUP_PROGRAM_VERTEX_ATTRIB(PROG_LANE)

		glBindVertexArray(0);
	}

	// Init VBO/VAO for water simulation (height computation)
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

	// Init VBO/VAO for water normals computation
	{
		glGenVertexArrays(1, &m_waterNormalsVertexArrayId);
		glBindVertexArray(m_waterNormalsVertexArrayId);

		VtxWaterNormals vertices[] = {
			{vec3(-1,-1,0)},	{vec3(+1,-1,0)},	{vec3(+1,+1,0)},
			{vec3(-1,-1,0)},	{vec3(+1,+1,0)},	{vec3(-1,+1,0)},
		};

		// Load into the VBO
		glGenBuffers(1, &m_waterNormalsVertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_waterNormalsVertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

		// Setup vertex buffer layout
		SETUP_PROGRAM_VERTEX_ATTRIB(PROG_WATER_NORMALS)

		glBindVertexArray(0);
	}

	// Init framebuffer objects for water simulation (height computation)
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

	// Init framebuffer object for water normals computation
	{
		glGenFramebuffers(1, &m_waterNormalsFboId);
		glutil::BindFramebuffer fboBinding(m_waterNormalsFboId);

		// - attach the textures:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_waterNormalsTexId, 0);
		
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

	// Init GPU resources for keyframing
	{
		// Create the buffer
		glGenBuffers(1, &m_keyframesBufferId);
		glBindBuffer(GL_TEXTURE_BUFFER, m_keyframesBufferId);
		
		float tboData[] = {
			1,0,0,
			0,1,0,
			0,0,1
		};
		glBufferData(GL_TEXTURE_BUFFER, sizeof(tboData), tboData, GL_STATIC_DRAW);

		// Create the associated texture and link to the buffer
		glGenTextures(1, &m_keyframesTexId);
		glBindTexture(GL_TEXTURE_BUFFER, m_keyframesTexId);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_keyframesBufferId);

		//glBindBuffer(GL_TEXTURE_BUFFER, 0);	// unbind the buffer
	}

	m_curKeyframe = m_track->keyframes[0];
	
#ifdef _USE_ANTTWEAKBAR
	gDebugNbLaneInstances++;
	if(!gDebugBar)
	{
		gDebugBar = TwNewBar("Lane");
		TwAddVarRW(gDebugBar, "Dist",			TW_TYPE_FLOAT,		&gDebugKeyframe.dist,	"min=0.1 max=20 step=0.05 group=Keyframe");
		TwAddVarRW(gDebugBar, "R0",				TW_TYPE_FLOAT,		&gDebugKeyframe.r0,		"min=0.1 max=20 step=0.05 group=Keyframe");
		TwAddVarRW(gDebugBar, "R1",				TW_TYPE_FLOAT,		&gDebugKeyframe.r1,		"min=0.1 max=20 step=0.05 group=Keyframe");
		TwAddVarRW(gDebugBar, "Yaw",			TW_TYPE_FLOAT,		&gDebugKeyframe.yaw,	"min=-10 max=10 step=0.01 group=Keyframe");
		TwAddVarRW(gDebugBar, "Pitch",			TW_TYPE_FLOAT,		&gDebugKeyframe.pitch,	"min=-10 max=10 step=0.01 group=Keyframe");
		TwAddVarRW(gDebugBar, "Roll",			TW_TYPE_FLOAT,		&gDebugKeyframe.roll,	"min=-10 max=10 step=0.01 group=Keyframe");
		TwAddVarRW(gDebugBar, "PosX",			TW_TYPE_FLOAT,		&gDebugKeyframe.pos.x,	"min=-20 max=20 step=0.05 group=Keyframe");
		TwAddVarRW(gDebugBar, "PosY",			TW_TYPE_FLOAT,		&gDebugKeyframe.pos.y,	"min=-20 max=20 step=0.05 group=Keyframe");
		TwAddVarRW(gDebugBar, "PosZ",			TW_TYPE_FLOAT,		&gDebugKeyframe.pos.z,	"min=-20 max=20 step=0.05 group=Keyframe");
		TwAddVarRW(gDebugBar, "Draw SDF",		TW_TYPE_BOOLCPP,	&gbDebugDrawDistToSurface,		"group=Debug");
		TwAddVarRW(gDebugBar, "Draw normals",	TW_TYPE_BOOLCPP,	&gbDebugDrawNormals,			"group=Debug");
		TwAddVarRW(gDebugBar, "Draw coordsys",	TW_TYPE_BOOLCPP,	&gbDebugDrawCoordinateSystems,	"group=Debug");
		TwAddVarRW(gDebugBar, "Draw water",		TW_TYPE_BOOLCPP,	&gbDebugDrawWaterBuffers,		"group=Debug");
		TwAddVarRW(gDebugBar, "Move lane",		TW_TYPE_BOOLCPP,	&gbDebugMoveLane,				"group=Debug");
	}
	m_debugTweaking = false;
	TwAddVarRW(gDebugBar, getLaneNameForDebugBar(m_id), TW_TYPE_BOOLCPP, &m_debugTweaking, "group=Tweaking");
#endif
}

void Lane::shut()
{
	assert(m_indices.size() > 0);
	m_indices.clear();

	// GPU resources for height computation
	assert(m_heightsTexId[0] != INVALID_GL_ID);

	for(GLuint& fboId : m_waterFboId)
	{
		glDeleteFramebuffers(1, &fboId);
		fboId = INVALID_GL_ID;
	}

	for(GLuint& texId : m_heightsTexId)
	{
		glDeleteTextures(1, &texId);
		texId = INVALID_GL_ID;
	}

	glDeleteVertexArrays(1, &m_waterVertexArrayId);	m_waterVertexArrayId = INVALID_GL_ID;
	glDeleteBuffers(1, &m_waterVertexBufferId);		m_waterVertexBufferId = INVALID_GL_ID;

	// GPU resources for normals computation
	glDeleteFramebuffers(1, &m_waterNormalsFboId);			m_waterNormalsFboId = INVALID_GL_ID;
	glDeleteTextures(1, &m_waterNormalsTexId);				m_waterNormalsTexId = INVALID_GL_ID;
	glDeleteVertexArrays(1, &m_waterNormalsVertexArrayId);	m_waterNormalsVertexArrayId = INVALID_GL_ID;
	glDeleteBuffers(1, &m_waterNormalsVertexBufferId);

	// GPU resources for the lane itself
	glDeleteVertexArrays(1, &m_vertexArrayId); m_vertexArrayId = INVALID_GL_ID;
	glDeleteBuffers(1, &m_vertexBufferId); m_vertexBufferId = INVALID_GL_ID;
	glDeleteBuffers(1, &m_indexBufferId); m_indexBufferId = INVALID_GL_ID;

	// GPU resources for keyframing
	glDeleteBuffers(1, &m_keyframesBufferId);
	glDeleteTextures(1, &m_keyframesTexId);
	
#ifdef _USE_ANTTWEAKBAR
	assert(gDebugBar);
	TwRemoveVar(gDebugBar, getLaneNameForDebugBar(m_id));
	gDebugNbLaneInstances--;
	if(gDebugNbLaneInstances == 0)
	{
		TwDeleteBar(gDebugBar);
		gDebugBar = NULL;
	}
#endif

	m_id = -1;
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
		m_curKeyframe.precomp.localToWorldMtx);

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

	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glBindTexture(GL_TEXTURE_2D, m_waterNormalsTexId);
	laneProgram->sendUniform("texNormals", textureSlot);
	textureSlot++;

	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glBindTexture(GL_TEXTURE_BUFFER, m_keyframesTexId);
	laneProgram->sendUniform("texKeyframes", textureSlot);
	textureSlot++;

	const LaneKeyframe& kf = m_curKeyframe;

	// Send keyframe information
	GPULaneKeyframe gpuKf;
	kf.toGPULaneKeyframe(gpuKf);
	gpuKf.sendUniforms(laneProgram);

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
		const GLuint texIds[_countof(m_heightsTexId)+1] = {
			m_heightsTexId[0],
			m_heightsTexId[1],
			m_heightsTexId[2],
			m_waterNormalsTexId,
		};
		for(int i=0 ; i < _countof(texIds) ; i++)
			gData.drawer->draw2DTexturedQuad(texIds[i], vec2(i*(10+gSideNbVtx.x),10), vec2(gSideNbVtx.x, gSideNbVtx.y));
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

	if(gbDebugDrawCoordinateSystems)
	{
		for(float y=-5.f ; y <= 5.f; y += 0.25f)
		{
			for(float x=-5.f ; x <= 5.f; x += 0.25f)
			{
				const vec3 worldSpacePos = vec3(x,y,0);
				vec3 worldSpaceRight, worldSpaceNormal, worldSpaceBack;
				getCoordinateSystem(worldSpacePos, worldSpaceRight, worldSpaceNormal, worldSpaceBack);
				static float gfDebugVectorSize = 0.1f;
				gData.drawer->drawLine(camera, worldSpacePos, COLOR_RED,	worldSpacePos + gfDebugVectorSize * worldSpaceRight,	COLOR_RED);
				gData.drawer->drawLine(camera, worldSpacePos, COLOR_GREEN,	worldSpacePos + gfDebugVectorSize * worldSpaceNormal,	COLOR_GREEN);
				gData.drawer->drawLine(camera, worldSpacePos, COLOR_BLUE,	worldSpacePos + gfDebugVectorSize * worldSpaceBack,		COLOR_BLUE);
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
#ifdef _USE_ANTTWEAKBAR
	if(m_debugTweaking)
		m_curKeyframe = gDebugKeyframe;
	else
#endif
	{
		// TODO: interpolation using m_curKeyframe.t
		// Find previous and next keyframes
		int idxFirst = 0;
		int idxSecond = 0;
		while(m_track->keyframes[idxSecond].t < m_curKeyframe.t && idxSecond < (int)m_track->keyframes.size()-1)
			idxSecond++;
		idxFirst = std::max(0, idxSecond-1);

		if(idxFirst == idxSecond)
			m_curKeyframe = m_track->keyframes[idxFirst];
		else
		{
			const LaneKeyframe& kf0 = m_track->keyframes[idxFirst];
			const LaneKeyframe& kf1 = m_track->keyframes[idxSecond];
			const float timeDelta = kf1.t.asSeconds() - kf0.t.asSeconds();
			assert(timeDelta > 0.0001f);
			const float u = (m_curKeyframe.t.asSeconds() - kf0.t.asSeconds()) / timeDelta;

			m_curKeyframe.dist	= glm::lerp(	kf0.dist,	kf1.dist,	u);
			m_curKeyframe.r0	= glm::lerp(	kf0.r0,		kf1.r0,		u);
			m_curKeyframe.r1	= glm::lerp(	kf0.r1,		kf1.r1,		u);
			const mat4 rotMtx0 = glm::yawPitchRoll(kf0.yaw, kf0.pitch, kf0.roll);	// yaw=Y, pitch=X, roll=Z
			const mat4 rotMtx1 = glm::yawPitchRoll(kf1.yaw, kf1.pitch, kf1.roll);
			const quat qRot0(rotMtx0);
			const quat qRot1(rotMtx1);
			const quat qRot = glm::slerp(qRot0, qRot1, u);
			const mat4 rotMtx = glm::mat4_cast(qRot);
			glm::extractEulerAngleXYZ(rotMtx, m_curKeyframe.pitch, m_curKeyframe.yaw, m_curKeyframe.roll);
			m_curKeyframe.yaw	= glm::lerp(	kf0.yaw,	kf1.yaw,	u);
			m_curKeyframe.pitch	= glm::lerp(	kf0.pitch,	kf1.pitch,	u);
			m_curKeyframe.roll	= glm::lerp(	kf0.roll,	kf1.roll,	u);
			m_curKeyframe.pos	= glm::lerp(	kf0.pos,	kf1.pos,	u);
		}
	}

	if(gbDebugMoveLane)
	{
		static vec3 debugPos = vec3(0, 0, 0);
		static float gfDebugAngle = 0.f;
		static bool gbDebugUseTime = true;
		if(gbDebugUseTime)
		{
			static float gfSpeed = 0.001f;
			gfDebugAngle += gfSpeed * gData.dTime.asMilliseconds();
		}
		
		m_curKeyframe.roll = gfDebugAngle;
		m_curKeyframe.pos = debugPos;
	}

	m_curKeyframe.updatePrecomputedData();
}

void Lane::updateWaterOnGPU()
{
	glutil::Disable<GL_CULL_FACE> cullFaceState;
	glutil::Disable<GL_DEPTH_TEST> depthTestState;

	GLint savedVp[4];
	glGetIntegerv(GL_VIEWPORT, savedVp);
	glViewport(0, 0, gSideNbVtx.x, gSideNbVtx.y);

	// Compute heights (water simulation)
	{
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
	}

	// Compute water normals
	{
		const LaneKeyframe& kf = m_curKeyframe;

		glBindVertexArray(m_waterNormalsVertexArrayId);

		const GPUProgram* waterNormalsProgram = gData.gpuProgramMgr->getProgram(PROG_WATER_NORMALS);
		waterNormalsProgram->use();

		glutil::BindFramebuffer fboBinding(m_waterNormalsFboId);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_heightsTexId[m_curBufferIdx]);
		waterNormalsProgram->sendUniform("texHeights", 0);

		waterNormalsProgram->sendUniform("gTexelSize", vec2(1.f/gSideNbVtx.x, 1.f/gSideNbVtx.y));
		waterNormalsProgram->sendUniform("gDistBetweenTexels", vec2(kf.precomp.capsulePerimeter / gSideNbVtx.x, kf.precomp.capsulePerimeter / gSideNbVtx.y));

		glDrawArrays(GL_TRIANGLES, 0, 3*2);
	}

	// Restore viewport
	glViewport(savedVp[0], savedVp[1], savedVp[2], savedVp[3]);

	glBindVertexArray(0);
}

vec3 Lane::getPosition() const
{
	const mat4 localToWorldMtx = m_curKeyframe.precomp.localToWorldMtx;
	return vec3( localToWorldMtx[3].x, localToWorldMtx[3].y, localToWorldMtx[3].z);
}

vec3 Lane::getNormalToSurface(const vec3& worldSpacePos) const
{
	vec3 worldSpaceRight;
	vec3 worldSpaceNormal;
	vec3 worldSpaceBack;
	getCoordinateSystem(worldSpacePos, worldSpaceRight, worldSpaceNormal, worldSpaceBack);
	return worldSpaceNormal;
}

void Lane::getCoordinateSystem(const vec3& worldSpacePos, vec3& worldSpaceRight, vec3& worldSpaceNormal, vec3& worldSpaceBack) const
{
	// TODO: handle Z

	vec4 localSpacePos = m_curKeyframe.precomp.worldToLocalMtx * vec4(worldSpacePos,1);
	localSpacePos.x /= localSpacePos.w;
	localSpacePos.y /= localSpacePos.w;
	localSpacePos.z /= localSpacePos.w;
	//localSpacePos.w = 1;
	
	vec3 localSpaceRight, localSpaceNormal, localSpaceBack;
	getLocalSpaceCoordinateSystem(vec3(localSpacePos), localSpaceRight, localSpaceNormal, localSpaceBack);

	const mat3 localToWorldRotMtx = mat3(m_curKeyframe.precomp.localToWorldMtx);
	worldSpaceRight		= localToWorldRotMtx * localSpaceRight;
	worldSpaceNormal	= localToWorldRotMtx * localSpaceNormal;
	worldSpaceBack		= localToWorldRotMtx * localSpaceBack;
}

void Lane::getLocalSpaceCoordinateSystem(const vec3& localSpacePos, vec3& localSpaceRight, vec3& localSpaceNormal, vec3& localSpaceBack) const
{
	// TODO: handle Z

	vec2 p = vec2(localSpacePos);

	const LaneKeyframe& kf = m_curKeyframe;
	
	vec2 backupNormal = vec2(0,1);
	vec2 backupTangent = vec2(-1,0);

	if(p.x > kf.precomp.halfDist + kf.precomp.xOffsetOnC1)
	{
		// Compute normal relative to C1
		localSpaceNormal = vec3(
			safeNormalize(p - vec2(kf.precomp.halfDist, 0.f), backupNormal),
			0);
	}
	else if(p.x < -kf.precomp.halfDist + kf.precomp.xOffsetOnC0)
	{
		// Compute normal relative to C2
		localSpaceNormal = vec3(
			safeNormalize(p - vec2(-kf.precomp.halfDist, 0.f), backupNormal),
			0);
	}
	else if(p.y > 0.f)
	{
		localSpaceNormal = vec3(kf.precomp.topTangentVector.y, -kf.precomp.topTangentVector.x, 0);
	}
	else //if(p.y <= 0.f)
	{
		localSpaceNormal = vec3(kf.precomp.bottomTangentVector.y, -kf.precomp.bottomTangentVector.x, 0);
	}

	localSpaceRight = vec3(localSpaceNormal.y, -localSpaceNormal.x, 0);
	localSpaceBack = vec3(0,0,1);	// could be something else later (affected by waves...)
}

float Lane::getDistToSurface(const vec3& worldSpacePos) const
{
	// TODO: handle Z

	vec4 localSpacePos = m_curKeyframe.precomp.worldToLocalMtx * vec4(worldSpacePos,1);
	//localSpacePos.x /= localSpacePos.w;
	//localSpacePos.y /= localSpacePos.w;
	//localSpacePos.z /= localSpacePos.w;
	//localSpacePos.w = 1;
	vec2 p = vec2(localSpacePos.x / localSpacePos.w, localSpacePos.y / localSpacePos.w);

	const LaneKeyframe& kf = m_curKeyframe;
	
	const vec2& segStart	= p.y >= 0 ? kf.precomp.topLeftPos : kf.precomp.bottomLeftPos;
	const vec2& segEnd		= p.y >= 0 ? kf.precomp.topRightPos : kf.precomp.bottomRightPos;
	const vec2 segment		= segEnd - segStart;
	const float u			= glm::dot(p - segStart, segment) / glm::dot(segment, segment);
	if(u < 0)
	{
		const vec2 c0Center = vec2(-kf.precomp.halfDist, 0);
		return glm::length(p - c0Center) - kf.r0;	// signed distance to C0
	}
	else if(u > 1)
	{
		const vec2 c1Center = vec2(kf.precomp.halfDist, 0);
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

void Lane::setupAtom(Atom* pAtom)
{
	pAtom->setModelResourcePtr(m_atomBlueprint);
	
	vec3 position = vec3(getPosition().x, getPosition().y, 2.f);
	pAtom->setPosition(position);
}
