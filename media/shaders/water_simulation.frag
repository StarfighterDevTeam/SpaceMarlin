#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_WATER_SIMULATION)

layout(location=0) out float fragHeight;

void main()
{
	// TODO
	fragHeight = 0.5;
}
