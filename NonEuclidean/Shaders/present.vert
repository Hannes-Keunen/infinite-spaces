#version 450

//Inputs
layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_uv;

//Outputs
out vec2 ex_uv;

void main(void) {
	gl_Position = vec4(in_pos, 0.0, 1.0);
	ex_uv = in_uv;
}
