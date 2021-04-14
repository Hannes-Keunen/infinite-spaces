#version 450
precision highp float;

//Inputs
uniform sampler2D tex;
in vec2 ex_uv;

//Outputs
out vec4 color;

void main(void) {
	color = texture2D(tex, ex_uv);
	// color = vec4(0.8, 0.2, 0.2, 1.0);
}
