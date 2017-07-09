#version 400 core
#include "SharedDefines.h"

HANDLE_PROG_SKYBOX(HANDLE_VERTEX_NO_ACTION, HANDLE_UNIFORM_DECLARE, HANDLE_ATTRIBUTE_NO_ACTION)

uniform samplerCube texSky;

in vec2 varUv;

layout(location=0) out vec4 fragColor;

void main()
{
	vec3 ndc = vec3(varUv*2-1, -1);
	vec4 worldSpaceDir = gProjToWorldRotMtx * vec4(ndc,1);
	worldSpaceDir.xyz /= worldSpaceDir.w;
	fragColor = texture(texSky, worldSpaceDir.xyz);
	if(fragColor.a < 0.5)
		discard;
}
