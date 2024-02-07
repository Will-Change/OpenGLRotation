// Windows includes (For Time, IO, etc.)
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"
#define GLM_ENABLE_EXPERIMENTAL
// glm includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>



/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME "monkeyhead_smooth.dae"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
unsigned int skyboxVAO, skyboxVBO;

float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

std::vector<std::string> skyboxFaces{
        "posx.jpg",
        "negx.jpg",
        "posy.jpg",
        "negy.jpg",
        "posz.jpg",
        "negz.jpg"};


#pragma region SimpleTypes
typedef struct a
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;
GLuint shaderProgramID;
GLuint skyboxProgramID;

ModelData mesh_data;
unsigned int mesh_vao = 0;
int width = 800;
int height = 600;

GLuint loc1, loc2, loc3;
GLfloat rotate_y = 0.0f;


#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name, 
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	); 

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);
	
	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}

GLuint CompileSkyboxShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	skyboxProgramID = glCreateProgram();
	if (skyboxProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(skyboxProgramID, "skyboxVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(skyboxProgramID, "skyboxFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(skyboxProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(skyboxProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(skyboxProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(skyboxProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(skyboxProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(skyboxProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(skyboxProgramID);
	return skyboxProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh() {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	mesh_data = load_mesh(MESH_NAME);
	unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);
	unsigned int vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

	//	This is for texture coordinates which you don't currently need, so I have commented it out
	//	unsigned int vt_vbo = 0;
	//	glGenBuffers (1, &vt_vbo);
	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	//	glBufferData (GL_ARRAY_BUFFER, monkey_head_data.mTextureCoords * sizeof (vec2), &monkey_head_data.mTextureCoords[0], GL_STATIC_DRAW);

	unsigned int vao = 0;
	glBindVertexArray(vao);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//	This is for texture coordinates which you don't currently need, so I have commented it out
	//	glEnableVertexAttribArray (loc3);
	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	//	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
}
#pragma endregion VBO_FUNCTIONS

GLfloat pitch = 0.0f;
GLfloat yaw = 0.0f;
GLfloat roll = 0.0f;

enum CameraMode { DEFAULT, FIRST_PERSON, THIRD_PERSON }; 
CameraMode currentCameraMode = DEFAULT;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -5.0f);

void keypress(unsigned char key, int x, int y) {
    const float rotationSpeed = 5.0f; 

    switch (key) {
        case 'w': // Increase pitch
            pitch += rotationSpeed;
            break;
        case 's': // Decrease pitch
            pitch -= rotationSpeed;
            break;
        case 'a': // Increase yaw (left)
            yaw += rotationSpeed;
            break;
        case 'd': // Decrease yaw (right)
            yaw -= rotationSpeed;
            break;
        case 'q': // Increase roll (left)
            roll += rotationSpeed;
            break;
        case 'e': // Decrease roll (right)
            roll -= rotationSpeed;
            break;
        case 'f': // First-person view
            currentCameraMode = FIRST_PERSON;
            break;
        case 't': // Third-person view
            currentCameraMode = THIRD_PERSON;
            break;
        case 'g': // Default view
            currentCameraMode = DEFAULT;
            break;
        default:
            break; 
    }
    // Ensure the scene gets updated with new rotations
    glutPostRedisplay();
}
///*

GLuint loadCubemap(std::vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (GLuint i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  

    return textureID;
}

GLuint skyboxTextureID = loadCubemap(skyboxFaces);

void renderSkybox() {
    glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
    glUseProgram(skyboxProgramID); // Use skybox shader program
    // Setup view and projection matrices here (remove translation from view matrix)
    
    // Bind VAO and texture, then draw the skybox
    glBindVertexArray(skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // Set depth function back to default
}
// quaternion
    glm::vec3 cameraTarget = glm::vec3(0.0, 0.0, 0.0); // Plane's center as the target
    glm::vec3 up = glm::vec3(0.0, 1.0, 0.0); // Up direction
void display() {
    glEnable(GL_DEPTH_TEST);
    
    glDepthFunc(GL_LEQUAL);
    //glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    // Calculate view matrix without the translation component for the skybox
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, up); // You should have these variables defined based on your camera's current position and orientation
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view)); // Remove translation component for the skybox

    // Render skybox first
    glDepthMask(GL_FALSE); // Disable writing to the depth buffer
    glUseProgram(skyboxProgramID); // Use skybox shader

    // Pass the modified view matrix and your projection matrix to the shader
    glUniformMatrix4fv(glGetUniformLocation(skyboxProgramID, "view"), 1, GL_FALSE, glm::value_ptr(viewNoTranslation));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxProgramID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Bind and draw the skybox
    glBindVertexArray(skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE); // Enable writing to the depth buffer again for other objects

    // Now render your scene as usual
    glUseProgram(shaderProgramID);

    // You might need to recalculate or reset the view matrix for the rest of your scene


    int matrix_location = glGetUniformLocation(shaderProgramID, "model");
    int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
    int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");

    glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

    // Quaternion-based rotation for the plane
    glm::quat pitchQuat = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawQuat = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat rollQuat = glm::angleAxis(glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::quat combinedQuat = rollQuat * yawQuat * pitchQuat;
    glm::mat4 rotationMatrix = glm::toMat4(combinedQuat);
    glm::mat4 model = rotationMatrix;

    

    glm::vec3 modelPosition = glm::vec3(0.0, 0.0, 0.0); // Assuming this is the model's position
    switch (currentCameraMode) {
        case FIRST_PERSON:
            // Adjust this offset for eye height
            glm::vec3 eyeOffset = glm::vec3(0.0, 2.5, 0.0); // Adjust this based on your model's size and desired eye height
		    cameraPos = modelPosition + eyeOffset; // Position the camera at eye level relative to the model

		    // Calculate the direction the model is looking, assuming forward is along the negative Z-axis in model space
		    glm::vec3 lookDirection = glm::vec3(combinedQuat * glm::vec4(0.0, 0.0, -1.0, 0.0));
		    cameraTarget = cameraPos + lookDirection; // The point the camera is looking at, in world space
		    break;
        case THIRD_PERSON:
            {
                glm::vec3 cameraOffset = glm::vec3(0.0f, 5.0f, 7.5f);
                glm::vec3 offsetPosition = glm::vec3(combinedQuat * glm::vec4(cameraOffset, 0.0));
                cameraPos = modelPosition + offsetPosition;
                cameraTarget = modelPosition;
            }
            break;
        case DEFAULT:            
        cameraPos = glm::vec3(0.0, 0.0, -5.0); // Fixed position relative to the world
            break;
    }
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = model * rotation;

    view = glm::lookAt(cameraPos, cameraTarget, up);

    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(persp_proj));

    glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);

    //renderSkybox();

    glutSwapBuffers();
}
void display12() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    // You might need to recalculate or reset the view matrix for the rest of your scene


    int matrix_location = glGetUniformLocation(shaderProgramID, "model");
    int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
    int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");

    glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

    // Quaternion-based rotation for the plane
    glm::quat pitchQuat = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawQuat = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat rollQuat = glm::angleAxis(glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::quat combinedQuat = rollQuat * yawQuat * pitchQuat;
    glm::mat4 rotationMatrix = glm::toMat4(combinedQuat);
    glm::mat4 model = rotationMatrix;

    glm::vec3 cameraPos;
    glm::vec3 cameraTarget = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);

    switch (currentCameraMode) {
        case FIRST_PERSON:
            cameraPos = glm::vec3(model * glm::vec4(0.0, 0.5, 0.0, 1.0)); // Adjust for a realistic cockpit view
            cameraTarget = glm::vec3(model * glm::vec4(0.0, 0.5, -1.0, 1.0)); // Looking forward from the cockpit
            break;
        case THIRD_PERSON:
            // Calculate the offset position in the plane's local space and then transform it to world space
            glm::vec3 offset = glm::vec3(0.0f, 2.0f, 10.0f); // Offset the camera above and behind the plane
            cameraPos = glm::vec3(model * glm::vec4(offset, 1.0)); // Transform offset to world space
            cameraTarget = glm::vec3(model * glm::vec4(0.0, 0.0, 0.0, 1.0)); // Plane's position in world space
            break;
        case DEFAULT:
            cameraPos = glm::vec3(0.0, 0.0, -5.0f);
            break;
    }

    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, up);

    persp_proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(persp_proj));

    glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);

    glutSwapBuffers();
}

