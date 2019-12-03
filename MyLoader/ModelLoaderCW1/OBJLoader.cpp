//////////////////////////////////////////////////////////////////////////////
//
//  OBJLoader.cpp
//
//////////////////////////////////////////////////////////////////////////////

// Importing Libraries 
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GLFW/glfw3.h"
#include "LoadShaders.h"
#include <glm/glm.hpp> //includes GLM
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <glm/ext/matrix_transform.hpp> // GLM: translate, rotate
#include <glm/ext/matrix_clip_space.hpp> // GLM: perspective and ortho 
#include <glm/gtc/type_ptr.hpp> // GLM: access to the value_ptr
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include<vector>
#include<fstream>
#include<string>
#include<conio.h>

//Specify the values  for a variable and the possible set of values they can get
enum VAO_IDs { Cube, NumVAOs = 1 };
enum Buffer_IDs { Triangles, Colours, Textures, Normals, NumBuffers = 4 };

GLuint  VAOs[NumVAOs];
GLuint  Buffers[NumBuffers];
GLuint texture1;

GLuint program;

GLuint NumVertices = 36;
//----------------------------------------------------------------------------
//
// Function to read the file path, writing the data into the out_vertices, out_uvs, out_normals and return error if something wrong
// The out are the arrays that can be modified by will and will store the data
void loadOBJ(const char* path, std::vector < glm::vec3 >& out_vertices, std::vector < glm::vec2 >& out_uvs, std::vector < glm::vec3 >& out_normals) {

	// temporary variables that will store the contents of the obj
	std::vector< GLuint > vertexIndices, uvIndices, normalIndices;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;

	// open the obj file
	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
	}
	// now we read the file until the end
	while (1) {
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF) 
			break; // EOF = End Of File. Quit the loop.

		// First we deal with the vertices if the line starts with v then the rest have to be 3 floats
		// and we create a glm::vec out of them then add it to the vector
		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		// Same as vertices goes with the textures
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		// Same with above goes the normals
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		// Same with the faces but with more data to read
		else if (strcmp(lineHeader, "f") == 0) {
			/*std::string vertex1, vertex2, vertex3, vertex4;*/
			GLuint vertexIndex[4], uvIndex[4], normalIndex[4];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d  %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2], &vertexIndex[3], &uvIndex[3], &normalIndex[3]);
			if (matches != 12) {
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[2]);
			vertexIndices.push_back(vertexIndex[3]);

			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);

			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[2]);
			uvIndices.push_back(uvIndex[3]);

			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);

			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[2]);
			normalIndices.push_back(normalIndex[3]);
		}
	}

	// For each vertex of each triangle
	for (GLuint i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		GLuint vertexIndex = vertexIndices[i];
		GLuint uvIndex = uvIndices[i];
		GLuint normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uvs = temp_uvs[uvIndex - 1];
		glm::vec3 normals = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uvs);
		out_normals.push_back(normals);
	}
}

