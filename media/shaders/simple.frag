#version 400 core
#include "SharedDefines.h"

HANDLE_PROG_SIMPLE(HANDLE_VERTEX_NO_ACTION, HANDLE_UNIFORM_DECLARE, HANDLE_ATTRIBUTE_NO_ACTION)

in vec4 varColor;

layout(location=0) out vec4 fragColor;

void main()
{
	fragColor = varColor;
}
