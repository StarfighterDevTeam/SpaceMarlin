#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_LANE)

#define _LANE_USES_GPU

out vec3 varNormal;
out vec3 varWorldSpaceViewVec;

void main()
{
	//float dot_product = dot(pos.xz, vec2(12.9898,78.233));
	//vec3 r = vec3(
	//	fract(sin(dot_product    ) * 43758.5453),
	//	fract(sin(dot_product*2.0) * 43758.5453),
	//	fract(sin(dot_product*3.0) * 43758.5453));
	//varColor = r;
	vec3 lPos = pos;

	vec3 worldSpacePos = vec3(gLocalToWorldMtx * vec4(lPos, 1));
	varWorldSpaceViewVec = worldSpacePos - gWorldSpaceCamPos;
	varNormal = normal;

#ifdef _LANE_USES_GPU
	lPos.y = textureLod(texHeights, uv, 0).r;
	float heightRight	= textureLod(texHeights, uv + vec2(+gTexelSize.x,0), 0).r;
	float heightTop		= textureLod(texHeights, uv + vec2(0, +gTexelSize.y), 0).r;
	vec3 posRight	= vec3(lPos.x + gDistBetweenTexels.x	, heightRight,	lPos.z);
	vec3 posTop		= vec3(lPos.x							, heightTop,	lPos.z + gDistBetweenTexels.y);
	varNormal = -normalize(cross(posRight-lPos, posTop-lPos));
#endif
	gl_Position = gLocalToProjMtx * vec4(lPos, 1);
}
