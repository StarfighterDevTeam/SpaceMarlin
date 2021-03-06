#include "Lane.h"
#include "Drawer.h"
#include "GPUProgramManager.h"
#include "Camera.h"
#include "glutil/glutil.h"
#include "Atom.h"
#include "Score.h"

static const int	gNbVerticesPerRing = 100;
static const int	gNbRings = 100;
static const float	gLaneLength = 100.f;	// in meters

void LaneKeyframe::PrecomputedData::update(float& dist, float& r0, float& r1, float& yaw, float& pitch, float& roll, vec2& offset)
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

	// TODO
	localToWorldMtx = glm::yawPitchRoll(yaw, pitch, roll);
	localToWorldMtx[3].x = offset.x;
	localToWorldMtx[3].y = offset.y;
	localToWorldMtx[3].z = 0.f;
	worldToLocalMtx = glm::inverse(localToWorldMtx);	// TODO: overkill...
}

void LaneKeyframe::setFromKeyframes(const LaneKeyframe& kf0, const LaneKeyframe& kf1, float newBeat)
{
	assert(kf0.beat <= newBeat);
	assert(kf1.beat >= newBeat);

	if(kf0.beat == newBeat || kf0.beat == kf1.beat)
	{
		*this = kf0;
		return;
	}

	if(kf1.beat == newBeat)
	{
		*this = kf1;
		return;
	}

	const float beatDelta = (float)(kf1.beat - kf0.beat);
	assert(beatDelta > 0.0001f);
	const float u = (newBeat - (float)kf0.beat) / beatDelta;

	beat	= newBeat;
	dist	= glm::lerp(	kf0.dist,	kf1.dist,	u);
	r0		= glm::lerp(	kf0.r0,		kf1.r0,		u);
	r1		= glm::lerp(	kf0.r1,		kf1.r1,		u);
	const mat4 rotMtx0 = glm::yawPitchRoll(kf0.yaw, kf0.pitch, kf0.roll);	// yaw=Y, pitch=X, roll=Z
	const mat4 rotMtx1 = glm::yawPitchRoll(kf1.yaw, kf1.pitch, kf1.roll);
	const quat qRot0(rotMtx0);
	const quat qRot1(rotMtx1);
	const quat qRot = glm::slerp(qRot0, qRot1, u);
	const mat4 rotMtx = glm::mat4_cast(qRot);
	glm::extractEulerAngleXYZ(rotMtx, pitch, yaw, roll);
	yaw		= glm::lerp(	kf0.yaw,	kf1.yaw,	u);
	pitch	= glm::lerp(	kf0.pitch,	kf1.pitch,	u);
	roll	= glm::lerp(	kf0.roll,	kf1.roll,	u);
	offset	= glm::lerp(	kf0.offset,	kf1.offset,	u);

	updatePrecomputedData();
}

void LaneKeyframe::toGPULaneKeyframe(GPULaneKeyframe& gpuKeyframe) const
{
	gpuKeyframe.r0							= r0;
	gpuKeyframe.r1							= r1;
	gpuKeyframe.halfDist					= precomp.halfDist;
	gpuKeyframe.theta						= precomp.theta;
	gpuKeyframe.capsulePerimeter			= precomp.capsulePerimeter;
	gpuKeyframe.threshold0					= precomp.threshold0;
	gpuKeyframe.threshold1					= precomp.threshold1;
	gpuKeyframe.threshold2					= precomp.threshold2;
	gpuKeyframe.threshold3					= precomp.threshold3;
	gpuKeyframe.threshold0to1				= precomp.threshold0to1;
	gpuKeyframe.threshold2to3				= precomp.threshold2to3;

	gpuKeyframe.topRightPos					= precomp.topRightPos;
	gpuKeyframe.topLeftPos					= precomp.topLeftPos;
	gpuKeyframe.bottomLeftPos				= precomp.bottomLeftPos;
	gpuKeyframe.bottomRightPos				= precomp.bottomRightPos;
	gpuKeyframe.topTangentVector			= precomp.topTangentVector;
	gpuKeyframe.keyframeBottomTangentVector	= precomp.bottomTangentVector;

	quat localToWorldRot(precomp.localToWorldMtx);
	gpuKeyframe.localToWorldRot.x			= localToWorldRot.x;
	gpuKeyframe.localToWorldRot.y			= localToWorldRot.y;
	gpuKeyframe.localToWorldRot.z			= localToWorldRot.z;
	gpuKeyframe.localToWorldRot.w			= localToWorldRot.w;
	gpuKeyframe.localToWorldTrans			= vec3(precomp.localToWorldMtx[3]);
}

