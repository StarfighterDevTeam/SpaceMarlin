#pragma once

#ifndef _GPU_PROGRAM_MANAGER_H
#define _GPU_PROGRAM_MANAGER_H

#include "glutil\GPUProgram.h"

class GPUProgramManager
{
public:
	enum ProgramId
	{
		PROG_MODEL,
		//PROG_SIMPLE,

		PROG_COUNT,
	};

	void		init();
	void		shut();
	void		update();

	const GPUProgram* getProgram(ProgramId id) const {return m_programs[id];}

private:
	GPUProgram*	m_programs[PROG_COUNT];
};

#endif // _GPU_PROGRAM_MANAGER_H
