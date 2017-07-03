#version 400 core

#include "../../src/SharedDefines.h"

precision highp float;
precision highp int;

layout(location=PROG_LANE_ATTRIB_POSITIONS)	in vec3 pos;
layout(location=PROG_LANE_ATTRIB_NORMALS)	in vec3 normal;

uniform mat4 gModelViewProjMtx;

out vec2 varUv;
out vec3 varColor;

void main()
{
	float dot_product = dot(pos.xz, vec2(12.9898,78.233));
	vec3 r = vec3(
		fract(sin(dot_product    ) * 43758.5453),
		fract(sin(dot_product*2.0) * 43758.5453),
		fract(sin(dot_product*3.0) * 43758.5453));
	varColor = r;
	varUv = pos.xz;
	gl_Position = gModelViewProjMtx * vec4(pos, 1.0);
}
