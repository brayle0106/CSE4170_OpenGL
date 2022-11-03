//
//  DrawScene.cpp
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2022 Sogang University. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "LoadScene.h"

// Begin of shader setup
#include "Shaders/LoadShaders.h"
#include "ShadingInfo.h"

extern SCENE scene;

// for simple shaders
GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS, h_ShaderProgram_GS; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables
GLint loc_draw_silhouette;

// for Phong(Textured) shaders
#define NUMBER_OF_LIGHT_SUPPORTED 4 
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_texture;
GLint loc_flag_texture_mapping;
GLint loc_flag_fog;
GLint loc_u_fragment_alpha, loc_u_draw_cube, loc_u_view_normal_TXPS;

//for Gouraud shader
GLint loc_global_ambient_color_GS;
loc_light_Parameters loc_light_GS[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material_GS;
GLint loc_ModelViewProjectionMatrix_GS, loc_ModelViewMatrix_GS, loc_ModelViewMatrixInvTrans_GS;
GLint loc_light_Test_GS, loc_u_view_normal_GS;
int flag_light_test_GS;

// lights in scene
Light_Parameters light[NUMBER_OF_LIGHT_SUPPORTED];
Light_Parameters light_GS[NUMBER_OF_LIGHT_SUPPORTED];

// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp>
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when ready.
glm::mat4 ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

/*********************************  START: camera *********************************/
typedef enum {
	CAMERA_1,
	CAMERA_2,
	CAMERA_3,
	CAMERA_4,
	NUM_CAMERAS
} CAMERA_INDEX;

typedef struct _Camera {
	float pos[3];
	float uaxis[3], vaxis[3], naxis[3];
	float fovy, aspect_ratio, near_c, far_c;
	int move, rotation_axis;
} Camera;

Camera camera_info[NUM_CAMERAS];
Camera current_camera;

void set_up_scene_lights(void);
glm::vec3 t_pos(3264.227783f, -2080.660400f, 20.040878f);

using glm::mat4;
void set_ViewMatrix_from_camera_frame(void) {
	ViewMatrix = glm::mat4(current_camera.uaxis[0], current_camera.vaxis[0], current_camera.naxis[0], 0.0f,
		current_camera.uaxis[1], current_camera.vaxis[1], current_camera.naxis[1], 0.0f,
		current_camera.uaxis[2], current_camera.vaxis[2], current_camera.naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-current_camera.pos[0], -current_camera.pos[1], -current_camera.pos[2]));
	set_up_scene_lights();
}

