// Windows includes (For Time, IO, etc.)
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
// Assimp includes
#include <assimp/cimport.h> // scene importer 
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
// Project includes
#include "maths_funcs.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// Meshes to Load
#define PARENT_NAME		"models/body.dae"
#define CHILD_NAME		"models/tail.dae"
#define LEYE_NAME		"models/l_eye.dae"
#define REYE_NAME		"models/r_eye.dae"
#define LIDS_NAME		"models/eyelids.dae"
#define NOSE_NAME		"models/nose.dae"
#define LLEG_NAME		"models/l_leg.dae"
#define RLEG_NAME		"models/r_leg.dae"
#define BG_NAME			"models/grass.dae"
#define FENCE_NAME		"models/fence.dae"
#define COLLAR_NAME		"models/collar.dae"
#define KENNEL_NAME		"models/kennel.dae"
#define TREE_NAME		"models/trees.dae"
#define GRASS_NAME		"models/vegetation.dae"
// Textures to Load
#define COLLAR			"textures/collartexture.jpg"
#define GRASS			"textures/grass.jpg"
#define	WOOD			"textures/wood.jpg"
#define	LEATHER			"textures/leather.jpg"
#define	KENNEL			"textures/kenneltexture.jpg"
#define	BARK			"textures/bark.jpg"
#define	EYEBALL			"textures/eye.jpg"
#define	FUR				"textures/pomeranian.png"
// Sounds to Load
#define	AMBIENT			"sounds/475635__o-ciz__forest-windy.wav"

#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;

// Shaders
GLuint shaderProgramID;
// Screen
int width = 1120;
int height = 630;
// Models
ModelData parent, child;
ModelData eyelids, leye, reye, nose;
ModelData lleg, rleg;
ModelData background, fence, collar, kennel;
ModelData veg, trees;
// Textures
unsigned int tree_tex, collar_tex, grass_tex, fence_tex, leather_tex, wood_tex, fur_tex, eyeball_tex;
// Buffers
unsigned int VP_VBOs[14]; // vertex positions
unsigned int VN_VBOs[14]; // vertex normals
unsigned int VT_VBOs[14]; // vertex textures
unsigned int VAOs[1];

GLuint loc1[8 * 3];
GLuint loc2[6 * 3];

GLfloat rotate_y = 0.0f;
GLfloat rotate_x = 0.0f;
GLfloat rotate_z = 0.0f;

