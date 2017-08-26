#version 450 

out vec4 color;

void main(void){
    color = vec4(gl_FragCoord.x/200, gl_FragCoord.y/200, 1.0, 1.0);
}

