#version 400 core

#include "../../src/SharedDefines.h"

precision highp float;
precision highp int;

layout(location=PROG_SKYBOX_ATTRIB_POSITIONS)	in vec3 pos;

uniform mat4 gModelViewProjMtx;

out vec3 varUv;

void main()
{
	varUv = pos;
	gl_Position = gModelViewProjMtx * vec4(8*pos, 1.0);
}
