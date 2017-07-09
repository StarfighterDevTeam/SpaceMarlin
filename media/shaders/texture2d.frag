#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_TEXTURE_2D)

in vec2 varUv;

layout(location=0) out vec4 fragColor;

void main()
{
	fragColor = texture(tex, varUv);
}
