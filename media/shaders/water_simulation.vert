#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_WATER_SIMULATION)

void main()
{
	gl_Position = vec4(pos, 0.0, 1.0);
}