void Lane::init(LaneTrack* track, int id, ModelResource* atomBlueprint, TwBar* editionBar, TwBar* debugBar)
{
	assert(m_id == -1);
	m_track = track;
	m_id = id;

	char str[256];
	sprintf_s(str, "Lane#%02d", id);
	m_name = str;

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
				gNbVerticesPerRing, gNbRings,
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
				gNbVerticesPerRing, gNbRings,
				0,
				GL_RGBA,
				GL_FLOAT,
				NULL);
	}

	// Init vertices positions
	vertices.resize(gNbVerticesPerRing * gNbRings);

	assert(gNbRings > 1);
	assert(gNbVerticesPerRing > 1);
	const double distBetweenRings			= 1. / gNbRings;
	const double angleBetweenRingVertices	= (2. * M_PI) / gNbVerticesPerRing;

	const vec2 toUV = vec2(1.f/(gNbRings-1), 1.f/(gNbVerticesPerRing-1));
	for(int y=0 ; y < gNbRings ; y++)
	{
		const float posZ = (float)(-y * distBetweenRings);
		for(int x=0 ; x < gNbVerticesPerRing ; x++)
		{
			VtxLane& vtx = vertices[x + y*gNbVerticesPerRing];
			const double angle = x * angleBetweenRingVertices;
			vtx.pos.x = (float)cos(angle);
			vtx.pos.y = (float)sin(angle);
			vtx.pos.z = posZ;
			vtx.normal = vec3(vtx.pos.x, vtx.pos.y, 0);
			vtx.uv = vec2(x * toUV.x, y * toUV.y);
		}
	}

	for(int y = 0 ; y < gNbRings-1 ; y++)
	{
		int x=0;
		for( ; x < gNbVerticesPerRing-1 ; x++)
		{
			m_indices.push_back((x+0) + (y+0)*gNbVerticesPerRing);
			m_indices.push_back((x+0) + (y+1)*gNbVerticesPerRing);
			m_indices.push_back((x+1) + (y+1)*gNbVerticesPerRing);

			m_indices.push_back((x+0) + (y+0)*gNbVerticesPerRing);
			m_indices.push_back((x+1) + (y+1)*gNbVerticesPerRing);
			m_indices.push_back((x+1) + (y+0)*gNbVerticesPerRing);
		}

		int n = 1 - gNbVerticesPerRing;
		m_indices.push_back((x+0) + (y+0)*gNbVerticesPerRing);
		m_indices.push_back((x+0) + (y+1)*gNbVerticesPerRing);
		m_indices.push_back((x+n) + (y+1)*gNbVerticesPerRing);

		m_indices.push_back((x+0) + (y+0)*gNbVerticesPerRing);
		m_indices.push_back((x+n) + (y+1)*gNbVerticesPerRing);
		m_indices.push_back((x+n) + (y+0)*gNbVerticesPerRing);
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
		
		std::vector<GPULaneKeyframe>	gpuKeyframes;
		gpuKeyframes.resize(track->normalizedKeyframes.size());
		for(int i=0 ; i < (int)gpuKeyframes.size() ; i++)
			track->normalizedKeyframes[i].toGPULaneKeyframe(gpuKeyframes[i]);

		glBufferData(GL_TEXTURE_BUFFER, gpuKeyframes.size() * sizeof(gpuKeyframes[0]), (float*)(&gpuKeyframes[0]), GL_STREAM_DRAW);

		// Create the associated texture and link to the buffer
		glGenTextures(1, &m_keyframesTexId);
		glBindTexture(GL_TEXTURE_BUFFER, m_keyframesTexId);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_keyframesBufferId);

		glBindBuffer(GL_TEXTURE_BUFFER, 0);	// unbind the buffer
	}

	m_curKeyframe = m_track->keyframes[0];
	
	// Edition
	m_prevEditionMode = false;
	m_editionMode = false;
	m_prevEditedBeat = 0;
	m_editedBeat = 0;
	auto addEditionVar = [&](const char* text, TwType type, void* ptr, const char* def)
	{
	#ifdef _USE_ANTTWEAKBAR
		char finalDef[1024];
		sprintf_s(finalDef, "%s group='%s'", def, getName());

		char finalText[256];
		sprintf_s(finalText, "[%02d] %s", m_id, text);

		TwAddVarRW(editionBar, finalText, type, ptr, finalDef);
	#endif
	};
	addEditionVar("Beat",	 TW_TYPE_INT32, &m_editedBeat,				"");
	addEditionVar("Dist",	 TW_TYPE_FLOAT, &m_editionLaneKeyframe.dist,		"min=0.1 max=20 step=0.05");
	addEditionVar("R0",		 TW_TYPE_FLOAT, &m_editionLaneKeyframe.r0,			"min=0.1 max=20 step=0.05");
	addEditionVar("R1",		 TW_TYPE_FLOAT, &m_editionLaneKeyframe.r1,			"min=0.1 max=20 step=0.05");
	addEditionVar("Yaw",	 TW_TYPE_FLOAT, &m_editionLaneKeyframe.yaw,			"min=-10 max=10 step=0.01");
	addEditionVar("Pitch",	 TW_TYPE_FLOAT, &m_editionLaneKeyframe.pitch,		"min=-10 max=10 step=0.01");
	addEditionVar("Roll",	 TW_TYPE_FLOAT, &m_editionLaneKeyframe.roll,		"min=-10 max=10 step=0.01");
	addEditionVar("OffsetX", TW_TYPE_FLOAT, &m_editionLaneKeyframe.offset.x,	"min=-20 max=20 step=0.05");
	addEditionVar("OffsetY", TW_TYPE_FLOAT, &m_editionLaneKeyframe.offset.y,	"min=-20 max=20 step=0.05");
	
	// Debug
	auto addDebugVar = [&](const char* text, bool* ptr)
	{
		*ptr = false;
	#ifdef _USE_ANTTWEAKBAR
		char finalDef[1024];
		sprintf_s(finalDef, "group='Debug draw %s'", getName());

		char finalText[256];
		sprintf_s(finalText, "[%02d] %s", m_id, text);

		TwAddVarRW(debugBar, finalText, TW_TYPE_BOOLCPP, ptr, finalDef);
	#endif
	};
	addDebugVar("Draw normals",		&m_bDebugDrawNormals);
	addDebugVar("Draw coordsys",	&m_bDebugDrawCoordinateSystems);
	addDebugVar("Draw SDF",			&m_bDebugDrawDistToSurface);
	addDebugVar("Draw water",		&m_bDebugDrawWaterBuffers);
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

	laneProgram->sendUniform("gLaneLength", gLaneLength);
	laneProgram->sendUniform("gLaneLengthInKeyframes", (int)m_track->normalizedKeyframes.size(), Hash::AT_RUNTIME);
	laneProgram->sendUniform("gSizeOfKeyframeInFloats", (GLint)(sizeof(GPULaneKeyframe)/sizeof(float)), Hash::AT_RUNTIME);

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
	if(m_bDebugDrawWaterBuffers)
	{
		const GLuint texIds[_countof(m_heightsTexId)+1] = {
			m_heightsTexId[0],
			m_heightsTexId[1],
			m_heightsTexId[2],
			m_waterNormalsTexId,
		};
		for(int i=0 ; i < _countof(texIds) ; i++)
			gData.drawer->draw2DTexturedQuad(texIds[i], vec2(i*(10+gNbVerticesPerRing),10), vec2(gNbVerticesPerRing, gNbRings));
	}

	if(m_bDebugDrawNormals)
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

	if(m_bDebugDrawCoordinateSystems)
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

	if(m_bDebugDrawDistToSurface)
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
	if(m_editionMode)
	{
		// Clamp edited beat
		m_editedBeat = clamp(m_editedBeat, 0, (int)m_track->keyframes.size()-1);

		// Reinitializing edition keyframe values
		if(m_prevEditionMode != m_editionMode || m_prevEditedBeat != m_editedBeat)
			m_editionLaneKeyframe = m_track->keyframes[m_editedBeat];
	}

	{
		// Find previous and next keyframes
		int idxPrev = 0;
		int idxNext = 0;
		while(m_track->keyframes[idxNext].beat < m_curKeyframe.beat && idxNext < (int)m_track->keyframes.size()-1)
			idxNext++;
		idxPrev = std::max(0, idxNext-1);
	
		m_curKeyframe.setFromKeyframes(m_track->keyframes[idxPrev], m_track->keyframes[idxNext], m_curKeyframe.beat);
	}

	if(m_editionMode)
	{
		m_curKeyframe = m_editionLaneKeyframe;
		m_editionLaneKeyframe.updatePrecomputedData();
		m_track->keyframes[m_editedBeat] = m_editionLaneKeyframe;
		m_track->computeNormalizedKeyframes();
		uploadTrackKeyframesToGPU();
	}

	m_curKeyframe.updatePrecomputedData();
	m_prevEditionMode = m_editionMode;
	m_prevEditedBeat = m_editedBeat;
}

