#version 400 core

#include "../../src/SharedDefines.h"

precision highp float;
precision highp int;

layout(location=PROG_BACKGROUND_ATTRIB_POSITIONS)	in vec3 pos;

uniform mat4 gModelViewProjMtx;

out vec3 varPos;

void main()
{
	varPos = pos;
	gl_Position = vec4(pos, 1.0);
}