void loadTexture(GLuint& texture, std::string texturepath)
{
	// load and create a texture 
// -------------------------

// texture 1
// ---------
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	GLint width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(texturepath.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

//----------------------------------------------------------------------------
//
// init
//
#define BUFFER_OFFSET(a) ((void*)(a))

void
init(void)
{
	glGenVertexArrays(NumVAOs, VAOs);
	glBindVertexArray(VAOs[Cube]);

	//Manages the Shader
	ShaderInfo  shaders[] =
	{
		{ GL_VERTEX_SHADER, "media/triangles.vert" },
		{ GL_FRAGMENT_SHADER, "media/triangles.frag" },
		{ GL_NONE, NULL }
	};

	program = LoadShaders(shaders);
	glUseProgram(program);

	// ambient light
	glm::vec4 ambient = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	//adding the Uniform to the shader
	GLuint aLoc = glGetUniformLocation(program, "ambient");
	glUniform4fv(aLoc, 1, glm::value_ptr(ambient));

	// light object
	glm::vec3 lightPos = glm::vec3(100.0f, 25.0f, 100.0f);
	GLuint dLightPosLoc = glGetUniformLocation(program, "lightPos");
	glUniform3fv(dLightPosLoc, 1, glm::value_ptr(lightPos));


	// diffuse light
	glm::vec3 diffuseLight = glm::vec3(0.5f, 0.2f, 0.7f);
	GLuint dLightLoc = glGetUniformLocation(program, "dLight");
	glUniform3fv(dLightLoc, 1, glm::value_ptr(diffuseLight));

	// specular light
	glm::vec3 specularLight = glm::vec3(0.7f);
	GLfloat shininess = 256; //128 is max value
	GLuint sLightLoc = glGetUniformLocation(program, "sLight");
	GLuint sShineLoc = glGetUniformLocation(program, "sShine");
	glUniform3fv(sLightLoc, 1, glm::value_ptr(specularLight));
	glUniform1fv(sShineLoc, 1, &shininess);

	//We declare the values and load the Creeper.obj file
	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
    std::vector< glm::vec3 > normals; 
	loadOBJ("media/textures/Creeper.obj", vertices, uvs, normals);

	// Colour Coordinates
	GLfloat  colours[][4] = {
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f }, 
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },
	};

	glGenBuffers(NumBuffers, Buffers);

	///Vertices and Indices Binding
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Triangles]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(Triangles, 3, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	///Normals Binding
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Normals]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glVertexAttribPointer(Normals, 3, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	//Colour Binding
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Colours]);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(colours), colours, 0);

	glVertexAttribPointer(Colours, 4, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	//Texture Binding
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Textures]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(Textures, 2, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	loadTexture(texture1, "media/textures/Texture.png");
	glUniform1i(glGetUniformLocation(program, "texture1"), 0);


	// creating the model matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));


	// creating the view matrix
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));

	// creating the projection matrix
	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3, 0.1f, 20.0f);

	// Adding all matrices up to create combined matrix
	glm::mat4 mv = view * model;


	//adding the Uniform to the shader
	int mvLoc = glGetUniformLocation(program, "mv_matrix");
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mv));
	//adding the Uniform to the shader
	int pLoc = glGetUniformLocation(program, "p_matrix");
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glEnableVertexAttribArray(Triangles);
	glEnableVertexAttribArray(Colours);
	glEnableVertexAttribArray(Textures);
	glEnableVertexAttribArray(Normals);
}

//----------------------------------------------------------------------------
//
// display
//
void
display(GLfloat delta)
{
	static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	
	glClearBufferfv(GL_COLOR, 0, black);
	glClear(GL_COLOR_BUFFER_BIT);
	// bind textures on corresponding texture units
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// creating the model matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(-1.5f, -1.5f, -1.5f));
	model = glm::rotate(model, glm::radians(delta), glm::vec3(1.0f, 0.0f, 1.5f));


	// creating the view matrix
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));

	// creating the projection matrix
	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3, 0.1f, 20.0f);

	// Adding all matrices up to create combined matrix
	glm::mat4 mv = view * model;


	//adding the Uniform to the shader
	int mvLoc = glGetUniformLocation(program, "mv_matrix");
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mv));
	//adding the Uniform to the shader
	int pLoc = glGetUniformLocation(program, "p_matrix");
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(projection));



	glBindVertexArray(VAOs[Cube]);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

//----------------------------------------------------------------------------
//
// main
//
int
main(int argc, char** argv)
{
		glfwInit();

		GLFWwindow* window = glfwCreateWindow(800, 600, "Model Loader", NULL, NULL);

		glfwMakeContextCurrent(window);
		glewInit();

		init();
		GLfloat timer = 0.0f;

		while (!glfwWindowShouldClose(window))
		{
			// uncomment to draw only wireframe 
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			display(timer);
			glfwSwapBuffers(window);
			glfwPollEvents();
			timer += 0.02f;
		}
		glfwDestroyWindow(window);

		glfwTerminate();
}

