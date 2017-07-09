#version 400 core
#include "SharedDefines.h"

PROG_VERTEX_SHADER(PROG_LANE)

//out vec2 varUv;
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
	vec3 worldSpacePos = vec3(gLocalToWorldMtx * vec4(pos, 1));
	varWorldSpaceViewVec = worldSpacePos - gWorldSpaceCamPos;
	varNormal = normal;
	//float sizeFactor = 5;
	//varUv = (pos.xz / sizeFactor) * 0.5 + 0.5;
	gl_Position = gLocalToProjMtx * vec4(pos, 1);
}
