#version 400 core

#include "../../src/SharedDefines.h"

precision highp float;
precision highp int;

layout(location=PROG_BACKGROUND_ATTRIB_POSITIONS)	in vec3 pos;

uniform mat4 gModelViewProjMtx;

out vec2 varUv;

void main()
{
	varUv = 0.5*(pos.xy + vec2(1,1));
	//gl_Position = gModelViewProjMtx * vec4(pos, 1.0);
	gl_Position = vec4(pos, 1.0);
}