void set_current_camera(int camera_num) {
	Camera* pCamera = &camera_info[camera_num];
	memcpy(&current_camera, pCamera, sizeof(Camera));
	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_camera(void) {
	//CAMERA_1 : original view
	Camera* pCamera = &camera_info[CAMERA_1];
	for (int k = 0; k < 3; k++)
	{
		pCamera->pos[k] = scene.camera.e[k];
		pCamera->uaxis[k] = scene.camera.u[k];
		pCamera->vaxis[k] = scene.camera.v[k];
		pCamera->naxis[k] = scene.camera.n[k];
	}
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_2 : bistro view
	pCamera = &camera_info[CAMERA_2];
	pCamera->pos[0] = -2446.801514f; pCamera->pos[1] = 1469.750122f; pCamera->pos[2] = 924.168640f;
	pCamera->uaxis[0] = -0.642788f; pCamera->uaxis[1] = -0.766045f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.262003; pCamera->vaxis[1] = -0.219846; pCamera->vaxis[2] = 0.939693;
	pCamera->naxis[0] = -0.719846f; pCamera->naxis[1] = 0.604023f; pCamera->naxis[2] = 0.342020f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_3 : tree view
	pCamera = &camera_info[CAMERA_3];
	pCamera->pos[0] = 3898.660156f; pCamera->pos[1] = -2321.278076f; pCamera->pos[2] = 513.558533f;
	pCamera->uaxis[0] = 0.357811f; pCamera->uaxis[1] = 0.933725f; pCamera->uaxis[2] = -0.010082f;
	pCamera->vaxis[0] = -0.180880f; pCamera->vaxis[1] = 0.079899f; pCamera->vaxis[2] = 0.980231f;
	pCamera->naxis[0] = 0.916095f; pCamera->naxis[1] = -0.348920f; pCamera->naxis[2] = 0.197483f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_4 : top view
	pCamera = &camera_info[CAMERA_4];
	pCamera->pos[0] = 41.592678f; pCamera->pos[1] = 142.501633f; pCamera->pos[2] = 12449.364258f;
	pCamera->uaxis[0] = 1.0f; pCamera->uaxis[1] = 0.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 1.0f; pCamera->vaxis[2] = 0.0f;
	pCamera->naxis[0] = 0.0f; pCamera->naxis[1] = 0.0f; pCamera->naxis[2] = 1.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;
	set_current_camera(CAMERA_1);
}
/*********************************  END: camera *********************************/

/******************************  START: shader setup ****************************/
// Begin of Callback function definitions
void prepare_shader_program(void) {
	char string[256];

	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	ShaderInfo shader_info_TXPS[3] = {
	{ GL_VERTEX_SHADER, "Shaders/Phong_Tx.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Phong_Tx.frag" },
	{ GL_NONE, NULL }
	};

	ShaderInfo shader_info_GS[3] = {
	{ GL_VERTEX_SHADER, "Shaders/Gouraud.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Gouraud.frag" },
	{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram_simple);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");
	loc_draw_silhouette = glGetUniformLocation(h_ShaderProgram_simple, "u_draw_silhouette");


	h_ShaderProgram_GS = LoadShaders(shader_info_GS);
	glUseProgram(h_ShaderProgram_GS);

	loc_ModelViewProjectionMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light_GS[i].light_on = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light_GS[i].position = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light_GS[i].ambient_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light_GS[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light_GS[i].specular_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light_GS[i].spot_direction = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light_GS[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light_GS[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light_GS[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_GS, string);
	}

	loc_material_GS.ambient_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.ambient_color");
	loc_material_GS.diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.diffuse_color");
	loc_material_GS.specular_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_color");
	loc_material_GS.emissive_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.emissive_color");
	loc_material_GS.specular_exponent = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_exponent");

	loc_light_Test_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_light_Test_GS");
	loc_u_view_normal_GS = glGetUniformLocation(h_ShaderProgram_GS, "u_view_normal");

	h_ShaderProgram_TXPS = LoadShaders(shader_info_TXPS);
	loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_exponent");

	loc_texture = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");
	loc_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_texture_mapping");
	loc_flag_fog = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_fog");

	loc_u_fragment_alpha = glGetUniformLocation(h_ShaderProgram_TXPS, "u_fragment_alpha");
	loc_u_draw_cube = glGetUniformLocation(h_ShaderProgram_TXPS, "u_draw_cube");
	loc_u_view_normal_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_view_normal");
}
/*******************************  END: shder setup ******************************/

/****************************  START: geometry setup ****************************/
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define INDEX_VERTEX_POSITION	0
#define INDEX_NORMAL			1
#define INDEX_TEX_COORD			2

bool b_draw_grid = false;

//axes
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) {
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded axes into graphics memory.\n");
}

void draw_axes(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(8000.0f, 8000.0f, 8000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//grid
#define GRID_LENGTH			(100)
#define NUM_GRID_VETICES	((2 * GRID_LENGTH + 1) * 4)
GLuint grid_VBO, grid_VAO;
GLfloat grid_vertices[NUM_GRID_VETICES][3];
GLfloat grid_color[3] = { 0.5f, 0.5f, 0.5f };

void prepare_grid(void) {
	int vertex_idx = 0;
	for (int x_idx = -GRID_LENGTH; x_idx <= GRID_LENGTH; x_idx++)
	{
		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = -GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	for (int y_idx = -GRID_LENGTH; y_idx <= GRID_LENGTH; y_idx++)
	{
		grid_vertices[vertex_idx][0] = -GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	glGenBuffers(1, &grid_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), &grid_vertices[0][0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &grid_VAO);
	glBindVertexArray(grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VAO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	fprintf(stdout, " * Loaded grid into graphics memory.\n");
}

void draw_grid(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(1.0f);
	glBindVertexArray(grid_VAO);
	glUniform3fv(loc_primitive_color, 1, grid_color);
	glDrawArrays(GL_LINES, 0, NUM_GRID_VETICES);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//bistro_exterior
GLuint* bistro_exterior_VBO;
GLuint* bistro_exterior_VAO;
int* bistro_exterior_n_triangles;
int* bistro_exterior_vertex_offset;
GLfloat** bistro_exterior_vertices;
GLuint* bistro_exterior_texture_names;

int flag_fog;
float cube_alpha, alpha_offset = 0.02f;
bool* flag_texture_mapping;
int light_EC_flag;
int light_WC_flag;
int light_WC_tiger_flag;
int EC_spot_flag;
int view_normal_flag;

void initialize_lights(void) {
	int i;
	glUseProgram(h_ShaderProgram_TXPS);
	glUniform4f(loc_global_ambient_color, 0.7f, 0.7f, 0.7f, 1.0f);
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		glUniform1i(loc_light[i].light_on, 0); // turn off all lights initially
		glUniform4f(loc_light[i].position, 0.0f, 0.0f, 1.0f, 0.0f);
		glUniform4f(loc_light[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 0) {
			glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light[i].diffuse_color, 0.0f, 0.0f, 0.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 0.0f);
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f);
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f);
	}
	glUseProgram(0);

	glUseProgram(h_ShaderProgram_GS);
	glUniform4f(loc_global_ambient_color_GS, 0.7f, 0.7f, 0.7f, 1.0f);
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		glUniform1i(loc_light_GS[i].light_on, 0); // turn off all lights initially
		glUniform4f(loc_light_GS[i].position, 0.0f, 0.0f, 1.0f, 0.0f);
		glUniform4f(loc_light_GS[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 0) {
			glUniform4f(loc_light_GS[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
			glUniform4f(loc_light_GS[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light_GS[i].diffuse_color, 0.0f, 0.0f, 0.0f, 1.0f);
			glUniform4f(loc_light_GS[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		glUniform3f(loc_light_GS[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light_GS[i].spot_exponent, 0.0f);
		glUniform1f(loc_light_GS[i].spot_cutoff_angle, 180.0f);
		glUniform4f(loc_light_GS[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f);
	}
	glUseProgram(0);
}

void set_up_scene_lights(void) {
	light_GS[0].light_on = light_EC_flag;
	light_GS[0].position[0] = 0.0f; light_GS[0].position[1] = 0.0f;
	light_GS[0].position[2] = 0.0f; light_GS[0].position[3] = 1.0f;

	light_GS[0].ambient_color[0] = 0.5f; light_GS[0].ambient_color[1] = 0.5f;
	light_GS[0].ambient_color[2] = 0.5f; light_GS[0].ambient_color[3] = 1.0f;

	light_GS[0].diffuse_color[0] = 0.8f; light_GS[0].diffuse_color[1] = 0.8f;
	light_GS[0].diffuse_color[2] = 0.8f; light_GS[0].diffuse_color[3] = 1.0f;

	light_GS[0].specular_color[0] = 0.9f; light_GS[0].specular_color[1] = 0.9f;
	light_GS[0].specular_color[2] = 0.9f; light_GS[0].specular_color[3] = 1.0f;

	light_GS[0].spot_direction[0] = 0.0f; light_GS[0].spot_direction[1] = 0.0f; // spot light direction in WC
	light_GS[0].spot_direction[2] = -1.0f;
	if(EC_spot_flag) light_GS[0].spot_cutoff_angle = 20.0f;
	else light_GS[0].spot_cutoff_angle = 180.0f;
	light_GS[0].spot_exponent = 10.0f;
	
	// light 1: WC
	light_GS[1].light_on = light_WC_flag;
	light_GS[1].position[0] = -142.356186f; light_GS[1].position[1] = -71.802094f;
	light_GS[1].position[2] = 1051.533325f; light_GS[1].position[3] = 1.0f;

	light_GS[1].ambient_color[0] = 0.5f; light_GS[1].ambient_color[1] = 0.5f;
	light_GS[1].ambient_color[2] = 0.5f; light_GS[1].ambient_color[3] = 1.0f;

	light_GS[1].diffuse_color[0] = 0.7f; light_GS[1].diffuse_color[1] = 0.7f;
	light_GS[1].diffuse_color[2] = 0.7f; light_GS[1].diffuse_color[3] = 1.0f;

	light_GS[1].specular_color[0] = 0.9f; light_GS[1].specular_color[1] = 0.9f;
	light_GS[1].specular_color[2] = 0.9f; light_GS[1].specular_color[3] = 1.0f;

	light_GS[1].spot_direction[0] = -0.827871f; light_GS[1].spot_direction[1] = 0.454579f; // spot light direction in WC
	light_GS[1].spot_direction[2] = -0.328614f;
	light_GS[1].spot_cutoff_angle = 20.0f;
	light_GS[1].spot_exponent = 10.0f;

	// light 1: WC & tiger
	light_GS[2].light_on = light_WC_tiger_flag;
	light_GS[2].position[0] = t_pos.x; light_GS[2].position[1] = t_pos.y;
	light_GS[2].position[2] = t_pos.z + 100.0f; light_GS[2].position[3] = 1.0f;

	light_GS[2].ambient_color[0] = 0.4f; light_GS[2].ambient_color[1] = 0.4f;
	light_GS[2].ambient_color[2] = 0.4f; light_GS[2].ambient_color[3] = 1.0f;

	light_GS[2].diffuse_color[0] = 0.5f; light_GS[2].diffuse_color[1] = 0.5f;
	light_GS[2].diffuse_color[2] = 0.5f; light_GS[2].diffuse_color[3] = 1.0f;

	light_GS[2].specular_color[0] = 0.5f; light_GS[2].specular_color[1] = 0.5f;
	light_GS[2].specular_color[2] = 0.5f; light_GS[2].specular_color[3] = 1.0f;

	glUseProgram(h_ShaderProgram_GS);
	glUniform1i(loc_light_GS[0].light_on, light_GS[0].light_on);
	glUniform4fv(loc_light_GS[0].position, 1, light_GS[0].position);
	glUniform4fv(loc_light_GS[0].ambient_color, 1, light_GS[0].ambient_color);
	glUniform4fv(loc_light_GS[0].diffuse_color, 1, light_GS[0].diffuse_color);
	glUniform4fv(loc_light_GS[0].specular_color, 1, light_GS[0].specular_color);

	glUniform3fv(loc_light_GS[0].spot_direction, 1, light_GS[0].spot_direction);
	glUniform1f(loc_light_GS[0].spot_cutoff_angle, light_GS[0].spot_cutoff_angle);
	glUniform1f(loc_light_GS[0].spot_exponent, light_GS[0].spot_exponent);

	glm::vec4 position_EC_1 = ViewMatrix * glm::vec4(light_GS[1].position[0], light_GS[1].position[1],
		light_GS[1].position[2], light_GS[1].position[3]);

	glUniform1i(loc_light_GS[1].light_on, light_GS[1].light_on);
	glUniform4fv(loc_light_GS[1].position, 1, &position_EC_1[0]);
	glUniform4fv(loc_light_GS[1].ambient_color, 1, light_GS[1].ambient_color);
	glUniform4fv(loc_light_GS[1].diffuse_color, 1, light_GS[1].diffuse_color);
	glUniform4fv(loc_light_GS[1].specular_color, 1, light_GS[1].specular_color);

	glm::vec3 direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light_GS[1].spot_direction[0], light_GS[1].spot_direction[1],
		light_GS[1].spot_direction[2]);
	glUniform3fv(loc_light_GS[1].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light_GS[1].spot_cutoff_angle, light_GS[1].spot_cutoff_angle);
	glUniform1f(loc_light_GS[1].spot_exponent, light_GS[1].spot_exponent);

	glm::vec4 position_EC_2 = ViewMatrix * glm::vec4(light_GS[2].position[0], light_GS[2].position[1],
		light_GS[2].position[2], light_GS[2].position[3]);

	glUniform1i(loc_light_GS[2].light_on, light_GS[2].light_on);
	glUniform4fv(loc_light_GS[2].position, 1, &position_EC_2[0]);
	glUniform4fv(loc_light_GS[2].ambient_color, 1, light_GS[2].ambient_color);
	glUniform4fv(loc_light_GS[2].diffuse_color, 1, light_GS[2].diffuse_color);
	glUniform4fv(loc_light_GS[2].specular_color, 1, light_GS[2].specular_color);

	glUseProgram(0);

	memcpy(&light, light_GS, sizeof(Light_Parameters) * NUMBER_OF_LIGHT_SUPPORTED);
	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform4fv(loc_light[0].position, 1, light[0].position);
	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);

	glUniform3fv(loc_light[0].spot_direction, 1, light[0].spot_direction);
	glUniform1f(loc_light[0].spot_cutoff_angle, light[0].spot_cutoff_angle);
	glUniform1f(loc_light[0].spot_exponent, light[0].spot_exponent);

	glUniform1i(loc_light[1].light_on, light[1].light_on);
	glUniform4fv(loc_light[1].position, 1, &position_EC_1[0]);
	glUniform4fv(loc_light[1].ambient_color, 1, light[1].ambient_color);
	glUniform4fv(loc_light[1].diffuse_color, 1, light[1].diffuse_color);
	glUniform4fv(loc_light[1].specular_color, 1, light[1].specular_color);
	
	glUniform3fv(loc_light[1].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[1].spot_cutoff_angle, light[1].spot_cutoff_angle);
	glUniform1f(loc_light[1].spot_exponent, light[1].spot_exponent);

	glUniform1i(loc_light[2].light_on, light[2].light_on);
	glUniform4fv(loc_light[2].position, 1, &position_EC_2[0]);
	glUniform4fv(loc_light[2].ambient_color, 1, light[2].ambient_color);
	glUniform4fv(loc_light[2].diffuse_color, 1, light[2].diffuse_color);
	glUniform4fv(loc_light[2].specular_color, 1, light[2].specular_color);

	glUseProgram(0);
}

void initialize_flags(void) {
	flag_fog = 0;
	light_EC_flag = 0;
	light_WC_flag = 1;
	light_WC_tiger_flag = 0;
	cube_alpha = 1.0f;
	flag_light_test_GS = 0;
	EC_spot_flag = 0;
	view_normal_flag = 0;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_flag_fog, flag_fog);
	glUseProgram(0);
}

bool readTexImage2D_from_file(const char* filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap, * tx_pixmap_32;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	if (tx_pixmap == NULL)
		return false;
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else {
		fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
	}

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
		FreeImage_Unload(tx_pixmap);

	return true;
}

void prepare_bistro_exterior(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	// VBO, VAO malloc
	bistro_exterior_VBO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);
	bistro_exterior_VAO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);

	bistro_exterior_n_triangles = (int*)malloc(sizeof(int) * scene.n_materials);
	bistro_exterior_vertex_offset = (int*)malloc(sizeof(int) * scene.n_materials);

	flag_texture_mapping = (bool*)malloc(sizeof(bool) * scene.n_textures);

	// vertices
	bistro_exterior_vertices = (GLfloat**)malloc(sizeof(GLfloat*) * scene.n_materials);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		MATERIAL* pMaterial = &(scene.material_list[materialIdx]);
		GEOMETRY_TRIANGULAR_MESH* tm = &(pMaterial->geometry.tm);

		// vertex
		bistro_exterior_vertices[materialIdx] = (GLfloat*)malloc(sizeof(GLfloat) * 8 * tm->n_triangle * 3);

		int vertexIdx = 0;
		for (int triIdx = 0; triIdx < tm->n_triangle; triIdx++) {
			TRIANGLE tri = tm->triangle_list[triIdx];
			for (int triVertex = 0; triVertex < 3; triVertex++) {
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].x;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].y;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].z;

				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].x;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].y;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].z;

				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].u;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].v;
			}
		}

		// # of triangles
		bistro_exterior_n_triangles[materialIdx] = tm->n_triangle;

		if (materialIdx == 0)
			bistro_exterior_vertex_offset[materialIdx] = 0;
		else
			bistro_exterior_vertex_offset[materialIdx] = bistro_exterior_vertex_offset[materialIdx - 1] + 3 * bistro_exterior_n_triangles[materialIdx - 1];

		glGenBuffers(1, &bistro_exterior_VBO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, bistro_exterior_VBO[materialIdx]);
		glBufferData(GL_ARRAY_BUFFER, bistro_exterior_n_triangles[materialIdx] * 3 * n_bytes_per_vertex,
			bistro_exterior_vertices[materialIdx], GL_STATIC_DRAW);

		free(bistro_exterior_vertices[materialIdx]);

		glGenVertexArrays(1, &bistro_exterior_VAO[materialIdx]);
		glBindVertexArray(bistro_exterior_VAO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, bistro_exterior_VBO[materialIdx]);
		glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
		glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
		glVertexAttribPointer(INDEX_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_NORMAL);
		glVertexAttribPointer(INDEX_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_TEX_COORD);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		if ((materialIdx > 0) && (materialIdx % 100 == 0))
			fprintf(stdout, " * Loaded %d bistro exterior materials into graphics memory.\n", materialIdx / 100 * 100);
	}
	fprintf(stdout, " * Loaded %d bistro exterior materials into graphics memory.\n", scene.n_materials);

	// textures
	bistro_exterior_texture_names = (GLuint*)malloc(sizeof(GLuint) * (scene.n_textures + 2));
	glGenTextures(scene.n_textures + 2, bistro_exterior_texture_names);

	for (int texId = 0; texId < scene.n_textures; texId++) {
		glActiveTexture(GL_TEXTURE0 + texId);
		glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[texId]);

		bool bReturn = readTexImage2D_from_file(scene.texture_file_name[texId]);

		if (bReturn) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			flag_texture_mapping[texId] = true;
		}
		else {
			flag_texture_mapping[texId] = false;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		
	}

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	glActiveTexture(GL_TEXTURE0 + scene.n_textures);
	glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[scene.n_textures]);
	bool read_Tex = readTexImage2D_from_file("Data/stone.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	glActiveTexture(GL_TEXTURE0 + scene.n_textures + 1);
	glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[scene.n_textures + 1]);
	read_Tex = readTexImage2D_from_file("Data/lava.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	fprintf(stdout, " * Loaded bistro exterior textures into graphics memory.\n\n");

	free(bistro_exterior_vertices);
}

