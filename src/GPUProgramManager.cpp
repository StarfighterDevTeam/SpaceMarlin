#include "GPUProgramManager.h"
#include "SharedDefines.h"

void GPUProgramManager::init()
{
	m_programs[PROG_MODEL] = new GPUProgram("media/shaders/model.vert", "media/shaders/model.frag");
	if(m_programs[PROG_MODEL]->compileAndAttach())
	{
		m_programs[PROG_MODEL]->bindAttribLocation(MODEL_ATTRIB_POSITIONS,	"pos");
		m_programs[PROG_MODEL]->bindAttribLocation(MODEL_ATTRIB_UVS,		"uv");
		m_programs[PROG_MODEL]->bindAttribLocation(MODEL_ATTRIB_NORMALS,	"normal");

		if(m_programs[PROG_MODEL]->link())
		{
			std::list<std::string> uniformNames;
			uniformNames.push_back("gModelViewProjMtx");
			uniformNames.push_back("texAlbedo");
			m_programs[PROG_MODEL]->setUniformNames(uniformNames);
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
