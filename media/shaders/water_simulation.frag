#version 400 core
#include "SharedDefines.h"
PROG_FRAGMENT_SHADER(PROG_WATER_SIMULATION)

layout(location=0) out float fragHeight;

in vec2 varUv;

void main()
{
	fragHeight = 10*max(0, -0.7 + 1-length(varUv*2-1));

	//fragHeight =
	//	gTerm1 * texture(texHeights1, varUv).r +
	//	gTerm2 * texture(texHeights2, varUv).r +
	//	gTerm3 * (
	//		texture(texHeights1, varUv + gTexelSize * vec2(-1, 0)).r +
	//		texture(texHeights1, varUv + gTexelSize * vec2(+1, 0)).r +
	//		texture(texHeights1, varUv + gTexelSize * vec2(0, -1)).r +
	//		texture(texHeights1, varUv + gTexelSize * vec2(0, +1)).r
	//		);
}
