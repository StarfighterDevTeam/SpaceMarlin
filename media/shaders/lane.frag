#version 400 core

precision highp float;
precision mediump int;

uniform sampler2D texAlbedo;
uniform float gTime;

//layout(location=0) in vec2 varUv;
in vec2 varUv;

layout(location=0) out vec4 fragColor;

void main()
{
	//frag_color = texture(texunit, var_texcoords);
	//fragColor = vec4(0.0,1.0,0.0,1.0);

	vec4 tAlbedo = texture2D(texAlbedo, varUv);
	
	fragColor = vec4(tAlbedo.rgb, 1);
	//fragColor.r = cos(gTime);
	if(tAlbedo.a < 0.5)
		discard;
}
