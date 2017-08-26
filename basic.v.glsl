#version 450

in vec2 coord2d;

void main(void){
    gl_Position = vec4(coord2d, 0, 1);
}