void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	GLuint skyboxProgramID = CompileSkyboxShaders();
	// load mesh into a vertex buffer array
	generateObjectBufferMesh();

	// Setup skybox
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // We assume the skybox shader expects the vertex position at location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Load cubemap textures
    skyboxTextureID = loadCubemap(skyboxFaces);
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Hello Triangle");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}

//*/
// euler
/*
void display() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    int matrix_location = glGetUniformLocation(shaderProgramID, "model");
    int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
    int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 cameraPos;
    glm::vec3 cameraTarget = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);

    switch (currentCameraMode) {
        case FIRST_PERSON:
            cameraPos = glm::vec3(model * glm::vec4(0.0, 0.5, 0.0, 1.0)); // Adjust for a realistic cockpit view
            cameraTarget = glm::vec3(model * glm::vec4(0.0, 0.5, -1.0, 1.0)); // Looking forward from the cockpit
            break;
        case THIRD_PERSON:
            // Calculate the offset position in the plane's local space and then transform it to world space
            glm::vec3 offset = glm::vec3(0.0f, 2.0f, 10.0f); // Offset the camera above and behind the plane
            cameraPos = glm::vec3(model * glm::vec4(offset, 1.0)); // Transform offset to world space
            cameraTarget = glm::vec3(model * glm::vec4(0.0, 0.0, 0.0, 1.0)); // Plane's position in world space
            break;
        case DEFAULT:
            cameraPos = glm::vec3(0.0, 0.0, -5.0f);
            break;
    }

    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, up);

    glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(persp_proj));

    glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);

    glutSwapBuffers();
}


*/
