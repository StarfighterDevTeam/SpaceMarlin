#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_WATER_NORMALS)

layout(location=0) out vec4 fragNormal;

in vec2 varUv;

void main()
{
	fragNormal = vec4(0,1,0,1);
}
