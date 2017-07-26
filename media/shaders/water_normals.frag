#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_WATER_NORMALS)

layout(location=0) out vec4 fragNormal;

in vec2 varUv;

void main()
{
	float height	= texture(texHeights, varUv).r;
	float heightR	= texture(texHeights, varUv + vec2(+gTexelSize.x,0)).r;
	//float heightL	= texture(texHeights, varUv + vec2(-gTexelSize.x,0)).r;
	float heightT	= texture(texHeights, varUv + vec2(0,+gTexelSize.x)).r;
	//float heightB	= texture(texHeights, varUv + vec2(0,-gTexelSize.x)).r;
	vec3 right = vec3(gDistBetweenTexels.x, heightR-height, 0);
	vec3 front = vec3(0, heightT-height, -gDistBetweenTexels.y);
	fragNormal = vec4(normalize(cross(right, front)), 1);
}