void Lane::uploadTrackKeyframesToGPU()
{
	glBindBuffer(GL_TEXTURE_BUFFER, m_keyframesBufferId);
		
	std::vector<GPULaneKeyframe>	gpuKeyframes;
	gpuKeyframes.resize(m_track->normalizedKeyframes.size());
	for(int i=0 ; i < (int)gpuKeyframes.size() ; i++)
		m_track->normalizedKeyframes[i].toGPULaneKeyframe(gpuKeyframes[i]);

	glBufferData(GL_TEXTURE_BUFFER, gpuKeyframes.size() * sizeof(gpuKeyframes[0]), (float*)(&gpuKeyframes[0]), GL_STREAM_DRAW);

	glBindBuffer(GL_TEXTURE_BUFFER, 0);	// unbind the buffer
}

void Lane::updateWaterOnGPU()
{
	glutil::Disable<GL_CULL_FACE> cullFaceState;
	glutil::Disable<GL_DEPTH_TEST> depthTestState;

	GLint savedVp[4];
	glGetIntegerv(GL_VIEWPORT, savedVp);
	glViewport(0, 0, gNbVerticesPerRing, gNbRings);

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

			waterSimulationProgram->sendUniform("gTexelSize", vec2(1.f/gNbVerticesPerRing, 1.f/gNbRings));
			waterSimulationProgram->sendUniform("gTime", gData.curFrameTime.asSeconds());

			// we use an algorithm from
			// http://collective.valve-erc.com/index.php?go=water_simulation
			// The params could be dynamically changed every frame of course

			const float C = PARAM_C; // ripple speed
			const float D = PARAM_D; // distance
			const float U = PARAM_U; // viscosity
			const float T = PARAM_T; // time
			const float TERM1 = ( 4.0f - 8.0f*C*C*T*T/(D*D) ) / (U*T+2) ;
			const float TERM2 = ( U*T-2.0f ) / (U*T+2.0f) ;
			const float TERM3 = ( 2.0f * C*C*T*T/(D*D) ) / (U*T+2) ;

			// Scale simulation speed anisotropically to roughly compensate for the unequal vertices distribution
			const float k = gLaneLength / m_curKeyframe.precomp.capsulePerimeter;
			const vec2 scaledTerm3 = TERM3 * vec2(4.0f * k / (1+1+k+k), 4.0f / (1+1+k+k));

			waterSimulationProgram->sendUniform("gTerm1", TERM1);
			waterSimulationProgram->sendUniform("gTerm2", TERM2);
			waterSimulationProgram->sendUniform("gTerm3", scaledTerm3);

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

		waterNormalsProgram->sendUniform("gTexelSize", vec2(1.f/gNbVerticesPerRing, 1.f/gNbRings));
		waterNormalsProgram->sendUniform("gDistBetweenTexels", vec2(kf.precomp.capsulePerimeter / gNbVerticesPerRing, kf.precomp.capsulePerimeter / gNbRings));

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

void Lane::setCurBeat(float curBeat)
{
	assert(m_track);
	m_curKeyframe.beat = clamp(curBeat, m_track->keyframes.front().beat, m_track->keyframes.back().beat);
	// Rest of m_curKeyframe will be updated at update(), using m_curKeyframe.beat as the base
}

void Lane::setupAtom(Atom* pAtom)
{
	pAtom->setModelResourcePtr(m_atomBlueprint);
	
	vec3 position = vec3(getPosition().x, getPosition().y, 2.f);
	pAtom->setPosition(position);
}
