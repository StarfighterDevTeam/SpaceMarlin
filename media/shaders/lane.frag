#version 400 core
#include "SharedDefines.h"

PROG_FRAGMENT_SHADER(PROG_LANE)

in vec3 varNormal;
in vec3 varWorldSpaceViewVec;

layout(location=0) out vec4 fragColor;

void main()
{
	vec3 normal = normalize(varNormal);
	vec3 viewVec = normalize(varWorldSpaceViewVec);
	vec3 reflectVec = reflect(viewVec, normal);
	vec4 tCubemap = texture(texCubemap, reflectVec);
	fragColor = vec4(tCubemap.rgb + vec3(0.15,0.3,0.4), 0.7);

	// TODO
	//float refractiveIndex = 1.5;
	//vec3 refractedDirection = refract(viewVec, normal, 1.0 / refractiveIndex);
	//vec2 refractionUv = gl_FragCoord.xy / gVpSize;
	//vec3 refractionColor = texture(texRefraction, refractionUv).rgb;
	//vec3 color = refractionColor + tCubemap.rgb;
	//fragColor = vec4(color, 1);
	//if(tCubemap.a < 0.5)
	//	discard;
}
