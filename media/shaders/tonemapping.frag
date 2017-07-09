#version 400 core
#include "SharedDefines.h"

HANDLE_PROG_SIMPLE(HANDLE_VERTEX_NO_ACTION, HANDLE_UNIFORM_DECLARE, HANDLE_ATTRIBUTE_NO_ACTION)

uniform sampler2D texScene;

layout(location=0) out vec4 fragColor;

in vec2 varUv;

void main()
{
	// TODO
	fragColor = textureLod(texScene, varUv, 0);
}
