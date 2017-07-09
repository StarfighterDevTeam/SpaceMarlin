#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_SIMPLE)

in vec4 varColor;

layout(location=0) out vec4 fragColor;

void main()
{
	fragColor = varColor;
}
