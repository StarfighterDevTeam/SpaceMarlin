#version 400 core
#include "SharedDefines.h"

HANDLE_PROG_LANE(HANDLE_VERTEX_NO_ACTION, HANDLE_UNIFORM_DECLARE, HANDLE_ATTRIBUTE_NO_ACTION)

//uniform sampler2D texAlbedo;
uniform samplerCube texCubemap;

//in vec2 varUv;
in vec3 varNormal;
in vec3 varWorldSpaceViewVec;

layout(location=0) out vec4 fragColor;

void main()
{
	//vec4 tAlbedo = texture2D(texAlbedo, varUv);
	
	vec3 normal = normalize(varNormal);
	vec3 viewVec = normalize(varWorldSpaceViewVec);
	vec3 reflectVec = reflect(viewVec, normal);
	vec4 tCubemap = texture(texCubemap, reflectVec);
	fragColor = vec4(tCubemap.rgb + vec3(0.15,0.3,0.4), 1);
	//if(fragColor.a < 0.5)
	//	fragColor = vec4(1,1,1,1);
}
