#pragma once

#ifndef _GPU_PROGRAM_MANAGER_H
#define _GPU_PROGRAM_MANAGER_H

#include "glutil\GPUProgram.h"

enum GPUProgramId
{
	PROG_MODEL,
	PROG_SIMPLE,

	PROG_COUNT,
};

struct VtxSimple
{
	glm::vec3 pos;
	glm::vec4 color;
};

struct VtxModel
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
};

class GPUProgramManager
{
public:
	void		init();
	void		shut();
	void		update();

	const GPUProgram* getProgram(GPUProgramId id) const {return m_programs[id];}

private:
	GPUProgram*	m_programs[PROG_COUNT];
};

#endif // _GPU_PROGRAM_MANAGER_H
