#include "GPUProgramManager.h"
#include "SharedDefines.h"

void GPUProgramManager::init()
{
	// ---- PROG_MODEL ----
	m_programs[PROG_MODEL] = new GPUProgram("media/shaders/model.vert", "media/shaders/model.frag");
	if(m_programs[PROG_MODEL]->compileAndAttach())
	{
		m_programs[PROG_MODEL]->bindAttribLocation(PROG_MODEL_ATTRIB_POSITIONS,	"pos");
		m_programs[PROG_MODEL]->bindAttribLocation(PROG_MODEL_ATTRIB_UVS,		"uv");
		m_programs[PROG_MODEL]->bindAttribLocation(PROG_MODEL_ATTRIB_NORMALS,	"normal");

		if(m_programs[PROG_MODEL]->link())
		{
			std::list<std::string> uniformNames;
			uniformNames.push_back("gModelViewProjMtx");
			uniformNames.push_back("texAlbedo");
			m_programs[PROG_MODEL]->setUniformNames(uniformNames);
		}
	}

	// ---- PROG_SIMPLE ----
	m_programs[PROG_SIMPLE] = new GPUProgram("media/shaders/simple.vert", "media/shaders/simple.frag");
	if(m_programs[PROG_SIMPLE]->compileAndAttach())
	{
		m_programs[PROG_SIMPLE]->bindAttribLocation(PROG_SIMPLE_ATTRIB_POSITIONS,	"pos");
		m_programs[PROG_SIMPLE]->bindAttribLocation(PROG_SIMPLE_ATTRIB_COLORS,		"color");

		if(m_programs[PROG_SIMPLE]->link())
		{
			std::list<std::string> uniformNames;
			uniformNames.push_back("gModelViewProjMtx");
			m_programs[PROG_SIMPLE]->setUniformNames(uniformNames);
		}
	}
}

void GPUProgramManager::shut()
{
	for(int i=0 ; i < _countof(m_programs) ; i++)
		SAFE_DELETE(m_programs[i]);
}

void GPUProgramManager::update()
{
}
