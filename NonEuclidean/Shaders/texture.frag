#version 450
precision highp float;

#define LIGHT vec3(0.36, 0.80, 0.48)

//Inputs
uniform sampler2D tex;
uniform int objId;

in vec2 ex_uv;
in vec3 ex_normal;

//Outputs
layout (location = 0) out vec4 color;
layout (location = 1) out int objId_out;

void main(void) {
	float s = dot(ex_normal, LIGHT)*0.5 + 0.5;
	color = vec4(texture(tex, ex_uv).rgb * s, 1.0);
	objId_out = objId;
}
