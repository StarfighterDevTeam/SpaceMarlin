#version 400 core
#include "SharedDefines.h"
PROG_VERTEX_SHADER(PROG_LANE)

#define _LANE_USES_GPU

out vec3 varViewSpaceNormal;
out vec3 varViewSpacePos;
out vec3 varWorldSpaceNormal;
out vec3 varWorldSpaceViewVec;

void main()
{
	//float dot_product = dot(pos.xz, vec2(12.9898,78.233));
	//vec3 r = vec3(
	//	fract(sin(dot_product    ) * 43758.5453),
	//	fract(sin(dot_product*2.0) * 43758.5453),
	//	fract(sin(dot_product*3.0) * 43758.5453));
	vec3 lPos = pos;

	vec3 worldSpacePos = vec3(gLocalToWorldMtx * vec4(lPos, 1));
	varWorldSpaceViewVec = worldSpacePos - gWorldSpaceCamPos;
	vec3 lNormal = normal;

	float waterHeight = 0;
#ifdef _LANE_USES_GPU
	waterHeight = textureLod(texHeights, uv, 0).r;
	lPos.y = waterHeight;
	float heightRight	= textureLod(texHeights, uv + vec2(+gTexelSize.x,0), 0).r;
	float heightTop		= textureLod(texHeights, uv + vec2(0, +gTexelSize.y), 0).r;
	vec3 posRight	= vec3(lPos.x + gDistBetweenTexels.x	, heightRight,	lPos.z);
	vec3 posTop		= vec3(lPos.x							, heightTop,	lPos.z + gDistBetweenTexels.y);
	lNormal = normalize(cross(posTop-lPos, posRight-lPos));
#endif

	float gSideNbVtx = 100;	// GROS PORC
	float gGridSize = 10.f;	// GROS PORC
	float cylinderRadius = 3;
	//float cylinderRadius = 3 + 2*sin(gTime*20);
	float u = 0.5 * (lPos.x/ ((gGridSize - (gGridSize/gSideNbVtx))/2) ) + 0.5;	// in [0,1]
	//float eps = 0.01;
	float eps = -0.0;
	float angle = (1+eps) * (-3.1415/2 + 2*3.1415*u);	// in [0,pi]
	lPos.y = (cylinderRadius+waterHeight) * sin(angle) - cylinderRadius;
	lPos.x = (cylinderRadius+waterHeight) * cos(angle);

	//lPos.x += 3*sin(gTime*1 + 0.3*lPos.z);
	//lPos.z *= 10;

	float rotAngle = angle - 3.1415/2;
	mat3 rotMtx = mat3(
			cos(rotAngle),	sin(rotAngle),	0,
			-sin(rotAngle),	cos(rotAngle),	0,
			0,				0,				1
		);

	lNormal = rotMtx * lNormal;

	varWorldSpaceNormal = mat3(gLocalToWorldMtx) * lNormal;
	varViewSpaceNormal = mat3(gLocalToViewMtx) * lNormal;
	varViewSpacePos = vec3( gLocalToViewMtx * vec4(lPos,1) );
	
	gl_Position = gLocalToProjMtx * vec4(lPos, 1);
}
