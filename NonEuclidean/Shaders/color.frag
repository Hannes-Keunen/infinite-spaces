#version 450
precision highp float;

#define LIGHT vec3(0.36, 0.80, 0.48)

//Inputs
uniform vec3 color;
uniform int objId;

in vec3 ex_normal;

//Outputs
layout (location = 0) out vec4 out_color;
layout (location = 1) out int out_objId;

void main(void) {
	float s = dot(ex_normal, LIGHT)*0.5 + 0.5;
	out_color = vec4(color * s, 1.0);
	out_objId = objId;
}
