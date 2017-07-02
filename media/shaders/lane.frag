#version 400 core

precision highp float;
precision mediump int;

//uniform sampler2D texAlbedo;
uniform float gTime;

in vec2 varUv;
in vec3 varColor;

layout(location=0) out vec4 fragColor;

void main()
{
	//vec4 tAlbedo = texture2D(texAlbedo, varUv);
	
	//fragColor = vec4(tAlbedo.rgb, 1);
	//if(tAlbedo.a < 0.5)
	//	discard;
	//fragColor = vec4(0, 0, 1, 1);
	fragColor = vec4(varColor, 1);
}
