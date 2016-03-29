// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;

// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#define PI 3.1415926535897

const int window_width = 1024, window_height = 768;

typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void loadObject(char*, glm::vec4, Vertex * &, GLushort* &, int);
void createVAOs(Vertex[], GLushort[], int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);
glm::vec3 setLookat(void);
void rotateCamera(void);
void deselectObjectIndicies(void);

// SRT Functions
void translateObjectMatrix(glm::mat4x4*, glm::mat4x4, glm::vec3);
void rotateObjectMatrix(glm::mat4x4*, glm::mat4x4, float, glm::vec3);
void scaleObjectMatrix(glm::mat4x4*, glm::mat4x4, glm::vec3);

// Object Setup
void translateBasePosition(void);
void rotateTopPosition(void);
void rotateArm1Position(void);
void rotateArm2Position(void);
void rotatePenPosition(void);

// GLOBAL VARIABLES
GLFWwindow* window;
char* wTitle = "R. Alex Clark (6416-3663)";

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

const GLuint NumObjects = 16;
GLuint VertexArrayId[NumObjects] = { 0, 
									 1, 2, 3, 4, 5, 6, 7, 8, // Base Objects 
									 9, 10, 11, 12, 13, 14, 15 // Selected Objects
								   };
GLuint VertexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
GLuint IndexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

