#include "Lane.h"
#include "Drawer.h"
#include "GPUProgramManager.h"
#include "Camera.h"
#include "glutil/glutil.h"
#include "SharedDefines.h"

static const int gGridSize = 100;

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
#endif
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

	const float fHalfSize = 10.f;

	std::vector<VtxLane>* pInitVertices = NULL;
#ifdef _LANE_USES_GPU
	std::vector<VtxLane> vertices;
	pInitVertices = &vertices;

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
				GL_R16F,
				gGridSize, gGridSize,
				0,
				GL_RED,
				GL_FLOAT,
				NULL);
	}
#else
	pInitVertices = &m_vertices[0];
#endif

	// Init vertices positions
	pInitVertices->resize(gGridSize*gGridSize);
	
	VtxLane vtx;
	vtx.pos.y = 0.f;
	vtx.normal = vec3(0,1,0);
	for(int z=0 ; z < gGridSize ; z++)
	{
		vtx.pos.z = (z-gGridSize/2) * fHalfSize / gGridSize;
		for(int x=0 ; x < gGridSize ; x++)
		{
			vtx.pos.x = (x-gGridSize/2) * fHalfSize / gGridSize;
			(*pInitVertices)[x + z*gGridSize] = vtx;
		}
	}

#if !defined(_LANE_USES_GPU)
	m_vertices[1] = m_vertices[2] = m_vertices[0];
#endif

	for(int z = 0 ; z < gGridSize-1 ; z++)
	{
		for(int x = 0 ; x < gGridSize-1 ; x++)
		{
			m_indices.push_back((x+0) + (z+0)*gGridSize);
			m_indices.push_back((x+0) + (z+1)*gGridSize);
			m_indices.push_back((x+1) + (z+1)*gGridSize);
			
			m_indices.push_back((x+0) + (z+0)*gGridSize);
			m_indices.push_back((x+1) + (z+1)*gGridSize);
			m_indices.push_back((x+1) + (z+0)*gGridSize);
		}
	}
	
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

		// Vertex buffer
		glEnableVertexAttribArray(PROG_LANE_ATTRIB_POSITIONS);
		glEnableVertexAttribArray(PROG_LANE_ATTRIB_NORMALS);

		glVertexAttribPointer(PROG_LANE_ATTRIB_POSITIONS, sizeof(VtxLane::pos)		/sizeof(GLfloat),	GL_FLOAT,	GL_FALSE,	sizeof(VtxLane), (const GLvoid*)offsetof(VtxLane, pos));
		glVertexAttribPointer(PROG_LANE_ATTRIB_NORMALS	, sizeof(VtxLane::normal)	/sizeof(GLfloat),	GL_FLOAT,	GL_FALSE,	sizeof(VtxLane), (const GLvoid*)offsetof(VtxLane, normal));

		glBindVertexArray(0);
	}
}

void Lane::shut()
{
	assert(m_indices.size() > 0);
	m_indices.clear();

#ifdef _LANE_USES_GPU
	assert(m_heightsTexId[0] != INVALID_GL_ID);
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

void Lane::draw(const Camera& camera, GLuint texCubemapId)
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

	mat4 modelMtx;
	mat4 modelViewMtx = camera.getViewMtx() * modelMtx;
	mat4 modelViewProjMtx = camera.getViewProjMtx() * modelMtx;
	const GPUProgram* laneProgram = gData.gpuProgramMgr->getProgram(PROG_LANE);
	laneProgram->use();
	laneProgram->sendUniform("gModelViewMtx", modelViewMtx);
	laneProgram->sendUniform("gModelViewProjMtx", modelViewProjMtx);
	//laneProgram->sendUniform("texAlbedo", 0);
	laneProgram->sendUniform("gTime", gData.curFrameTime.asSeconds());
	laneProgram->sendUniform("gModelMtx", modelMtx);
	laneProgram->sendUniform("gWorldSpaceCamPos", camera.getPosition());

	if(texCubemapId != INVALID_GL_ID)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texCubemapId);
	}
	laneProgram->sendUniform("texCubemap", 0);

	glBindVertexArray(m_vertexArrayId);

#if !defined(_LANE_USES_GPU)
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, m_vertices[m_curBufferIdx].size() * sizeof(VtxLane), &m_vertices[m_curBufferIdx][0], GL_STREAM_DRAW);
#endif

	glDisable(GL_CULL_FACE);

	glDrawElements(
		GL_TRIANGLES,
		(GLsizei)m_indices.size(),
		GL_UNSIGNED_SHORT,
		(void*)0
	);

	glEnable(GL_CULL_FACE);
	glBindVertexArray(0);
}

void Lane::update()
{
#if !defined(_LANE_USES_GPU)
	// BEGIN TEST
	{
		static float gfLastTimeSecs = 0.f;
		if(gData.curFrameTime.asSeconds() - gfLastTimeSecs > 5.f)
		{
			gfLastTimeSecs = gData.curFrameTime.asSeconds();
			m_vertices[m_curBufferIdx][gGridSize/2 + (gGridSize/2)*gGridSize].pos.y = 1.f;
		}
	}
	// END TEST

	// BEGIN TEST
	{
		//const float fHalfSize = 10.f;
		//for(VtxLane& vtx : m_vertices[m_curBufferIdx])
		//{
		//	const float dx = vtx.pos.x / fHalfSize;
		//	const float dy = vtx.pos.z / fHalfSize;
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
		for(int y=1 ; y<gGridSize-1 ; y++)	// don't do anything with border values
		{
			VtxLane *row = buf + y*(gGridSize);
			VtxLane *row1 = buf1 + y*(gGridSize);
			VtxLane *row1up = buf1 + (y-1)*(gGridSize);
			VtxLane *row1down = buf1 + (y+1)*(gGridSize);
			VtxLane *row2 = buf2 + y*(gGridSize);
			for(int x=1;x<gGridSize-1;x++)
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
