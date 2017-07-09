#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_TEXTURE_2D)

out vec2 varUv;

void main()
{
	varUv = uv;
	gl_Position = vec4(pos, 0.0, 1.0);
}
