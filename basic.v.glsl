#version 450

in vec2 coord2d;
in vec3 color;
uniform float t;

out vec3 f_color;

void main(void){
	vec2 new_coord = coord2d + t;
    gl_Position = vec4(new_coord, 0, 1);
    f_color = color;
}