// Rotations
float tail_rotation = 0.0f;
float tree_rotation = 0.1f;
float model_rotation = 0.0f;
// translation
float translation_x = -2.0f;
float translation_y = 0.05f;
float translation_z = 2.0f;
// Animations
boolean left_up = true;
boolean right_up = false;
float tail_z = 0.0f;
float tree_z = 0.0f;
float leg_l_y = 0.0f;
float leg_r_y = 0.0f;
float leg_translation = 0.0f;
// Scaling
float scaling_x = 1.4f;
float scaling_y = 1.4f;
float scaling_z = 1.4f;
// Camera
float camera_x = 0.0f;
float camera_y = 5.0f;
float camera_z = 15.0f;
float target_x = 0.0f;
float target_y = 0.0f;
float target_z = 0.0f;
vec3 camera_pos = vec3(0.0f, 0.0f, 0.0f);		// initial position of eye
vec3 camera_target = vec3(0.0f, 0.0f, 0.0f);	// initial position of target
vec3 up = vec3(0.0f, 1.0f, 0.0f);				// up vector
// Mouse Movement
float mouse_x = 0.0f;
float mouse_y = 0.0f;
float old_x = 0.0f;
float old_y = 0.0f;
float forward_x = 0.0f;
float forward_y = 0.0f;
float forward_z = 0.0f;
float left_x = 0.0f;
float left_y = 0.0f;
float left_z = 0.0f;
float right_x = 0.0f;
float right_y = 0.0f;
float right_z = 0.0f; 
float speed = 0.01f;
// 3d Perspective
mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
// Loads Textures using ASSIMP
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
// Loads Textures using STB Image
#pragma region TEXTURE LOADING
unsigned int load_tex(const char* file_name) {
	// load and create a texture 
	unsigned int texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(file_name, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	return texture;
}
#pragma endregion TEXTURE LOADING
// Shader Functions
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
#pragma endregion SHADER_FUNCTIONS
// VBO Functions 
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh() {
	// load meshes
	parent = load_mesh(PARENT_NAME);
	child = load_mesh(CHILD_NAME);
	eyelids = load_mesh(LIDS_NAME);
	leye = load_mesh(LEYE_NAME);
	reye = load_mesh(REYE_NAME);
	nose = load_mesh(NOSE_NAME);
	lleg = load_mesh(LLEG_NAME);
	rleg = load_mesh(RLEG_NAME);
	background = load_mesh(BG_NAME);
	fence = load_mesh(FENCE_NAME);
	collar = load_mesh(COLLAR_NAME);
	kennel = load_mesh(KENNEL_NAME);
	trees = load_mesh(TREE_NAME);
	veg = load_mesh(GRASS_NAME);
	// load textures
	collar_tex = load_tex(COLLAR);
	grass_tex = load_tex(GRASS);
	fence_tex = load_tex(WOOD);
	leather_tex = load_tex(LEATHER);
	wood_tex = load_tex(KENNEL);
	tree_tex = load_tex(BARK);
	fur_tex = load_tex(FUR);
	eyeball_tex = load_tex(EYEBALL);
	// Generate Buffers
	glGenBuffers(14, VP_VBOs); // position buffer
	glGenBuffers(14, VN_VBOs); // normal buffer
	glGenBuffers(14, VT_VBOs); // texture buffer
	
	// Dog - Body
	loc1[0] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc1[1] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc1[2] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, parent.mPointCount * sizeof(vec3), &parent.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, parent.mPointCount * sizeof(vec3), &parent.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, parent.mPointCount * sizeof(vec2), &parent.mTextureCoords[0], GL_STATIC_DRAW);
	// Dog - Tail
	loc1[3] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc1[4] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc1[5] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, child.mPointCount * sizeof(vec3), &child.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, child.mPointCount * sizeof(vec3), &child.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, child.mPointCount * sizeof(vec2), &child.mTextureCoords[0], GL_STATIC_DRAW);
	// Dog - Eyelids
	loc1[6] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc1[7] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc1[8] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[2]);
	glBufferData(GL_ARRAY_BUFFER, eyelids.mPointCount * sizeof(vec3), &eyelids.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[2]);
	glBufferData(GL_ARRAY_BUFFER, eyelids.mPointCount * sizeof(vec3), &eyelids.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[2]);
	glBufferData(GL_ARRAY_BUFFER, eyelids.mPointCount * sizeof(vec2), &eyelids.mTextureCoords[0], GL_STATIC_DRAW);
	// Dog - Left Eye
	loc1[9] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc1[10] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc1[11] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[3]);
	glBufferData(GL_ARRAY_BUFFER, leye.mPointCount * sizeof(vec3), &leye.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[3]);
	glBufferData(GL_ARRAY_BUFFER, leye.mPointCount * sizeof(vec3), &leye.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[3]);
	glBufferData(GL_ARRAY_BUFFER, leye.mPointCount * sizeof(vec2), &leye.mTextureCoords[0], GL_STATIC_DRAW);
	// Dog - Right Eye
	loc1[12] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc1[13] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc1[14] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[4]);
	glBufferData(GL_ARRAY_BUFFER, reye.mPointCount * sizeof(vec3), &reye.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[4]);
	glBufferData(GL_ARRAY_BUFFER, reye.mPointCount * sizeof(vec3), &reye.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[4]);
	glBufferData(GL_ARRAY_BUFFER, reye.mPointCount * sizeof(vec2), &reye.mTextureCoords[0], GL_STATIC_DRAW);
	// Dog - Nose
	loc1[15] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc1[16] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc1[17] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[5]);
	glBufferData(GL_ARRAY_BUFFER, nose.mPointCount * sizeof(vec3), &nose.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[5]);
	glBufferData(GL_ARRAY_BUFFER, nose.mPointCount * sizeof(vec3), &nose.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[5]);
	glBufferData(GL_ARRAY_BUFFER, nose.mPointCount * sizeof(vec2), &nose.mTextureCoords[0], GL_STATIC_DRAW);
	// Dog - Left Leg
	loc1[18] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc1[19] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc1[20] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[6]);
	glBufferData(GL_ARRAY_BUFFER, lleg.mPointCount * sizeof(vec3), &lleg.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[6]);
	glBufferData(GL_ARRAY_BUFFER, lleg.mPointCount * sizeof(vec3), &lleg.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[6]);
	glBufferData(GL_ARRAY_BUFFER, lleg.mPointCount * sizeof(vec2), &lleg.mTextureCoords[0], GL_STATIC_DRAW);
	// Dog - Right Leg
	loc1[21] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc1[22] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc1[23] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[7]);
	glBufferData(GL_ARRAY_BUFFER, rleg.mPointCount * sizeof(vec3), &rleg.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[7]);
	glBufferData(GL_ARRAY_BUFFER, rleg.mPointCount * sizeof(vec3), &rleg.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[7]);
	glBufferData(GL_ARRAY_BUFFER, rleg.mPointCount * sizeof(vec2), &rleg.mTextureCoords[0], GL_STATIC_DRAW);
	// Scene - Ground
	loc2[0] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2[1] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc2[2] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[8]);
	glBufferData(GL_ARRAY_BUFFER, background.mPointCount * sizeof(vec3), &background.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[8]);
	glBufferData(GL_ARRAY_BUFFER, background.mPointCount * sizeof(vec3), &background.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[8]);
	glBufferData(GL_ARRAY_BUFFER, background.mPointCount * sizeof(vec2), &background.mTextureCoords[0], GL_STATIC_DRAW);
	// Scene - Fence
	loc2[3] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2[4] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc2[5] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[9]);
	glBufferData(GL_ARRAY_BUFFER, fence.mPointCount * sizeof(vec3), &fence.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[9]);
	glBufferData(GL_ARRAY_BUFFER, fence.mPointCount * sizeof(vec3), &fence.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[9]);
	glBufferData(GL_ARRAY_BUFFER, fence.mPointCount * sizeof(vec2), &fence.mTextureCoords[0], GL_STATIC_DRAW);
	// Scene - Trees
	loc2[6] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2[7] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc2[8] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[10]);
	glBufferData(GL_ARRAY_BUFFER, trees.mPointCount * sizeof(vec3), &trees.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[10]);
	glBufferData(GL_ARRAY_BUFFER, trees.mPointCount * sizeof(vec3), &trees.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[10]);
	glBufferData(GL_ARRAY_BUFFER, trees.mPointCount * sizeof(vec2), &trees.mTextureCoords[0], GL_STATIC_DRAW);
	// Scene - Collar, Stick, & Chain
	loc2[9] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2[10] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc2[11] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[11]);
	glBufferData(GL_ARRAY_BUFFER, collar.mPointCount * sizeof(vec3), &collar.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[11]);
	glBufferData(GL_ARRAY_BUFFER, collar.mPointCount * sizeof(vec3), &collar.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[11]);
	glBufferData(GL_ARRAY_BUFFER, collar.mPointCount * sizeof(vec2), &collar.mTextureCoords[0], GL_STATIC_DRAW);
	// Scene - Grass/Vegetation
	loc2[12] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2[13] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc2[14] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[12]);
	glBufferData(GL_ARRAY_BUFFER, veg.mPointCount * sizeof(vec3), &veg.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[12]);
	glBufferData(GL_ARRAY_BUFFER, veg.mPointCount * sizeof(vec3), &veg.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[12]);
	glBufferData(GL_ARRAY_BUFFER, veg.mPointCount * sizeof(vec2), &veg.mTextureCoords[0], GL_STATIC_DRAW);
	// Scene - Kennel
	loc2[15] = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2[16] = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc2[17] = glGetAttribLocation(shaderProgramID, "vertex_texture");
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[13]);
	glBufferData(GL_ARRAY_BUFFER, kennel.mPointCount * sizeof(vec3), &kennel.mNormals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[13]);
	glBufferData(GL_ARRAY_BUFFER, kennel.mPointCount * sizeof(vec3), &kennel.mVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[13]);
	glBufferData(GL_ARRAY_BUFFER, kennel.mPointCount * sizeof(vec2), &kennel.mTextureCoords[0], GL_STATIC_DRAW);
}
#pragma endregion VBO_FUNCTIONS

