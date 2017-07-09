#version 400 core
#include "SharedDefines.h"

HANDLE_PROG_SIMPLE(HANDLE_VERTEX_NO_ACTION, HANDLE_UNIFORM_DECLARE, HANDLE_ATTRIBUTE_DECLARE)

out vec4 varColor;

void main()
{
	varColor = color;
	gl_Position = gLocalToProjMtx * vec4(pos, 1.0);
}
