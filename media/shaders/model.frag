#version 400 core

precision highp float;
precision mediump int;

uniform sampler2D texAlbedo;
uniform float gTime;

in vec2 varUv;
in vec3 varLocalPos;

layout(location=0) out vec4 fragColor;

void main()
{
	vec4 tAlbedo = texture2D(texAlbedo, varUv);
	
	fragColor = tAlbedo;
	if(fragColor.a < 0.5)
		discard;
}