void display() {
	// Tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthMask(GL_TRUE); //update the depth buffer
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // sky colour - same colour as fog
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID); 
	// Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
	// Camera/View
	mat4 view = identity_mat4();
	camera_target = vec3(target_x, target_y, target_z);
	camera_pos = vec3(camera_x, camera_y, camera_z);
	view = look_at(camera_pos, camera_target, up);
	// Update Uniforms & Draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	// Root - Dog - Body
	mat4 model = identity_mat4();
	model = rotate_y_deg(model, rotate_x);
	model = rotate_x_deg(model, rotate_y);
	model = rotate_z_deg(model, rotate_z);
	model = translate(model, vec3(translation_x, translation_y, translation_z));
	model = scale(model, vec3(scaling_x, scaling_y, scaling_z));
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
	glEnableVertexAttribArray(loc1[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[0]);
	glVertexAttribPointer(loc1[0], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc1[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[0]);
	glVertexAttribPointer(loc1[1], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc1[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[0]);
	glVertexAttribPointer(loc1[2], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, fur_tex);
	glDrawArrays(GL_TRIANGLES, 0, parent.mPointCount);
	// Child - Dog - Tail
	mat4 modelChild = identity_mat4();
	modelChild = rotate_y_deg(modelChild, 0.0f); // only rotates on z axis-s
	modelChild = rotate_x_deg(modelChild, 0.0f);
	modelChild = rotate_z_deg(modelChild, tail_z);
	modelChild = translate(modelChild, vec3(0.0f, 0.0f, 0.0f));
	modelChild = model * modelChild;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelChild.m);
	glEnableVertexAttribArray(loc1[3]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[1]);
	glVertexAttribPointer(loc1[3], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc1[4]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[1]);
	glVertexAttribPointer(loc1[4], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc1[5]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[1]);
	glVertexAttribPointer(loc1[5], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, fur_tex);
	glDrawArrays(GL_TRIANGLES, 0, child.mPointCount);
	// Child - Dog - Eyelids
	mat4 modelLids = identity_mat4();
	modelLids = model * modelLids;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelLids.m);
	glEnableVertexAttribArray(loc1[6]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[2]);
	glVertexAttribPointer(loc1[6], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc1[7]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[2]);
	glVertexAttribPointer(loc1[7], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc1[8]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[2]);
	glVertexAttribPointer(loc1[8], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, fur_tex);
	glDrawArrays(GL_TRIANGLES, 0, eyelids.mPointCount);
	// Child - Dog - Left Eye
	mat4 modelEyeL = identity_mat4();
	modelEyeL = model * modelEyeL;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelEyeL.m);
	glEnableVertexAttribArray(loc1[9]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[3]);
	glVertexAttribPointer(loc1[9], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc1[10]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[3]);
	glVertexAttribPointer(loc1[10], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc1[11]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[3]);
	glVertexAttribPointer(loc1[11], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, eyeball_tex);
	glDrawArrays(GL_TRIANGLES, 0, leye.mPointCount);
	// Child - Dog - Right Eye
	mat4 modelEyeR = identity_mat4();
	modelEyeR = model * modelEyeR;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelEyeR.m);
	glEnableVertexAttribArray(loc1[12]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[4]);
	glVertexAttribPointer(loc1[12], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc1[13]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[4]);
	glVertexAttribPointer(loc1[13], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc1[14]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[4]);
	glVertexAttribPointer(loc1[14], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, eyeball_tex);
	glDrawArrays(GL_TRIANGLES, 0, reye.mPointCount);
	// Child - Dog - Nose
	mat4 modelNose = identity_mat4();
	modelNose = model * modelNose;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelNose.m);
	glEnableVertexAttribArray(loc1[15]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[5]);
	glVertexAttribPointer(loc1[15], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc1[16]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[5]);
	glVertexAttribPointer(loc1[16], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc1[17]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[5]);
	glVertexAttribPointer(loc1[17], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, leather_tex);
	glDrawArrays(GL_TRIANGLES, 0, nose.mPointCount);
	// Child - Dog - Left Leg
	mat4 modelLegL = identity_mat4();
	modelLegL = translate(modelLegL, vec3(0.0f, leg_l_y, 0.0f)); // update y values during animation
	modelLegL = model * modelLegL;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelLegL.m);
	glEnableVertexAttribArray(loc1[18]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[6]);
	glVertexAttribPointer(loc1[18], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc1[19]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[6]);
	glVertexAttribPointer(loc1[19], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc1[20]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[6]);
	glVertexAttribPointer(loc1[20], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, fur_tex);
	glDrawArrays(GL_TRIANGLES, 0, lleg.mPointCount);
	// Child - Dog - Right Leg
	mat4 modelLegR = identity_mat4();
	modelLegR = translate(modelLegR, vec3(0.0f, leg_r_y, 0.0f)); // update y values during animation
	modelLegR = model * modelLegR;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelLegR.m);
	glEnableVertexAttribArray(loc1[21]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[7]);
	glVertexAttribPointer(loc1[21], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc1[22]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[7]);
	glVertexAttribPointer(loc1[22], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc1[23]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[7]);
	glVertexAttribPointer(loc1[23], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, fur_tex);
	glDrawArrays(GL_TRIANGLES, 0, rleg.mPointCount);
	// Root - Scene - Ground
	mat4 bg = identity_mat4();
	bg = rotate_y_deg(bg, 300.0f);
	bg = rotate_x_deg(bg, 0.0f);
	bg = rotate_z_deg(bg, 0.0f);
	bg = translate(bg, vec3(0.0f, -0.5f, 0.0f));
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, bg.m);
	glEnableVertexAttribArray(loc2[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[8]);
	glVertexAttribPointer(loc2[0], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[8]);
	glVertexAttribPointer(loc2[1], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc2[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[8]);
	glVertexAttribPointer(loc2[2], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, grass_tex);
	glDrawArrays(GL_TRIANGLES, 0, background.mPointCount);
	// Child - Scene - Fence
	mat4 bgChild = identity_mat4();
	bgChild = bg * bgChild;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, bgChild.m);
	glEnableVertexAttribArray(loc2[3]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[9]);
	glVertexAttribPointer(loc2[3], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2[4]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[9]);
	glVertexAttribPointer(loc2[4], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc2[5]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[9]);
	glVertexAttribPointer(loc2[5], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, fence_tex);
	glDrawArrays(GL_TRIANGLES, 0, fence.mPointCount);
	// Root - Scene - Trees
	mat4 bgTrees = identity_mat4();
	bgTrees = translate(bgTrees, vec3(0.0f, -0.5f, 0.0f));
	bgTrees = rotate_x_deg(bgTrees, tree_z); // rotation for "wind" appearance
	bgTrees = bg * bgTrees;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, bgTrees.m);
	glEnableVertexAttribArray(loc2[6]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[10]);
	glVertexAttribPointer(loc2[6], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2[7]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[10]);
	glVertexAttribPointer(loc2[7], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc2[8]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[10]);
	glVertexAttribPointer(loc2[8], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, tree_tex);
	glDrawArrays(GL_TRIANGLES, 0, trees.mPointCount);
	// Root - Scene - Collar, Stick, Chain
	mat4 bgCollar = identity_mat4();
	bgCollar = bg * bgCollar;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, bgCollar.m);
	glEnableVertexAttribArray(loc2[9]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[11]);
	glVertexAttribPointer(loc2[9], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2[10]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[11]);
	glVertexAttribPointer(loc2[10], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc2[11]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[11]);
	glVertexAttribPointer(loc2[11], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, collar_tex);
	glDrawArrays(GL_TRIANGLES, 0, collar.mPointCount);
	// Root - Scene - Grass/Vegetation
	mat4 bgGrass = identity_mat4();
	bgGrass = bg * bgGrass;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, bgGrass.m);
	glEnableVertexAttribArray(loc2[12]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[12]);
	glVertexAttribPointer(loc2[12], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2[13]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[12]);
	glVertexAttribPointer(loc2[13], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc2[14]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[12]);
	glVertexAttribPointer(loc2[14], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, grass_tex);
	glDrawArrays(GL_TRIANGLES, 0, veg.mPointCount);
	// Root - Scene - Kennel
	mat4 bgKennel = identity_mat4();
	bgKennel = bg * bgKennel;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, bgKennel.m);
	glEnableVertexAttribArray(loc2[15]);
	glBindBuffer(GL_ARRAY_BUFFER, VP_VBOs[13]);
	glVertexAttribPointer(loc2[15], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2[16]);
	glBindBuffer(GL_ARRAY_BUFFER, VN_VBOs[13]);
	glVertexAttribPointer(loc2[16], 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(VAOs[0]);
	glEnableVertexAttribArray(loc2[17]);
	glBindBuffer(GL_ARRAY_BUFFER, VT_VBOs[13]);
	glVertexAttribPointer(loc2[17], 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindTexture(GL_TEXTURE_2D, wood_tex);
	glDrawArrays(GL_TRIANGLES, 0, kennel.mPointCount);
	glutSwapBuffers();
}

void updateScene() {
	// Delta Calculation
	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;
	// Proximity Calculation
	vec3 dist = vec3(translation_x, translation_y, translation_z) - vec3(camera_x, camera_y, camera_z);
	float x_distance = dist.v[0];
	float z_distance = dist.v[2];
	// Proximity Check
	if (x_distance > -5 && x_distance < 5 && z_distance > -5 && z_distance < 5) {
		// Rotate Tail
		tail_z += tail_rotation * delta;
		if (fmodf(tail_z, 15.0f) > 0) tail_rotation = -30.0f;
		if (tail_z / 15.0f <= -1) tail_rotation = 30.0f;
		// Wiggle Legs
		if (left_up) {
			leg_l_y += leg_translation;
			if (leg_r_y > 0.0f) leg_r_y -= leg_translation;
			if (leg_l_y >= 0.1f) {
				left_up = false;
				right_up = true;
			}
		}
		if (right_up) {
			leg_r_y += leg_translation;
			if (leg_l_y > 0.0f) leg_l_y -= leg_translation;
			if (leg_r_y >= 0.1f) {
				left_up = true;
				right_up = false;
			}
		}
	}
	// Rotate Model
	rotate_x += model_rotation * delta;
	rotate_x = fmodf(rotate_x, 360.0f);
	// Rotate Trees (very slight)
	tree_z += tree_rotation * delta;
	if (fmodf(tree_z, 1.0f) > 0) tree_rotation = -0.1f;
	if (tree_z / 1.0f <= -1) tree_rotation = 0.1f;
	// Draw the next frame
	glutPostRedisplay();
}

void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	// Load mesh into a vertex buffer array
	generateObjectBufferMesh();
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {

	// trigger animation
	if (key == 'x') { // start tail wag
		tail_rotation = 30.0f;
		leg_translation = 0.01f;
	}
	// model rotation
	if (key == ',') {	// rotate left (x-axis)
		model_rotation = 20.0f;
	}
	if (key == '.') {	// rotate right (x-axis)
		model_rotation = -20.0f;
	}
	// camera motions
	if (key == 'w') {	// Move camera forwards
		camera_x += forward_x * speed;
		camera_z += forward_z * speed;
		target_x += forward_x * speed;
		target_z += forward_z * speed;
	}
	if (key == 'a') {	// Move camera left
		vec3 left = cross(vec3(forward_x, forward_y, forward_z), up);
		camera_x -= left.v[0] * speed;
		camera_z -= left.v[2] * speed;
		target_x -= left.v[0] * speed;
		target_z -= left.v[2] * speed;
	}
	if (key == 's') {	// Move camera backwards
		camera_x -= forward_x * speed;
		camera_z -= forward_z * speed;
		target_x -= forward_x * speed;
		target_z -= forward_z * speed;
	}
	if (key == 'd') {	// Move camera left
		vec3 right = cross(vec3(forward_x, forward_y, forward_z), up);
		camera_x += right.v[0] * speed;
		camera_z += right.v[2] * speed;
		target_x += right.v[0] * speed;
		target_z += right.v[2] * speed;
	}
	// model scaling - uniform
	if (key == 'c') {	// Larger
		scaling_x += 0.1f;
		scaling_y += 0.1f;
		scaling_z += 0.1f;
	}
	if (key == 'v') {	// Smaller
		scaling_x -= 0.1f;
		scaling_y -= 0.1f;
		scaling_z -= 0.1f;
	}
	// reset function
	if (key == 32) {
		model_rotation = 0.0f;
		tail_rotation = 0.0f;
		leg_translation = 0.0f;
	}
	glutPostRedisplay();
}

void arrows(int key, int x, int y) {

	if (key == GLUT_KEY_DOWN) {
		translation_y -= 0.1;
	}
	if (key == GLUT_KEY_UP) {
		translation_y += 0.1;
	}
	if (key == GLUT_KEY_LEFT) {
		translation_x -= 0.1;
	}
	if (key == GLUT_KEY_RIGHT) {
		translation_x += 0.1;
	}
	glutPostRedisplay();
}

void mouse(int x, int y) {
	// calc forward vector for camera movement
	forward_x = target_x - camera_x;
	forward_y = target_y - camera_y;
	forward_z = target_z - camera_z;
	// save old mouse vals
	old_x = mouse_x;
	old_y = mouse_y;
	mouse_x = x;
	mouse_y = y;
	// range check
	if (mouse_y < 100) {
		mouse_y = 100;
		glutWarpPointer(mouse_x, height / 2);
	}
	if (mouse_y > height - 100) {
		mouse_y = height - 100;
		glutWarpPointer(mouse_x, height / 2);
	}
	// movements
	if ((mouse_x - old_x) > 0)	// moved right
		target_x += 0.15f;
	else if ((mouse_x - old_x) < 0)	// moved left
		target_x -= 0.15f;
	if ((mouse_y - old_y) > 0)	// moved up
		target_y += 0.15f;
	else if ((mouse_y - old_y) < 0)	// moved down
		target_y -= 0.15f;
}

int main(int argc, char** argv) {
	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Display");
	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutSpecialFunc(arrows);
	glutKeyboardFunc(keypress);
	glutPassiveMotionFunc(mouse);
	glutIdleFunc(updateScene);
	// hide cursor, move to center
	glutSetCursor(GLUT_CURSOR_NONE);
	glutWarpPointer(width / 2, height / 2);
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
	PlaySound(TEXT(AMBIENT), NULL, SND_ASYNC);
	glutMainLoop();
	return 0;
}