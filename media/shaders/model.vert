#version 400 core
#include "SharedDefines.h"

HANDLE_PROG_MODEL(HANDLE_VERTEX_NO_ACTION, HANDLE_UNIFORM_DECLARE, HANDLE_ATTRIBUTE_DECLARE)

out vec2 varUv;
out vec3 varLocalPos;

void main()
{
	vec3 lPos = pos;
	varLocalPos = lPos;
	float enveloppe = (lPos.x+2);
	//enveloppe = enveloppe < 0 ? 0 : enveloppe;
	lPos.y += enveloppe * sin(15*gTime + lPos.x)*0.05;

	varUv = uv;
	gl_Position = gLocalToProjMtx * vec4(lPos, 1.0);
}
