#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_MODEL)

out vec2 varUv;

void main()
{
	vec3 lPos = pos;
	varUv = uv;
	gl_Position = gLocalToProjMtx * vec4(lPos, 1.0);
}
