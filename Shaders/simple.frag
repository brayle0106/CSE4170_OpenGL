#version 330

in vec4 v_color;

uniform bool u_draw_silhouette = false;

layout (location = 0) out vec4 final_color;

void main(void) {
    if(u_draw_silhouette) final_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    else final_color = v_color;
}
