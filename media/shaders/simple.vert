#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_SIMPLE)

out vec4 varColor;

void main()
{
	varColor = color;
	gl_Position = gLocalToProjMtx * vec4(pos, 1.0);
}
