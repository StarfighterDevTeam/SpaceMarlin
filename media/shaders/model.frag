#version 400 core
#include "SharedDefines.h"

HANDLE_PROG_MODEL(HANDLE_VERTEX_NO_ACTION, HANDLE_UNIFORM_DECLARE, HANDLE_ATTRIBUTE_NO_ACTION)

uniform sampler2D texAlbedo;

in vec2 varUv;
in vec3 varLocalPos;

layout(location=0) out vec4 fragColor;

void main()
{
	vec4 tAlbedo = texture2D(texAlbedo, varUv);
	
	fragColor = tAlbedo;
	if(fragColor.a < 0.5)
		discard;
}
