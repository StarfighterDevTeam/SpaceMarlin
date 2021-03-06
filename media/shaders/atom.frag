#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_ATOM)

in vec2 varUv;

layout(location=0) out vec4 fragColor;

void main()
{
	vec4 tAlbedo = texture2D(texAlbedo, varUv);
	fragColor = tAlbedo;
	if(fragColor.a < 0.5)
		discard;
}