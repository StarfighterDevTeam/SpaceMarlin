#version 400 core

#include "../../src/SharedDefines.h"

precision highp float;
precision highp int;

layout(location=PROG_MODEL_ATTRIB_POSITIONS)	in vec3 pos;
layout(location=PROG_MODEL_ATTRIB_UVS)			in vec2 uv;
layout(location=PROG_MODEL_ATTRIB_NORMALS)		in vec3 normal;

uniform mat4 gModelViewProjMtx;

out vec2 varUv;

out vec3 varLocalPos;
uniform float gTime;

void main()
{
	vec3 lPos = pos;
	varLocalPos = lPos;
	float enveloppe = (lPos.x+2);
	//enveloppe = enveloppe < 0 ? 0 : enveloppe;
	lPos.y += enveloppe * sin(15*gTime + lPos.x)*0.05;

	varUv = uv;
	gl_Position = gModelViewProjMtx * vec4(lPos, 1.0);
}