void draw_bistro_exterior(void) {
	glUseProgram(h_ShaderProgram_TXPS);
	ModelViewMatrix = ViewMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		glUniform4fv(loc_material.ambient_color, 1, scene.material_list[materialIdx].shading.ph.ka);
		glUniform4fv(loc_material.diffuse_color, 1, scene.material_list[materialIdx].shading.ph.kd);
		glUniform4fv(loc_material.specular_color, 1, scene.material_list[materialIdx].shading.ph.ks);
		glUniform1f(loc_material.specular_exponent, scene.material_list[materialIdx].shading.ph.spec_exp);
		glUniform4fv(loc_material.emissive_color, 1, scene.material_list[materialIdx].shading.ph.kr);

		int texId = scene.material_list[materialIdx].diffuseTexId;
		glUniform1i(loc_texture, texId);
		glUniform1i(loc_flag_texture_mapping, flag_texture_mapping[texId]);

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0 + texId);
		glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[texId]);

		glBindVertexArray(bistro_exterior_VAO[materialIdx]);
		glDrawArrays(GL_TRIANGLES, 0, 3 * bistro_exterior_n_triangles[materialIdx]);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
}

#define N_TIGER_FRAMES 12
#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat* tiger_vertices[N_TIGER_FRAMES];
int cur_frame_tiger = 0;

