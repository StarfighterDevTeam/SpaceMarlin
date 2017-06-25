// model.frag

#version 330 core

precision highp float;
precision mediump int;

uniform sampler2D texAlbedo;

//layout(location=0) in vec2 varUv;
varying vec2 varUv;

layout(location=0) out vec4 fragColor;

void main()
{
	//frag_color = texture(texunit, var_texcoords);
	//fragColor = vec4(0.0,1.0,0.0,1.0);

	vec4 tAlbedo = texture2D(texAlbedo, varUv);
	
	fragColor = vec4(tAlbedo.rgb, 1);
	//fragColor = vec4(varUv.xy,0, 1);
}
