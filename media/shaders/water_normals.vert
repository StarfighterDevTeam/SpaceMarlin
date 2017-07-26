#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_WATER_NORMALS)

out vec2 varUv;

void main()
{
	varUv = pos.xy * 0.5 + 0.5;
	gl_Position = vec4(pos, 0.0, 1.0);
}