int read_geometry(GLfloat** object, int bytes_per_primitive, char* filename) {
	int n_triangles;
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	*object = (float*)malloc(n_triangles * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

void my_prepare_tiger(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry(&tiger_vertices[i], n_bytes_per_triangle, filename);
		// Assume all geometry files are effective.
		tiger_n_total_triangles += tiger_n_triangles[i];

		if (i == 0)
			tiger_vertex_offset[i] = 0;
		else
			tiger_vertex_offset[i] = tiger_vertex_offset[i - 1] + 3 * tiger_n_triangles[i - 1];
	}

	// Initialize vertex buffer object.
	glGenBuffers(1, &tiger_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
			tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);

	// As the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; i++)
		free(tiger_vertices[i]);

	// Initialize vertex array object.
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

GLfloat tiger_route[10][3] = {
	{ 3264.227783f, -2080.660400f, 20.040878f }, { 1100.184143f, -1200.893677f, 20.040878f },
	{ 650.184143f, -1100.893677f, 20.040878f }, { -617.070374f, -433.350311f, 47.251564f },
	{-822.511597f, 222.470490f, 44.453773f }, { -425.771667f, 841.519470f, 20.040878f },
	{ -85.842186f, 1200.930176f, 20.040878f }, { 145.480377f, 1455.413452f, 20.040878f },
	{ 1481.764771f, 3716.732910f, 20.040878f }, { 1073.459351f, 4154.527344f, 20.040878f }
};

#include<math.h>
int t_flag = 0;
float t_rot;
int t_camera = 0;
int g_camera = 0;
int tiger_rot = 0;
int t_pause = 1;


void my_tiger_pos() {
	// check current route
	float Ins = (float)(t_pos.x - tiger_route[t_flag + 1][0]) / (tiger_route[t_flag][0] - tiger_route[t_flag + 1][0]);

	// move next route
	if (Ins < 0) {
		if (t_flag == 8) t_flag = 0;
		else ++t_flag;
		t_pos.x = tiger_route[t_flag][0];
		t_pos.y = tiger_route[t_flag][1];
		t_pos.z = tiger_route[t_flag][2];
	}

	float dir_x = tiger_route[t_flag + 1][0] - tiger_route[t_flag][0];
	float dir_y = tiger_route[t_flag + 1][1] - tiger_route[t_flag][1];
	glm::vec2 n = normalize(glm::vec2(dir_x, dir_y));

	t_rot = acosf(-n.y);
	if (n.x > 0) t_rot *= -1.0f;

	// JUMP
	if (t_flag == 1 || t_flag == 6) {
		if (Ins > 0.5) t_pos.z = 20.040878f + 100 * sinf(acosf((float)(Ins - 0.5) / 0.5f));
		else t_pos.z = 20.040878f + 100 * sinf(acosf((float)(0.5 - Ins) / 0.5f));
		t_pos.x += n.x * 30;
		t_pos.y += n.y * 30;
	}

	// STRAIGNT
	else {
		t_pos.x += n.x * 30;
		t_pos.y += n.y * 30;
		t_pos.z = 20.040878f;
	}

	// Camera Mode
	if (t_camera || g_camera) {
		glm::vec3 vec3_tmp;
		current_camera.naxis[0] = -n.x;
		current_camera.naxis[1] = -n.y;
		current_camera.naxis[2] = 0.0f;

		vec3_tmp = glm::cross(glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]),
			glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]));

		current_camera.uaxis[0] = vec3_tmp.x; current_camera.uaxis[1] = vec3_tmp.y; current_camera.uaxis[2] = vec3_tmp.z;

		if (t_camera) {
			glm::vec3 vec3_n(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
			glm::vec3 vec3_v(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
			glm::mat4 mat4_tmp = glm::rotate(glm::mat4(1.0f), sinf(tiger_rot) * TO_RADIAN, vec3_tmp);
			vec3_n = glm::vec3(mat4_tmp * glm::vec4(vec3_n, 0.0f));
			vec3_v = glm::vec3(mat4_tmp * glm::vec4(vec3_v, 0.0f));

			current_camera.naxis[0] = vec3_n.x; current_camera.naxis[1] = vec3_n.y; current_camera.naxis[2] = vec3_n.z;
			current_camera.vaxis[0] = vec3_v.x; current_camera.vaxis[1] = vec3_v.y; current_camera.vaxis[2] = vec3_v.z;

			current_camera.pos[0] = t_pos.x + 2 * 88.0f * n.x;
			current_camera.pos[1] = t_pos.y + 2 * 88.0f * n.y;
			current_camera.pos[2] = t_pos.z + 2 * 62.0f;
		}

		else {
			current_camera.pos[0] = t_pos.x - 2 * 150.0f * n.x;
			current_camera.pos[1] = t_pos.y - 2 * 150.0f * n.y;
			current_camera.pos[2] = t_pos.z + 2 * 100.0f;
		}
	}
}

