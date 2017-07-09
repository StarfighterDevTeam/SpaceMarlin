#version 400 core
#include "SharedDefines.h"

HANDLE_PROG_SKYBOX(HANDLE_VERTEX_NO_ACTION, HANDLE_UNIFORM_DECLARE, HANDLE_ATTRIBUTE_DECLARE)

out vec2 varUv;

void main()
{
	varUv = uv;
	gl_Position = vec4(pos,0,1);
}
