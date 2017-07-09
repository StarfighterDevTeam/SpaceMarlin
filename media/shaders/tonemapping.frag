#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_TONEMAPPING)

layout(location=0) out vec4 fragColor;

in vec2 varUv;

void main()
{
	// TODO
	fragColor = textureLod(texScene, varUv, 0);
}