size_t NumIndices[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
size_t VertexBufferSize[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
size_t IndexBufferSize[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint LightID;
GLuint LightID2;

GLint gX = 0.0;
GLint gZ = 0.0;

// animation control
bool animation = false;
GLfloat phi = 0.0;

// Key Action Identifier To Catch Keypresses 
int keyMode = 0;
int shiftPressed = 0;

// Camera Variables
int rotationDirection; // Direction of rotation (L/R, U/D)
float thetaX, thetaY = 1.0; // Rotation positions for left/right & up/down movement of camera

// Object Variables
float BaseXPosition = 0.0;
float BaseZPosition = 0.0;
float TopYRotation = 0.0;
float Arm1ZRotation = 0.0;
float Arm2ZRotation = 0.0;
float PenXRotation = 0.0;
float PenZRotation = 0.0;
float PenYRotation = 0.0;
								 
// Object Matricies
glm::mat4x4 BaseModelMatrix = glm::mat4(1.0);
glm::mat4x4 Arm1ModelMatrix = glm::mat4(1.0);
glm::mat4x4 Arm2ModelMatrix = glm::mat4(1.0);
glm::mat4x4 TopModelMatrix = glm::mat4(1.0);
glm::mat4x4 ButtonModelMatrix = glm::mat4(1.0);
glm::mat4x4 JointModelMatrix = glm::mat4(1.0);
glm::mat4x4 PenModelMatrix = glm::mat4(1.0);

// Object Indicies
unsigned int BaseIndex = 2;
unsigned int Arm1Index = 3;
unsigned int Arm2Index = 4;
unsigned int ButtonIndex = 5;
unsigned int JointIndex = 6;
unsigned int PenIndex = 7;
unsigned int TopIndex = 8;


void translateObjectMatrix(glm::mat4x4 *ModelMatrixToSet, glm::mat4x4 ModelMatrixToTranslateOff, glm::vec3 TranslationDirection) {

	// Translate ModelMatrixToSet  by TranslationDirection from Basis ModelMatrixToTranslateOff
	*ModelMatrixToSet = glm::translate(
		ModelMatrixToTranslateOff,
		TranslationDirection
		);
}

void rotateObjectMatrix(glm::mat4x4 *ModelMatrixToSet, glm::mat4x4 ModelMatrixToRotateOff, float rotation, glm::vec3 RotationDirection) {

	// Rotate Object Model Matrix by rotation in RotationDirection from Basis ModelMatrixToRotateOff
	*ModelMatrixToSet = glm::rotate(
		ModelMatrixToRotateOff,
		rotation,
		RotationDirection
		);
	
}

void scaleObjectMatrix(glm::mat4x4 *ModelMatrixToSet, glm::mat4x4 ModelMatrixToScaleOff, glm::vec3 ScaleAmount) {
		
	// Scale Object Model Matrix by ScaleAmount from Basis ModelMatrixToScaleOff
	*ModelMatrixToSet = glm::scale(
		ModelMatrixToScaleOff,
		ScaleAmount
		);

}


void loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLushort* &out_Indices, int ObjectId)
{
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIndices[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
}

void createObjects(void)
{
	//-- COORDINATE AXES --//
	Vertex CoordVerts[] =
	{
		{ { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
	};

	VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordVerts, NULL, 0);
	
	//-- GRID --//
	Vertex GridVerticies[44];
	for (int i = 0, j = -5.0; i < 44; i += 2, (j >= 5.0) ? j = -5.0 : j++) {

		// Draw the lines parallel to the Z axis
		if (i < 22) {
			GridVerticies[i] = { { j, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
			GridVerticies[i+1] = { { j, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
		}
		// Draw the lines parallel to the X axis
		else {
			GridVerticies[i] = { { -5.0, 0.0, j, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
			GridVerticies[i + 1] = { { 5.0, 0.0, j, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
		}

	}

	// Implement GridVerticies
	VertexBufferSize[1] = sizeof(GridVerticies);
	createVAOs(GridVerticies, NULL, 1);
	
	//-- .OBJs --//

	// ATTN: load your models here
	Vertex* Verts;
	GLushort* Idcs;

	// Load model pieces into space

	// Base Objects
	loadObject("models/base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 2);
	createVAOs(Verts, Idcs, 2);
	loadObject("models/arm1.obj", glm::vec4(0.0, 0.0, 1.0, 1.0), Verts, Idcs, 3);
	createVAOs(Verts, Idcs, 3);
	loadObject("models/arm2.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), Verts, Idcs, 4);
	createVAOs(Verts, Idcs, 4);
	loadObject("models/button.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 5);
	createVAOs(Verts, Idcs, 5);
	loadObject("models/joint.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Verts, Idcs, 6);
	createVAOs(Verts, Idcs, 6);
	loadObject("models/pen.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), Verts, Idcs, 7);
	createVAOs(Verts, Idcs, 7);
	loadObject("models/top.obj", glm::vec4(0.0, 1.0, 0.0, 1.0), Verts, Idcs, 8);
	createVAOs(Verts, Idcs, 8);

	// Selected Objects
	loadObject("models/base.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts, Idcs, 9);
	createVAOs(Verts, Idcs, 9);
	loadObject("models/arm1.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts, Idcs, 10);
	createVAOs(Verts, Idcs, 10);
	loadObject("models/arm2.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts, Idcs, 11);
	createVAOs(Verts, Idcs, 11);
	loadObject("models/button.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), Verts, Idcs, 12);
	createVAOs(Verts, Idcs, 12);
	loadObject("models/joint.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts, Idcs, 13);
	createVAOs(Verts, Idcs, 13);
	loadObject("models/pen.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts, Idcs, 14);
	createVAOs(Verts, Idcs, 14);
	loadObject("models/top.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts, Idcs, 15);
	createVAOs(Verts, Idcs, 15);

}

void deselectObjectIndicies() {
	BaseIndex = 2;
	Arm1Index = 3;
	Arm2Index = 4;
	ButtonIndex = 5;
	JointIndex = 6;
	PenIndex = 7;
	TopIndex = 8;
}

void renderScene(void)
{
	// Update camera view based on arrow key movement
	gViewMatrix = glm::lookAt(setLookat(), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glm::vec3 lightPos2 = glm::vec3(-4, 4, -4);

		glm::mat4x4 ModelMatrix = glm::mat4(1.0);

		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);

		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		// Draw XYZ coordinates axes
		glBindVertexArray(VertexArrayId[0]);
		glDrawArrays(GL_LINES, 0, 6);

		// Draw Grid
		glBindVertexArray(VertexArrayId[1]);
		glDrawArrays(GL_LINES, 0, 44);

		// Translate Objects to Correct Positions
		// Base
		glBindVertexArray(VertexArrayId[BaseIndex]);
		translateObjectMatrix(&BaseModelMatrix, ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		translateObjectMatrix(&BaseModelMatrix, BaseModelMatrix, glm::vec3(0.0f + BaseXPosition, 0.5f, 0.0f + BaseZPosition));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &BaseModelMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[BaseIndex], GL_UNSIGNED_SHORT, 0);

		// Top
		glBindVertexArray(VertexArrayId[TopIndex]);
		translateObjectMatrix(&TopModelMatrix, BaseModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		rotateObjectMatrix(&TopModelMatrix, TopModelMatrix, TopYRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		translateObjectMatrix(&TopModelMatrix, TopModelMatrix, glm::vec3(0.0f, 0.75f, 0.0f));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &TopModelMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[TopIndex], GL_UNSIGNED_SHORT, 0);

		// Arm1
		glBindVertexArray(VertexArrayId[Arm1Index]);
		translateObjectMatrix(&Arm1ModelMatrix, TopModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		rotateObjectMatrix(&Arm1ModelMatrix, Arm1ModelMatrix, float((-1) * PI / 4) + Arm1ZRotation, glm::vec3(0.0f, 0.0f, 1.0f));
		translateObjectMatrix(&Arm1ModelMatrix, Arm1ModelMatrix, glm::vec3(0.0f, 0.75f, 0.0f));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Arm1ModelMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[Arm1Index], GL_UNSIGNED_SHORT, 0);

		// Joint
		glBindVertexArray(VertexArrayId[JointIndex]);
		translateObjectMatrix(&JointModelMatrix, Arm1ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		scaleObjectMatrix(&JointModelMatrix, JointModelMatrix, glm::vec3(0.65f));
		translateObjectMatrix(&JointModelMatrix, JointModelMatrix, glm::vec3(0.0f, 2.05f, 0.0f));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &JointModelMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[JointIndex], GL_UNSIGNED_SHORT, 0);

		// Arm2
		glBindVertexArray(VertexArrayId[Arm2Index]);
		translateObjectMatrix(&Arm2ModelMatrix, JointModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		scaleObjectMatrix(&Arm2ModelMatrix, Arm2ModelMatrix, glm::vec3(2.0f));
		rotateObjectMatrix(&Arm2ModelMatrix, Arm2ModelMatrix, float((-1) * PI / 2.5) + Arm2ZRotation, glm::vec3(0.0f, 0.0f, 1.0f));
		translateObjectMatrix(&Arm2ModelMatrix, Arm2ModelMatrix, glm::vec3(0.0f, 0.5f, 0.0f));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Arm2ModelMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[Arm2Index], GL_UNSIGNED_SHORT, 0);

		// Pen
		glBindVertexArray(VertexArrayId[PenIndex]);
		translateObjectMatrix(&PenModelMatrix, Arm2ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		rotateObjectMatrix(&PenModelMatrix, PenModelMatrix, float(2 * PI / 4), glm::vec3(0.0f, 0.0f, 1.0f));
		translateObjectMatrix(&PenModelMatrix, PenModelMatrix, glm::vec3(0.6f, 0.0f, 0.0f));
		rotateObjectMatrix(&PenModelMatrix, PenModelMatrix, PenXRotation, glm::vec3(1.0f, 0.0f, 0.0f));
		rotateObjectMatrix(&PenModelMatrix, PenModelMatrix, PenZRotation, glm::vec3(0.0f, 0.0f, 1.0f));
		translateObjectMatrix(&PenModelMatrix, PenModelMatrix, glm::vec3(0.0f, 0.2f, 0.0f));
		rotateObjectMatrix(&PenModelMatrix, PenModelMatrix, PenYRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &PenModelMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[PenIndex], GL_UNSIGNED_SHORT, 0);


		// Button
		glBindVertexArray(VertexArrayId[ButtonIndex]);
		translateObjectMatrix(&ButtonModelMatrix, PenModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		translateObjectMatrix(&ButtonModelMatrix, ButtonModelMatrix, glm::vec3(0.05f, 0.0f, 0.0f));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ButtonModelMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[ButtonIndex], GL_UNSIGNED_SHORT, 0);

		glBindVertexArray(0);

	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void pickObject(void)
{
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		
		// Base
		glBindVertexArray(VertexArrayId[BaseIndex]);
		translateObjectMatrix(&BaseModelMatrix, ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		translateObjectMatrix(&BaseModelMatrix, BaseModelMatrix, glm::vec3(0.0f + BaseXPosition, 0.5f, 0.0f + BaseZPosition));
		MVP = gProjectionMatrix * gViewMatrix * BaseModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, BaseIndex / 255.0f);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[BaseIndex], GL_UNSIGNED_SHORT, 0);

		// Top
		glBindVertexArray(VertexArrayId[TopIndex]);
		translateObjectMatrix(&TopModelMatrix, BaseModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		rotateObjectMatrix(&TopModelMatrix, TopModelMatrix, TopYRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		translateObjectMatrix(&TopModelMatrix, TopModelMatrix, glm::vec3(0.0f, 0.75f, 0.0f));
		MVP = gProjectionMatrix * gViewMatrix * TopModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, TopIndex / 255.0f);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[TopIndex], GL_UNSIGNED_SHORT, 0);

		// Arm1
		glBindVertexArray(VertexArrayId[Arm1Index]);
		translateObjectMatrix(&Arm1ModelMatrix, TopModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		rotateObjectMatrix(&Arm1ModelMatrix, Arm1ModelMatrix, float((-1) * PI / 4) + Arm1ZRotation, glm::vec3(0.0f, 0.0f, 1.0f));
		translateObjectMatrix(&Arm1ModelMatrix, Arm1ModelMatrix, glm::vec3(0.0f, 0.75f, 0.0f));
		MVP = gProjectionMatrix * gViewMatrix * Arm1ModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, Arm1Index / 255.0f);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[Arm1Index], GL_UNSIGNED_SHORT, 0);

		// Joint
		glBindVertexArray(VertexArrayId[JointIndex]);
		translateObjectMatrix(&JointModelMatrix, Arm1ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		scaleObjectMatrix(&JointModelMatrix, JointModelMatrix, glm::vec3(0.65f));
		translateObjectMatrix(&JointModelMatrix, JointModelMatrix, glm::vec3(0.0f, 2.05f, 0.0f));
		MVP = gProjectionMatrix * gViewMatrix * JointModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, JointIndex / 255.0f);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[JointIndex], GL_UNSIGNED_SHORT, 0);

		// Arm2
		glBindVertexArray(VertexArrayId[Arm2Index]);
		translateObjectMatrix(&Arm2ModelMatrix, JointModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		scaleObjectMatrix(&Arm2ModelMatrix, Arm2ModelMatrix, glm::vec3(2.0f));
		rotateObjectMatrix(&Arm2ModelMatrix, Arm2ModelMatrix, float((-1) * PI / 2.5) + Arm2ZRotation, glm::vec3(0.0f, 0.0f, 1.0f));
		translateObjectMatrix(&Arm2ModelMatrix, Arm2ModelMatrix, glm::vec3(0.0f, 0.5f, 0.0f));
		MVP = gProjectionMatrix * gViewMatrix * Arm2ModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, Arm2Index / 255.0f);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[Arm2Index], GL_UNSIGNED_SHORT, 0);

		// Pen
		glBindVertexArray(VertexArrayId[PenIndex]);
		translateObjectMatrix(&PenModelMatrix, Arm2ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		rotateObjectMatrix(&PenModelMatrix, PenModelMatrix, float(2 * PI / 4), glm::vec3(0.0f, 0.0f, 1.0f));
		translateObjectMatrix(&PenModelMatrix, PenModelMatrix, glm::vec3(0.6f, 0.0f, 0.0f));
		rotateObjectMatrix(&PenModelMatrix, PenModelMatrix, PenXRotation, glm::vec3(1.0f, 0.0f, 0.0f));
		rotateObjectMatrix(&PenModelMatrix, PenModelMatrix, PenZRotation, glm::vec3(0.0f, 0.0f, 1.0f));
		translateObjectMatrix(&PenModelMatrix, PenModelMatrix, glm::vec3(0.0f, 0.2f, 0.0f));
		rotateObjectMatrix(&PenModelMatrix, PenModelMatrix, PenYRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		MVP = gProjectionMatrix * gViewMatrix * PenModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, PenIndex / 255.0f);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[PenIndex], GL_UNSIGNED_SHORT, 0);


		// Button
		glBindVertexArray(VertexArrayId[ButtonIndex]);
		translateObjectMatrix(&ButtonModelMatrix, PenModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		translateObjectMatrix(&ButtonModelMatrix, ButtonModelMatrix, glm::vec3(0.05f, 0.0f, 0.0f));
		MVP = gProjectionMatrix * gViewMatrix * ButtonModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, ButtonIndex / 255.0f);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[ButtonIndex], GL_UNSIGNED_SHORT, 0);

		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);
	
	if (gPickedIndex == 255){ // Full white, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		switch (gPickedIndex) {
			case 2:
				oss << "Base";
				deselectObjectIndicies();
				keyMode = 5;
				BaseIndex = 9;
				break;
			case 3:
				oss << "Arm1";
				deselectObjectIndicies();
				keyMode = 1;
				Arm1Index = 10;
				break;
			case 4:
				oss << "Arm2";
				deselectObjectIndicies();
				keyMode = 2;
				Arm2Index = 11;
				break;
			case 5:
				oss << "Button";
				deselectObjectIndicies();
				ButtonIndex = 12;
				break;
			case 6:
				oss << "Joint";
				deselectObjectIndicies();
				JointIndex = 13;
				break;
			case 7:
				oss << "Pen";
				deselectObjectIndicies();
				keyMode = 4;
				PenIndex = 14;
				break;
			case 8:
				oss << "Top";
				deselectObjectIndicies();
				keyMode = 6;
				TopIndex = 15;
				break;
			case 9:
				oss << "Base";
				break;
			case 10:
				oss << "Arm1";
				break;
			case 11:
				oss << "Arm2";
				break;
			case 12:
				oss << "Button";
				break;
			case 13:
				oss << "Joint";
				break;
			case 14:
				oss << "Pen";
				break;
			case 15:
				oss << "Top";
				break;
			default:
				oss << "point " << gPickedIndex;
		}
		gMessage = oss.str();
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, wTitle, NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void)
{

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");
	
	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	LightID2 = glGetUniformLocation(programID, "LightPosition_worldspace2");

	createObjects();
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset); 
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
			);
	}
}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}


static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Keypress Actions
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_1:
			deselectObjectIndicies();
			if (keyMode != 1) {
				keyMode = 1;
				Arm1Index = 10;
				printf("Arm1 is selected\n");
			}
			else {
				keyMode = 0;
				printf("Arm1 is deselected\n");
			}
			break;
		case GLFW_KEY_2:
			deselectObjectIndicies();
			if (keyMode != 2) {
				keyMode = 2;
				Arm2Index = 11;
				printf("Arm2 is selected\n");
			}
			else {
				keyMode = 0;
				printf("Arm2 is deselected\n");
			}
			break;
		case GLFW_KEY_C:
			deselectObjectIndicies();
			if (keyMode != 3) {
				keyMode = 3;
				printf("Camera is selected\n");
			}
			else {
				keyMode = 0;
				printf("Camera is deselected\n");
			}
			break;
		case GLFW_KEY_P:
			deselectObjectIndicies();
			if (keyMode != 4) {
				keyMode = 4;
				PenIndex = 14;
				printf("Pen is selected\n");
			}
			else {
				keyMode = 0;
				printf("Pen is deselected\n");
			}
			break;
		case GLFW_KEY_B:
			deselectObjectIndicies();
			if (keyMode != 5) {
				keyMode = 5;
				BaseIndex = 9;
				printf("Base is selected\n");
			}
			else {
				keyMode = 0;
				printf("Base is deselected\n");
			}
			break;
		case GLFW_KEY_T:
			deselectObjectIndicies();
			if (keyMode != 6) {
				keyMode = 6;
				TopIndex = 15;
				printf("Top is selected\n");
			}
			else {
				keyMode = 0;
				printf("Top is deselected\n");
			}
			break;
		case GLFW_KEY_LEFT:
			printf("Left arrow key pressed\n");
			rotationDirection = 1;
			break;
		case GLFW_KEY_RIGHT:
			printf("Right arrow key pressed\n");
			rotationDirection = 2;
			break;
		case GLFW_KEY_UP:
			printf("Up arrow key pressed\n");
			rotationDirection = 3;
			break;
		case GLFW_KEY_DOWN:
			printf("Down arrow key pressed\n");
			rotationDirection = 4;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			shiftPressed = 1;
			printf("Left shift key pressed\n");
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			shiftPressed = 1;
			printf("Right shift key pressed\n");
			break;
		default:
			break;
		}
	}

	// Release Actions
	if (action == GLFW_RELEASE) {

		switch (key) {
			case GLFW_KEY_LEFT_SHIFT:
			case GLFW_KEY_RIGHT_SHIFT:
				shiftPressed = 0;
				break;
			default:
				rotationDirection = 0;
				break;
		}
		

	}
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
}

glm::vec3 setLookat() {

	// Rotation matrix about the X axis, up and down
	glm::mat4 RotationX = { 
							{ 1.0, 0.0, 0.0, 0.0 },
							{ 0.0, cos(thetaY), -sin(thetaY), 0.0 },
							{ 0.0, sin(thetaY), cos(thetaY), 0.0 },
							{ 0.0, 0.0, 0.0, 1.0 }
						  };

	// Rotation matrix about the Y axis, left and right
	glm::mat4 RotationY = {
							{ cos(thetaX), 0.0, sin(thetaX), 0.0 },
							{ 0.0, 1.0, 0.0, 0.0 },
							{ -sin(thetaX), 0.0, cos(thetaX), 0.0 },
							{ 0.0, 0.0, 0.0, 1.0 }
						};

	// vec multiplied by result of the rotation matricies gives us our vector of rotation
	glm::vec4 vec = { 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 rotation = RotationY * RotationX * vec;

	// Set output vector as scaled rotation calculation vector
	glm::vec3 output = { 10 * rotation[0], 10 * rotation[1], 10 * rotation[2] };

	return output;
}

void rotateCamera() {

	switch (rotationDirection) {
	case 1:				// Left
		thetaX += 0.05;
		break;
	case 2:				// Right
		thetaX -= 0.05;
		break;
	case 3:				// Up
		thetaY += 0.05;
		break;
	case 4:				// Down
		thetaY -= 0.05;
		break;
	default:			// Not started
		break;
	}

}

void rotateArm1Position() {

	switch (rotationDirection) {
	case 3:				// Up
		Arm1ZRotation < (2 * PI / 3) ? Arm1ZRotation += 0.05 : false;
		break;
	case 4:				// Down
		Arm1ZRotation > ( (-1) * PI / 4) ? Arm1ZRotation -= 0.05 : false;
		break;
	default:			// Not started
		break;
	}
}

void rotateArm2Position() {

	switch (rotationDirection) {
	case 3:				// Up
		Arm2ZRotation < ( PI ) ? Arm2ZRotation += 0.05 : false;
		break;
	case 4:				// Down
		Arm2ZRotation > ( (-1) * PI / 3 ) ? Arm2ZRotation -= 0.05 : false;
		break;
	default:			// Not started
		break;
	}
}

void rotateTopPosition() {

	switch (rotationDirection) {
	case 1:				// Left
		TopYRotation += 0.05;
		break;
	case 2:				// Right
		TopYRotation -= 0.05;
		break;
	default:			// Not started
		break;
	}
}

void translateBasePosition() {

	switch (rotationDirection) {
	case 1:				// Left
		BaseZPosition < (5) ? BaseZPosition += 0.1 : false;
		break;
	case 2:				// Right
		BaseZPosition > ((-1) * 5) ? BaseZPosition -= 0.1 : false;
		break;
	case 3:				// Up
		BaseXPosition > ((-1) * 5) ? BaseXPosition -= 0.1 : false;
		break;
	case 4:				// Down
		BaseXPosition < (5) ? BaseXPosition += 0.1 : false;
		break;
	default:			// Not started
		break;
	}
}

void rotatePenPosition() {

	switch (rotationDirection) {
	case 1:				// Left
		if (shiftPressed) {
			PenYRotation < ( PI / 2) ? PenYRotation += 0.05 : false;
		}
		else {
			PenXRotation < (PI / 3) ? PenXRotation += 0.05 : false;
		}
		break;
	case 2:				// Right
		if (shiftPressed) {
			PenYRotation > ((-1) * PI / 2) ? PenYRotation -= 0.05 : false;
		}
		else {
			PenXRotation > ((-1) * PI / 3) ? PenXRotation -= 0.05 : false;
		}
		break;
	case 3:				// Up
		PenZRotation < ( PI / 4) ? PenZRotation += 0.05 : false;
		break;
	case 4:				// Down
		PenZRotation > ((-1) * PI / 4) ? PenZRotation -= 0.05 : false;
		break;
	default:			// Not started
		break;
	}

}


int main(void)
{
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		//// Measure speed
		//double currentTime = glfwGetTime();
		//nbFrames++;
		//if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
		//	// printf and reset
		//	printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		//	nbFrames = 0;
		//	lastTime += 1.0;
		//}
		
		if (animation){
			phi += 0.01;
			if (phi > 360)
				phi -= 360;
		}

		switch (keyMode) {
			case 0:
				break;
			case 1:		// Arm1
				rotateArm1Position();
				break;
			case 2:		// Arm2
				rotateArm2Position();
				break;
			case 3:		// Camera
				rotateCamera();
				break;
			case 4:		// Pen
				rotatePenPosition();
				break;
			case 5:		// Base
				translateBasePosition();
				break;
			case 6:		// Top
				rotateTopPosition();
				break;
		}

		// DRAWING POINTS
		renderScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}