void my_draw_tiger(void) {
	glUseProgram(h_ShaderProgram_simple);

	ModelViewMatrix = glm::translate(ViewMatrix, t_pos);
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, t_rot, glm::vec3(0.0f, 0.0f, -1.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glFrontFace(GL_CCW);
	glBindVertexArray(tiger_VAO);
	glUniform3f(loc_primitive_color, 1.0f, 1.0f, 0.0f);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

// dragon object
GLuint dragon_VBO, dragon_VAO;
int dragon_n_triangles;
GLfloat* dragon_vertices;
int dragon_rot = 0;

void my_prepare_dragon(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, dragon_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float);
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/dynamic/dragon_vnt.geom");
	dragon_n_triangles = read_geometry(&dragon_vertices, n_bytes_per_triangle, filename);
	dragon_n_total_triangles += dragon_n_triangles;

	glGenBuffers(1, &dragon_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glBufferData(GL_ARRAY_BUFFER, dragon_n_total_triangles * 3 * n_bytes_per_vertex, dragon_vertices, GL_STATIC_DRAW);

	free(dragon_vertices);

	glGenVertexArrays(1, &dragon_VAO);
	glBindVertexArray(dragon_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

float d_height = 500.0f;
void my_draw_dragon(void) {
	glUseProgram(h_ShaderProgram_simple);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(4470.700684f, -2466.115723f, d_height));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, dragon_rot * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(1150.0f + 200 * sinf(dragon_rot * TO_RADIAN * 2), 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(10.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glFrontFace(GL_CCW);
	glBindVertexArray(dragon_VAO);
	glUniform3f(loc_primitive_color, 1.0f, 0.0f, 0.0f);
	glDrawArrays(GL_TRIANGLES, 0, 3 * dragon_n_triangles);
	glBindVertexArray(0);
	glUseProgram(0);
}

// bike object
GLuint bike_VBO, bike_VAO;
int bike_n_triangles;
GLfloat* bike_vertices;

void my_prepare_bike(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, bike_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float);
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static/bike_vnt.geom");
	bike_n_triangles = read_geometry(&bike_vertices, n_bytes_per_triangle, filename);
	bike_n_total_triangles += bike_n_triangles;

	glGenBuffers(1, &bike_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, bike_VBO);
	glBufferData(GL_ARRAY_BUFFER, bike_n_total_triangles * 3 * n_bytes_per_vertex, bike_vertices, GL_STATIC_DRAW);

	free(bike_vertices);

	glGenVertexArrays(1, &bike_VAO);
	glBindVertexArray(bike_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, bike_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void my_draw_bike(void) {
	glUseProgram(h_ShaderProgram_simple);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(3394.154297f, -2466.190674f, 20.040878f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 14 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glFrontFace(GL_CCW);
	glUniform3f(loc_primitive_color, 1.0f, 0.0f, 0.0f);
	glBindVertexArray(bike_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * bike_n_triangles);
	glBindVertexArray(0);
	glUseProgram(0);
}

void set_material(Material_Parameters material, int mode) {
	if (mode == 1) {
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform4fv(loc_material.ambient_color, 1, material.ambient_color);
		glUniform4fv(loc_material.diffuse_color, 1, material.diffuse_color);
		glUniform4fv(loc_material.specular_color, 1, material.specular_color);
		glUniform1f(loc_material.specular_exponent, material.specular_exponent);
		glUniform4fv(loc_material.emissive_color, 1, material.emissive_color);
	}

	else if (mode == 2) {
		glUseProgram(h_ShaderProgram_GS);
		glUniform4fv(loc_material_GS.ambient_color, 1, material.ambient_color);
		glUniform4fv(loc_material_GS.diffuse_color, 1, material.diffuse_color);
		glUniform4fv(loc_material_GS.specular_color, 1, material.specular_color);
		glUniform1f(loc_material_GS.specular_exponent, material.specular_exponent);
		glUniform4fv(loc_material_GS.emissive_color, 1, material.emissive_color);
	}

	glUseProgram(0);
}

// godzilla object
GLuint godzilla_VBO, godzilla_VAO;
int godzilla_n_triangles;
GLfloat* godzilla_vertices;
Material_Parameters material_godzilla;

void my_prepare_godzilla(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, godzilla_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float);
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static/godzilla_vnt.geom");
	godzilla_n_triangles = read_geometry(&godzilla_vertices, n_bytes_per_triangle, filename);
	godzilla_n_total_triangles += godzilla_n_triangles;

	glGenBuffers(1, &godzilla_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, godzilla_VBO);
	glBufferData(GL_ARRAY_BUFFER, godzilla_n_total_triangles * 3 * n_bytes_per_vertex, godzilla_vertices, GL_STATIC_DRAW);

	free(godzilla_vertices);

	glGenVertexArrays(1, &godzilla_VAO);
	glBindVertexArray(godzilla_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, godzilla_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_godzilla.ambient_color[0] = 0.24725f;
	material_godzilla.ambient_color[1] = 0.1995f;
	material_godzilla.ambient_color[2] = 0.0745f;
	material_godzilla.ambient_color[3] = 1.0f;

	material_godzilla.diffuse_color[0] = 0.0f;
	material_godzilla.diffuse_color[1] = 0.0f;
	material_godzilla.diffuse_color[2] = 0.0f;
	material_godzilla.diffuse_color[3] = 1.0f;

	material_godzilla.specular_color[0] = 0.728281f;
	material_godzilla.specular_color[1] = 0.655802f;
	material_godzilla.specular_color[2] = 0.466065f;
	material_godzilla.specular_color[3] = 1.0f;

	material_godzilla.specular_exponent = 10.0f;

	material_godzilla.emissive_color[0] = 0.1f;
	material_godzilla.emissive_color[1] = 0.1f;
	material_godzilla.emissive_color[2] = 0.0f;
	material_godzilla.emissive_color[3] = 1.0f;
}

void my_draw_godzilla(void) {
	set_material(material_godzilla, 1);

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_texture, scene.n_textures + 1);
	glUniform1i(loc_flag_texture_mapping, 0);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + scene.n_textures + 1);
	glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[scene.n_textures + 1]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-1722.175415f, 891.344543f, 20.040878f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 28 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(4.0f, 4.0f, 4.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glFrontFace(GL_CCW);
	glBindVertexArray(godzilla_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * godzilla_n_triangles);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

// spider object
#define N_SPIDER_FRAMES 16
GLuint spider_VBO, spider_VAO;
int spider_n_triangles[N_SPIDER_FRAMES];
int spider_vertex_offset[N_SPIDER_FRAMES];
GLfloat* spider_vertices[N_SPIDER_FRAMES];
int cur_frame_spider = 0;
Material_Parameters material_spider;

void my_prepare_spider(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, spider_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float);
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_SPIDER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic/spider_vnt_%d%d.geom", i / 10, i % 10);
		spider_n_triangles[i] = read_geometry(&spider_vertices[i], n_bytes_per_triangle, filename);
		spider_n_total_triangles += spider_n_triangles[i];

		if (i == 0)
			spider_vertex_offset[i] = 0;
		else
			spider_vertex_offset[i] = spider_vertex_offset[i - 1] + 3 * spider_n_triangles[i - 1];
	}

	glGenBuffers(1, &spider_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glBufferData(GL_ARRAY_BUFFER, spider_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_SPIDER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, spider_vertex_offset[i] * n_bytes_per_vertex,
			spider_n_triangles[i] * n_bytes_per_triangle, spider_vertices[i]);

	for (i = 0; i < N_SPIDER_FRAMES; i++)
		free(spider_vertices[i]);

	glGenVertexArrays(1, &spider_VAO);
	glBindVertexArray(spider_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_spider.ambient_color[0] = 0.24725f;
	material_spider.ambient_color[1] = 0.1995f;
	material_spider.ambient_color[2] = 0.0745f;
	material_spider.ambient_color[3] = 1.0f;

	material_spider.diffuse_color[0] = 0.0f;
	material_spider.diffuse_color[1] = 0.0f;
	material_spider.diffuse_color[2] = 0.0f;
	material_spider.diffuse_color[3] = 1.0f;

	material_spider.specular_color[0] = 0.728281f;
	material_spider.specular_color[1] = 0.655802f;
	material_spider.specular_color[2] = 0.466065f;
	material_spider.specular_color[3] = 1.0f;

	material_spider.specular_exponent = 10.0f;

	material_spider.emissive_color[0] = 0.1f;
	material_spider.emissive_color[1] = 0.1f;
	material_spider.emissive_color[2] = 0.0f;
	material_spider.emissive_color[3] = 1.0f;
}

int spider_rot = 0;
void my_draw_spider(void) {
	set_material(material_spider, 1);

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_texture, scene.n_textures + 1);
	glUniform1i(loc_flag_texture_mapping, 1);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + scene.n_textures + 1);
	glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[scene.n_textures + 1]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-1876.430908f, 546.649414f, 883.476807f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 130 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, spider_rot * TO_RADIAN, glm::vec3(0.0f, -1.0f, 0.0f));
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(600.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, (40 * sinf(spider_rot * 0.3f)) * TO_RADIAN, glm::vec3(0.0f, -1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glFrontFace(GL_CCW);
	glBindVertexArray(spider_VAO);
	glDrawArrays(GL_TRIANGLES, spider_vertex_offset[cur_frame_spider], 3 * spider_n_triangles[cur_frame_spider]);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

// ironman object
GLuint ironman_VBO, ironman_VAO;
int ironman_n_triangles;
GLfloat* ironman_vertices;
Material_Parameters material_ironman;

void my_prepare_ironman(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, ironman_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float);
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static/ironman_vnt.geom");
	ironman_n_triangles = read_geometry(&ironman_vertices, n_bytes_per_triangle, filename);
	ironman_n_total_triangles += ironman_n_triangles;

	glGenBuffers(1, &ironman_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glBufferData(GL_ARRAY_BUFFER, ironman_n_total_triangles * 3 * n_bytes_per_vertex, ironman_vertices, GL_STATIC_DRAW);

	free(ironman_vertices);

	glGenVertexArrays(1, &ironman_VAO);
	glBindVertexArray(ironman_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_ironman.ambient_color[0] = 0.24725f;
	material_ironman.ambient_color[1] = 0.1995f;
	material_ironman.ambient_color[2] = 0.0745f;
	material_ironman.ambient_color[3] = 1.0f;

	material_ironman.diffuse_color[0] = 0.0f;
	material_ironman.diffuse_color[1] = 0.0f;
	material_ironman.diffuse_color[2] = 0.0f;
	material_ironman.diffuse_color[3] = 1.0f;

	material_ironman.specular_color[0] = 0.728281f;
	material_ironman.specular_color[1] = 0.655802f;
	material_ironman.specular_color[2] = 0.466065f;
	material_ironman.specular_color[3] = 1.0f;

	material_ironman.specular_exponent = 10.0f;

	material_ironman.emissive_color[0] = 0.1f;
	material_ironman.emissive_color[1] = 0.1f;
	material_ironman.emissive_color[2] = 0.0f;
	material_ironman.emissive_color[3] = 1.0f;
}

void my_draw_ironman(void) {
	set_material(material_ironman, 2);

	glUseProgram(h_ShaderProgram_GS);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(2654.549805f, -1375.125000f, 20.040878f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -20 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(80.0f, 80.0f, 80.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glFrontFace(GL_CCW);
	glBindVertexArray(ironman_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ironman_n_triangles);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

// tank object
GLuint tank_VBO, tank_VAO;
int tank_n_triangles;
GLfloat* tank_vertices;

void my_prepare_tank(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tank_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float);
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static/tank_vnt.geom");
	tank_n_triangles = read_geometry(&tank_vertices, n_bytes_per_triangle, filename);
	tank_n_total_triangles += tank_n_triangles;

	glGenBuffers(1, &tank_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glBufferData(GL_ARRAY_BUFFER, tank_n_total_triangles * 3 * n_bytes_per_vertex, tank_vertices, GL_STATIC_DRAW);

	free(tank_vertices);

	glGenVertexArrays(1, &tank_VAO);
	glBindVertexArray(tank_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void my_draw_tank(void) {
	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_texture, scene.n_textures);
	glUniform1i(loc_flag_texture_mapping, 1);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + scene.n_textures);
	glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[scene.n_textures]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(3850.695557f, -1302.840332f, 20.040878f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(80.0f, 80.0f, 80.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glFrontFace(GL_CCW);
	glBindVertexArray(tank_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * tank_n_triangles);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

// optimus object
GLuint optimus_VBO, optimus_VAO;
int optimus_n_triangles;
GLfloat* optimus_vertices;
Material_Parameters material_optimus;

void my_prepare_optimus(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, optimus_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float);
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static/optimus_vnt.geom");
	optimus_n_triangles = read_geometry(&optimus_vertices, n_bytes_per_triangle, filename);
	optimus_n_total_triangles += optimus_n_triangles;

	glGenBuffers(1, &optimus_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, optimus_VBO);
	glBufferData(GL_ARRAY_BUFFER, optimus_n_total_triangles * 3 * n_bytes_per_vertex, optimus_vertices, GL_STATIC_DRAW);

	free(optimus_vertices);

	glGenVertexArrays(1, &optimus_VAO);
	glBindVertexArray(optimus_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, optimus_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_optimus.ambient_color[0] = 0.24725f;
	material_optimus.ambient_color[1] = 0.1995f;
	material_optimus.ambient_color[2] = 0.0745f;
	material_optimus.ambient_color[3] = 1.0f;

	material_optimus.diffuse_color[0] = 0.0f;
	material_optimus.diffuse_color[1] = 0.0f;
	material_optimus.diffuse_color[2] = 0.0f;
	material_optimus.diffuse_color[3] = 1.0f;

	material_optimus.specular_color[0] = 0.728281f;
	material_optimus.specular_color[1] = 0.655802f;
	material_optimus.specular_color[2] = 0.466065f;
	material_optimus.specular_color[3] = 1.0f;

	material_optimus.specular_exponent = 10.0f;

	material_optimus.emissive_color[0] = 0.1f;
	material_optimus.emissive_color[1] = 0.1f;
	material_optimus.emissive_color[2] = 0.0f;
	material_optimus.emissive_color[3] = 1.0f;
}

void my_draw_optimus(void) {
	set_material(material_optimus, 2);

	glUseProgram(h_ShaderProgram_GS);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(1706.633667f, 4223.221680f, 20.040878f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -140 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glFrontFace(GL_CCW);
	glBindVertexArray(optimus_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * optimus_n_triangles);
	glBindVertexArray(0);

	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

// cube object
GLuint cube_VBO, cube_VAO;
GLfloat cube_vertices[36][8] = { // vertices enumerated counterclockwise
	{ -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f }, 
	{ -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f },
	{ -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f },
	{ -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f },
	{ -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f },
	{ -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f },
	{ -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f },
	{ -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
	{ -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f }
};
// 한 면에 6개 점 +  6개 법선

Material_Parameters material_cube;
float rotation_angle_cube = 0.0f;
int blending_mode = 0;

void prepare_cube(void) {
	glGenBuffers(1, &cube_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), &cube_vertices[0][0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &cube_VAO);
	glBindVertexArray(cube_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_cube.ambient_color[0] = 0.1745f;
	material_cube.ambient_color[1] = 0.01175f;
	material_cube.ambient_color[2] = 0.01175f;
	material_cube.ambient_color[3] = 1.0f;

	material_cube.diffuse_color[0] = 0.61424f;
	material_cube.diffuse_color[1] = 0.04136f;
	material_cube.diffuse_color[2] = 0.04136f;
	material_cube.diffuse_color[3] = 1.0f;

	material_cube.specular_color[0] = 0.727811f;
	material_cube.specular_color[1] = 0.626959f;
	material_cube.specular_color[2] = 0.626959f;
	material_cube.specular_color[3] = 1.0f;

	material_cube.specular_exponent = 20.0f;

	material_cube.emissive_color[0] = 0.0f;
	material_cube.emissive_color[1] = 0.0f;
	material_cube.emissive_color[2] = 0.0f;
	material_cube.emissive_color[3] = 1.0f;

	cube_alpha = 0.5f;
}

void draw_cube(void) {
	set_material(material_cube, 1);
	glFrontFace(GL_CCW);

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1f(loc_u_fragment_alpha, cube_alpha);
	glUniform1i(loc_u_draw_cube, 1);
	glUniform1i(loc_texture, scene.n_textures);
	glUniform1i(loc_flag_texture_mapping, 0);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + scene.n_textures);
	glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[scene.n_textures]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-610.618164f, 631.597229f, 732.029785f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, rotation_angle_cube, glm::vec3(1.0f, 1.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(70.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glCullFace(GL_FRONT);
	glBindVertexArray(cube_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
	glBindVertexArray(0);

	glCullFace(GL_BACK);
	glBindVertexArray(cube_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
	glBindVertexArray(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glUniform1i(loc_u_draw_cube, 0);
	glUseProgram(0);
}
 
void timer_scene(int timestamp_scene) {
	rotation_angle_cube = (timestamp_scene % 360) * TO_RADIAN;
	dragon_rot += 6;
	spider_rot += 2;
	tiger_rot += 1;
	d_height += 100 * sin(timestamp_scene / 2);

	if (t_pause) {
		my_tiger_pos();
		set_ViewMatrix_from_camera_frame();
		cur_frame_tiger = timestamp_scene % N_TIGER_FRAMES;
	}

	cur_frame_spider = timestamp_scene % N_SPIDER_FRAMES;
	glutPostRedisplay();
	glutTimerFunc(100, timer_scene, (timestamp_scene + 1) % INT_MAX);
}

void my_prepare_objects(void) {
	my_prepare_tiger();
	my_prepare_dragon();
	my_prepare_bike();
	my_prepare_godzilla();
	my_prepare_spider();
	my_prepare_ironman();
	my_prepare_tank();
	my_prepare_optimus();
	prepare_cube();
}

void my_draw_objects(void) {
	glLineWidth(2.0f);

	glDisable(GL_DEPTH_TEST);
	glUseProgram(h_ShaderProgram_simple);
	glUniform1i(loc_draw_silhouette, true);
	glUseProgram(0);
	my_draw_tiger();

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(h_ShaderProgram_simple);
	glUniform1i(loc_draw_silhouette, false);
	glUseProgram(0);

	my_draw_tiger();
	my_draw_dragon();
	my_draw_bike();
	my_draw_godzilla();
	my_draw_spider();
	my_draw_ironman();
	my_draw_tank();
	my_draw_optimus();
	draw_cube();

	glLineWidth(1.0f);
}
/*****************************  END: geometry setup *****************************/

/********************  START: callback function definitions *********************/
void display(void) {
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	draw_grid();
	draw_axes();
	draw_bistro_exterior();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_CULL_FACE);
	my_draw_objects();
	glDisable(GL_CULL_FACE);

	glutSwapBuffers();
}

int prevx, prevy;
int view_flag = 0;
void my_mousepress(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) {
		prevx = x; prevy = y;
	}
}

void my_mousemove(int x, int y) {
	if (view_flag) {
		glm::vec3 n, u, v;
		glm::mat4 mat4_tmp;
		glm::vec3 vec3_tmp;
		float delx, dely;

		delx = (float)(x - prevx), dely = -(float)(y - prevy);
		prevx = x, prevy = y;

		n.x = current_camera.naxis[0]; n.y = current_camera.naxis[1]; n.z = current_camera.naxis[2];
		u.x = current_camera.uaxis[0]; u.y = current_camera.uaxis[1]; u.z = current_camera.uaxis[2];
		v.x = current_camera.vaxis[0]; v.y = current_camera.vaxis[1]; v.z = current_camera.vaxis[2];

		// rotation by delx
		mat4_tmp = glm::rotate(glm::mat4(1.0f), delx * TO_RADIAN * 0.1f, v);
		n = glm::vec3(mat4_tmp * glm::vec4(n, 0.0f));
		u = glm::vec3(mat4_tmp * glm::vec4(u, 0.0f));

		// rotation by dely
		mat4_tmp = glm::rotate(glm::mat4(1.0f), dely * TO_RADIAN * 0.1f, -u);
		n = glm::vec3(mat4_tmp * glm::vec4(n, 0.0f));
		v = glm::vec3(mat4_tmp * glm::vec4(v, 0.0f));

		// n, u, v Update
		current_camera.naxis[0] = n.x; current_camera.naxis[1] = n.y; current_camera.naxis[2] = n.z;
		current_camera.uaxis[0] = u.x; current_camera.uaxis[1] = u.y; current_camera.uaxis[2] = u.z;
		current_camera.vaxis[0] = v.x; current_camera.vaxis[1] = v.y; current_camera.vaxis[2] = v.z;

		set_ViewMatrix_from_camera_frame();
		glutPostRedisplay();
	}
}

void my_mousewheel(int button, int dir, int x, int y) {
	if (glutGetModifiers() & GLUT_ACTIVE_SHIFT && !g_camera && !t_camera) {
		// zoom in
		if (dir > 0) {
			current_camera.pos[0] -= 15 * current_camera.naxis[0];
			current_camera.pos[1] -= 15 * current_camera.naxis[1];
			current_camera.pos[2] -= 15 * current_camera.naxis[2];
		}
		// zoom out
		else {
			current_camera.pos[0] += 15 * current_camera.naxis[0];
			current_camera.pos[1] += 15 * current_camera.naxis[1];
			current_camera.pos[2] += 15 * current_camera.naxis[2];
		}
		set_ViewMatrix_from_camera_frame();
		glutPostRedisplay();
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'f':
		b_draw_grid = b_draw_grid ? false : true;
		glutPostRedisplay();
		break;

	case 'm':
		if (t_camera) t_camera = 1 - t_camera;
		if (g_camera) g_camera = 1 - g_camera;
		view_flag = 1 - view_flag;
		break;

	case 'w':
		if (view_flag) {
			current_camera.pos[0] -= 30 * current_camera.naxis[0];
			current_camera.pos[1] -= 30 * current_camera.naxis[1];
			current_camera.pos[2] -= 30 * current_camera.naxis[2];
			set_ViewMatrix_from_camera_frame();
			glutPostRedisplay();
		}
		break;
	case 'a':
		if (view_flag) {
			current_camera.pos[0] -= 30 * current_camera.uaxis[0];
			current_camera.pos[1] -= 30 * current_camera.uaxis[1];
			current_camera.pos[2] -= 30 * current_camera.uaxis[2];
			set_ViewMatrix_from_camera_frame();
			glutPostRedisplay();
		}
		break;
	case 's':
		if (view_flag) {
			current_camera.pos[0] += 30 * current_camera.naxis[0];
			current_camera.pos[1] += 30 * current_camera.naxis[1];
			current_camera.pos[2] += 30 * current_camera.naxis[2];
			set_ViewMatrix_from_camera_frame();
			glutPostRedisplay();
		}
		break;
	case 'd':
		if (view_flag) {
			current_camera.pos[0] += 30 * current_camera.uaxis[0];
			current_camera.pos[1] += 30 * current_camera.uaxis[1];
			current_camera.pos[2] += 30 * current_camera.uaxis[2];
			set_ViewMatrix_from_camera_frame();
			glutPostRedisplay();
		}
		break;
	case 't':
		if (view_flag) view_flag = 1 - view_flag;
		if (g_camera) g_camera = 1 - g_camera;
		t_camera = 1 - t_camera;
		if (t_camera) {
			current_camera.vaxis[0] = 0.0f;
			current_camera.vaxis[1] = 0.0f;
			current_camera.vaxis[2] = 1.0f;
		}
		break;
	case 'g':
		if (view_flag) view_flag = 1 - view_flag;
		if (t_camera) t_camera = 1 - t_camera;
		g_camera = 1 - g_camera;
		if (g_camera) {
			current_camera.vaxis[0] = 0.0f;
			current_camera.vaxis[1] = 0.0f;
			current_camera.vaxis[2] = 1.0f;
		}
		break;
	case 'e':
		if (view_flag) {
			glm::vec3 n, u, v;
			glm::mat4 rot;
			n.x = current_camera.naxis[0]; n.y = current_camera.naxis[1]; n.z = current_camera.naxis[2];
			u.x = current_camera.uaxis[0]; u.y = current_camera.uaxis[1]; u.z = current_camera.uaxis[2];
			v.x = current_camera.vaxis[0]; v.y = current_camera.vaxis[1]; v.z = current_camera.vaxis[2];
			rot = glm::rotate(mat4(1.0f), 2 * TO_RADIAN, n);
			v = glm::vec3(rot * glm::vec4(v, 0.0f));
			u = glm::vec3(rot * glm::vec4(u, 0.0f));
			current_camera.uaxis[0] = u.x; current_camera.uaxis[1] = u.y; current_camera.uaxis[2] = u.z;
			current_camera.vaxis[0] = v.x; current_camera.vaxis[1] = v.y; current_camera.vaxis[2] = v.z;
			set_ViewMatrix_from_camera_frame();
			glutPostRedisplay();
		}
		break;

	case 'q':
		if (view_flag) {
			glm::vec3 n, u, v;
			glm::mat4 rot;
			n.x = current_camera.naxis[0]; n.y = current_camera.naxis[1]; n.z = current_camera.naxis[2];
			u.x = current_camera.uaxis[0]; u.y = current_camera.uaxis[1]; u.z = current_camera.uaxis[2];
			v.x = current_camera.vaxis[0]; v.y = current_camera.vaxis[1]; v.z = current_camera.vaxis[2];
			rot = glm::rotate(mat4(1.0f), 2 * TO_RADIAN, -n);
			v = glm::vec3(rot * glm::vec4(v, 0.0f));
			u = glm::vec3(rot * glm::vec4(u, 0.0f));
			current_camera.uaxis[0] = u.x; current_camera.uaxis[1] = u.y; current_camera.uaxis[2] = u.z;
			current_camera.vaxis[0] = v.x; current_camera.vaxis[1] = v.y; current_camera.vaxis[2] = v.z;
			set_ViewMatrix_from_camera_frame();
			glutPostRedisplay();
		}
		break;
	case 'z':
		if (view_flag) {
			current_camera.pos[0] += 30 * current_camera.vaxis[0];
			current_camera.pos[1] += 30 * current_camera.vaxis[1];
			current_camera.pos[2] += 30 * current_camera.vaxis[2];
			set_ViewMatrix_from_camera_frame();
			glutPostRedisplay();
		}
		break;
	case 'x':
		if (view_flag) {
			current_camera.pos[0] -= 30 * current_camera.vaxis[0];
			current_camera.pos[1] -= 30 * current_camera.vaxis[1];
			current_camera.pos[2] -= 30 * current_camera.vaxis[2];
			set_ViewMatrix_from_camera_frame();
			glutPostRedisplay();
		}
		break;
	case 'p':
		t_pause = 1 - t_pause;
		if (t_camera) t_camera = 1 - t_camera;
		if (g_camera) g_camera = 1 - g_camera;
		view_flag = 1;
		break;
	case 'l':
		flag_light_test_GS = 1 - flag_light_test_GS;
		glUseProgram(h_ShaderProgram_GS);
		glUniform1i(loc_light_Test_GS, flag_light_test_GS);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case '0':
		if (g_camera) g_camera = 1 - g_camera;
		if (t_camera) t_camera = 1 - t_camera;
		set_current_camera(CAMERA_1);
		glutPostRedisplay();
		break;
	case 'n':
		light_EC_flag = 1 - light_EC_flag;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_light[0].light_on, light_EC_flag);
		glUseProgram(0);
		glUseProgram(h_ShaderProgram_GS);
		glUniform1i(loc_light_GS[0].light_on, light_EC_flag);
		glUseProgram(0);
		set_ViewMatrix_from_camera_frame();
		glutPostRedisplay();
		break;
	case 'b':
		light_WC_flag = 1 - light_WC_flag;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_light[1].light_on, light_WC_flag);
		glUseProgram(0);
		glUseProgram(h_ShaderProgram_GS);
		glUniform1i(loc_light_GS[1].light_on, light_WC_flag);
		glUseProgram(0);
		set_ViewMatrix_from_camera_frame();
		glutPostRedisplay();
		break;
	case 'v':
		light_WC_tiger_flag = 1 - light_WC_tiger_flag;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_light[2].light_on, light_WC_tiger_flag);
		glUseProgram(0);
		glUseProgram(h_ShaderProgram_GS);
		glUniform1i(loc_light_GS[2].light_on, light_WC_tiger_flag);
		glUseProgram(0);
		set_ViewMatrix_from_camera_frame();
		glutPostRedisplay();
		break;
	case 'h':
		if (blending_mode) cube_alpha += alpha_offset;
		if (cube_alpha > 1.0f) cube_alpha = 1.0f;
		glutPostRedisplay();
		break;
	case 'j':
		if(blending_mode) cube_alpha -= alpha_offset;
		if (cube_alpha < 0.0f) cube_alpha = 0.0f;
		glutPostRedisplay();
		break;
	case 'k':
		blending_mode = 1 - blending_mode;
		break;
	case '1':
		if (g_camera) g_camera = 1 - g_camera;
		if (t_camera) t_camera = 1 - t_camera;
		set_current_camera(CAMERA_2);
		glutPostRedisplay();
		break;
	case '2':
		if (g_camera) g_camera = 1 - g_camera;
		if (t_camera) t_camera = 1 - t_camera;
		set_current_camera(CAMERA_3);
		glutPostRedisplay();
		break;
	case '3':
		if (g_camera) g_camera = 1 - g_camera;
		if (t_camera) t_camera = 1 - t_camera;
		set_current_camera(CAMERA_4);
		glutPostRedisplay();
		break;
	case '5':
		float degree;
		EC_spot_flag = 1 - EC_spot_flag;
		if (EC_spot_flag) degree = 20.0f;
		else degree = 180.0f;
		
		glUseProgram(h_ShaderProgram_GS);
		glUniform1f(loc_light_GS[0].spot_cutoff_angle, degree);
		glUseProgram(0);
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1f(loc_light[0].spot_cutoff_angle, degree);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case '6':
		view_normal_flag = 1 - view_normal_flag;
		glUseProgram(h_ShaderProgram_GS);
		glUniform1i(loc_u_view_normal_GS, view_normal_flag);
		glUseProgram(0);
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_u_view_normal_TXPS, view_normal_flag);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void reshape(int width, int height) {
	float aspect_ratio;

	glViewport(0, 0, width, height);

	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &grid_VAO);
	glDeleteBuffers(1, &grid_VBO);

	glDeleteVertexArrays(scene.n_materials, bistro_exterior_VAO);
	glDeleteBuffers(scene.n_materials, bistro_exterior_VBO);

	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);

	glDeleteTextures(scene.n_textures, bistro_exterior_texture_names);

	free(bistro_exterior_n_triangles);
	free(bistro_exterior_vertex_offset);

	free(bistro_exterior_VAO);
	free(bistro_exterior_VBO);

	free(bistro_exterior_texture_names);
	free(flag_texture_mapping);
}
/*********************  END: callback function definitions **********************/

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);

	glutMotionFunc(my_mousemove);
	glutMouseWheelFunc(my_mousewheel);
	glutMouseFunc(my_mousepress);
	glutTimerFunc(100, timer_scene, 0);

	glutCloseFunc(cleanup);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	initialize_lights();
	initialize_flags();
}

void prepare_scene(void) {
	prepare_axes();
	prepare_grid();
	prepare_bistro_exterior();

	// project code
	my_prepare_objects();
	set_up_scene_lights();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	initialize_camera();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "********************************************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "********************************************************************************\n\n");
}

void print_message(const char* m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "********************************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n********************************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 9
void drawScene(int argc, char* argv[]) {
	char program_name[64] = "Sogang CSE4170 Bistro Exterior Scene";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used:",
		"		'f' : draw x, y, z axes and grid",
		"		'1' : set the camera for original view",
		"		'2' : set the camera for bistro view",
		"		'3' : set the camera for tree view",
		"		'4' : set the camera for top view",
		"		'5' : set the camera for front view",
		"		'6' : set the camera for side view",
		"		'ESC' : program close",
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(900, 600);
	glutInitWindowPosition(20, 20);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
