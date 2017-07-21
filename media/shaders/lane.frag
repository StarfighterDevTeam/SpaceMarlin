#version 400 core
#include "SharedDefines.h"

PROG_FRAGMENT_SHADER(PROG_LANE)

in vec3 varViewSpaceNormal;
in vec3 varViewSpacePos;
in vec3 varWorldSpaceNormal;
in vec3 varWorldSpaceViewVec;

in vec3 varDebug;

layout(location=0) out vec4 fragColor;

void main()
{
	vec3 wNormal = normalize(varWorldSpaceNormal);
	vec3 wViewVec = normalize(varWorldSpaceViewVec);
	vec3 wReflectVec = reflect(wViewVec, wNormal);
	vec4 tCubemap = texture(texCubemap, wReflectVec);
	vec3 specular = tCubemap.rgb;
	vec3 ambient = vec3(0.15,0.3,0.4);
	
	vec3 vViewVec = -normalize(varViewSpacePos);
	vec3 vNormal = normalize(varViewSpaceNormal);
	float refractiveIndex = 1.5;
	vec3 vRefractedDirection = refract(-vViewVec, vNormal, 1.0 / refractiveIndex);
	float refractionLength = 0.5;
	vec3 vRefractedPos = varViewSpacePos + vRefractedDirection*refractionLength;
	vec4 projRefractedPos = gViewToProjMtx * vec4(vRefractedPos,1);
	vec3 ndcRefractedPos = projRefractedPos.xyz / projRefractedPos.w;
	vec2 refractionUv = (ndcRefractedPos.xy)*0.5 + 0.5;
	
	// https://en.wikipedia.org/wiki/Schlick%27s_approximation
	float r0 = 0.5;	// ( (n1-n2)/(n1+n2) )²
	float reflectionFactor = r0 + (1-r0) * pow(1-abs(dot(vNormal, vViewVec)), 5);	// Schlick's Fresnel approximation
	vec3 color =	reflectionFactor * (specular + ambient) +
					(1-reflectionFactor) * texture(texRefraction, refractionUv).rgb;
	fragColor = vec4(color, 1);

	fragColor = vec4(varDebug,1);
	//fragColor = vec4(varViewSpaceNormal.rgb,1);
	//fragColor = vec4(varWorldSpaceNormal.rgb,1);
	//fragColor = vec4(vViewVec.rgb,1);
	//fragColor = vec4(vRefractedDirection,1);
	//if(tCubemap.a < 0.5)
	//	discard;
}
