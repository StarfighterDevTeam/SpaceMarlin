#version 400 core

#include "../../src/SharedDefines.h"

precision highp float;
precision highp int;

layout(location=PROG_LANE_ATTRIB_POSITIONS)	in vec3 pos;

uniform mat4 gModelViewProjMtx;

out vec2 varUv;

void main()
{
	varUv = pos.xz;
	gl_Position = gModelViewProjMtx * vec4(pos, 1.0);
}
