#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_WATER_NORMALS)

layout(location=0) out vec4 fragNormal;

in vec2 varUv;

void main()
{
	float height	= textureLod(texHeights, varUv, 0).r;
	float heightR	= textureLod(texHeights, varUv + vec2(+gTexelSize.x,0), 0).r;
	//float heightL	= textureLod(texHeights, varUv + vec2(-gTexelSize.x,0), 0).r;
	float heightT	= textureLod(texHeights, varUv + vec2(0,+gTexelSize.x), 0).r;
	//float heightB	= textureLod(texHeights, varUv + vec2(0,-gTexelSize.x), 0).r;
	vec3 right = vec3(gDistBetweenTexels.x, heightR-height, 0);
	vec3 front = vec3(0, heightT-height, -gDistBetweenTexels.y);
	fragNormal = vec4(normalize(cross(right, front)), 1);
}
