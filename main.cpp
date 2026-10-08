#define _CRT_SECURE_NO_WARNINGS

// STANDARD AND OPENGL LIBRARY INCLUDES
#include <stdio.h>
#include <stdlib.h>

// MULTIPLATFORM OPENGL/GLUT CONFIGURATION
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	//If you follow the setup instructions provided for Visual Studio
	//The include path specified already includes "GL", so we can ignore it
#include <GL/glew.h>
#include <GL/freeglut.h>
#elif defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <GL/glew.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

// PROJECT INCLUDES
#include "camera.h"	
#include "shaders.h"
#include "mesh.h"
#include "math.h"

// GLOBAL CAMERA STATE (Position, Rotation, FOV, Near plane, Far plane)
Camera cam = { { 0.0f, 1.0f, 8.0f }, { 0.0f, 0.0f, 0.0f }, 60, 1, 1000.0f };

// WINDOW DIMENSIONS
int screen_width = 1024;
int screen_height = 768;

// GLOBAL POINTER TO TRIANGLE MESH LIST (Legacy)
Mesh* meshList = NULL;

// SHADER CONFIGURATION AND WINDOW STATE
const int MAX_SHADERS = 10;
GLuint shprg[MAX_SHADERS]; // Shader program id
int activeShader = 2;
int windowHandle;

// GLOBAL VARIABLES FOR LOGIC AND ANIMATION
const float PI = 3.14159265359f;
bool isPerspective = true;
float knotRotationY = 0.0f;
bool rotation_knot = true;
bool isFirstPerson = false;
bool carLights = false;

// CONSTANTS FOR RENDERING LIMITS AND SCENE CONFIGURATION
const int CONSTANT_OF_RENDERING = 5;
const float ROAD_LENGTH = 20.0f;
const float SPEED_BASED_ON_FRAMES = 0.5f;
const float LAMP_LIGHT_Y = 4.0f;
const float LAMP_X = 4.0f;
const float LAMP_LIGHT_X = 1.0f;
const float CAR_LIGHT_Y = -0.2f;
const float CAR_LIGHT_Z = 0.0f;
const float CAR_LIGHT_STEP_X = 0.3f;

// POINTERS TO LOADED OBJ MODELS
struct obj* car;
struct obj* road;
struct obj* lamp;
struct obj* desert;
struct obj* cubemap;

// OBJ LOADER LIBRARY INCLUDE
#include "lab3/obj-4.h"

// ABSOLUTE PATHS FOR OBJ FILES
const char* CAR_MODEL_ROUTE = "C:/Users/jaime/source/repos/DVA338_Lab3/DVA338_Lab2/lab3/Cadillac/cadillac.obj";
const char* ROAD_MODEL_ROUTE = "C:/Users/jaime/source/repos/DVA338_Lab3/DVA338_Lab2/lab3/RoadSkybox/road.obj";
const char* LAMP_MODEL_ROUTE = "C:/Users/jaime/source/repos/DVA338_Lab3/DVA338_Lab2/lab3/RoadSkybox/lamp.obj";
const char* DESERT_MODEL_ROUTE = "C:/Users/jaime/source/repos/DVA338_Lab3/DVA338_Lab2/lab3/RoadSkybox/desert.obj";
const char* CUBEMAP_MODEL_ROUTE = "C:/Users/jaime/source/repos/DVA338_Lab3/DVA338_Lab2/lab3/RoadSkybox/cubemap.obj";

// CAR INSTANCE STRUCTURE (Currently unused)
typedef struct {
	Vector pos;      //  T
	Vector rot;      //  R
	Vector scale;    //  S
	Vector step;     //  Mov/frame
} Car;

// const int NUM_CARS = 2000;
// Car cars[NUM_CARS];

// GLOBAL FRAME COUNTER FOR ANIMATION
int frame = 0;


// GLOBAL TRANSFORMATION MATRICES (View, Projection, and Combined)
Matrix V, P, PV;


// FUNCTION TO PRINT SHADER COMPILATION ERRORS
void printShaderErrorInfo(GLuint shaderProgram)
{
	GLint maxLength = 0;
	glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

	// The maxLength includes the NULL character
	GLchar* errorLog = (GLchar*)calloc(maxLength, sizeof(GLchar));
	glGetShaderInfoLog(shaderProgram, maxLength, &maxLength, &errorLog[0]);

	// Provide the infolog in whatever manor you deem best.
	printf("%s\n", errorLog);
	printf("\n");
	free(errorLog);
}


// FUNCTION TO READ TEXT FILES FROM DISK (Used for shaders)
char* readShaders(const char* fileName)
{
	if (fileName == NULL) exit(-1);

	// OPEN FILE AND GET POINTER
	FILE* file = fopen(fileName, "rb");
	// CHECK POINTER
	if (file == NULL)
	{
		printf("File not found\n");
		exit(-2);
	}

	// GO TO THE END
	fseek(file, 0, SEEK_END);
	// TELL POSITION
	long size = ftell(file);
	if (size <= 0) {
		printf("Buffer size error\n");
		fclose(file);
		exit(-3);
	}
	// GO TO BEGGINING
	rewind(file);

	size_t sizet = (size_t)size;

	// CREATE BUFFER 
	char* buffer = (char*)malloc(size + 1);
	// CHECK BUFFER
	if (buffer == NULL) {
		printf("Buffer error\n");
		fclose(file);
		exit(-4);
	}

	// READ FILE
	size_t bytesRead = fread(buffer, 1, size, file);
	buffer[bytesRead] = '\0';

	// CLOSE FILE
	fclose(file);
	return buffer;
}


void update(int value)
{
	frame++;

	if (rotation_knot) {
		knotRotationY += 2.0f;
		if (knotRotationY > 360.0f) knotRotationY -= 360.0f;
	}

	// REQUEST WINDOW REDRAW
	glutPostRedisplay();
	// CALL THIS FUNCTION AGAIN AFTER 20ms
	glutTimerFunc(20, update, 0);
}


// FUNCTION TO COMPILE AND LINK VERTEX AND FRAGMENT SHADERS INTO A PROGRAM
GLuint prepareShaderProgram(const char* vertName, const char* fragName)
{
	GLint success = GL_FALSE;

	GLuint shprgLocal = glCreateProgram();

	char* vertCode = readShaders(vertName);
	char* fragCode = readShaders(fragName);

	// CREATE AND COMPILE VERTEX SHADER
	GLuint vs;
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertCode, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	if (!success) printf("Error in vertex shader!\n");
	else printf("Vertex shader compiled successfully!\n");
	if (success == GL_FALSE)
	{
		printShaderErrorInfo(vs);
		glDeleteShader(vs); // Don't leak the shader.
	}

	// CREATE AND COMPILE FRAGMENT SHADER
	GLuint fs;
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragCode, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if (!success) printf("Error in fragment shader!\n");
	else printf("Fragment shader compiled successfully!\n");
	if (success == GL_FALSE)
	{
		printShaderErrorInfo(fs);
		glDeleteShader(fs); // Don't leak the shader.
	}

	glAttachShader(shprgLocal, vs);
	glAttachShader(shprgLocal, fs);

	// BINDS MEMORY POSITIONS IN GPU FOR NO DATA LOSS
	glBindAttribLocation(shprgLocal, 0, "vPos");
	glBindAttribLocation(shprgLocal, 1, "vNorm");

	// LINK FINAL PROGRAM AND CHECK FOR ERRORS
	glLinkProgram(shprgLocal);
	GLint isLinked = GL_FALSE;
	glGetProgramiv(shprgLocal, GL_LINK_STATUS, &isLinked);
	if (!isLinked) printf("Link error in shader program!\n");
	else printf("Shader program linked successfully!\n");

	return shprgLocal;
}


void changeSize(int w, int h)
{
	screen_width = w;
	screen_height = h;
	glViewport(0, 0, screen_width, screen_height);
}


// FUNCTION TO BIND SHADER VARIABLES WITH OBJ MODEL PROPERTIES
void bindCurrentShaderLocations(GLuint program)
{
	// GET ATTRIBUTE LOCATIONS
	GLint vPosP = glGetAttribLocation(program, "vPos");
	GLint vNormP = glGetAttribLocation(program, "vNorm");
	GLint vTexCoordP = glGetAttribLocation(program, "vTexCoord");

	// GET UNIFORM LOCATIONS (Material properties)
	GLint locAmbientColorP = glGetUniformLocation(program, "ambientColor");
	GLint locDiffuseColorP = glGetUniformLocation(program, "diffuseColor");
	GLint locSpecularColorP = glGetUniformLocation(program, "specularColor");
	GLint locTextureMapP = glGetUniformLocation(program, "textureMap");

	// APPLY LOCATIONS TO EACH LOADED MODEL
	if (car) {
		obj_set_vert_loc(car, -1, vNormP, vTexCoordP, vPosP);
		obj_set_prop_loc(car, OBJ_KA, locAmbientColorP, -1, -1);
		obj_set_prop_loc(car, OBJ_KD, locDiffuseColorP, locTextureMapP, -1);
		obj_set_prop_loc(car, OBJ_KS, locSpecularColorP, -1, -1);
	}
	if (road) {
		obj_set_vert_loc(road, -1, vNormP, vTexCoordP, vPosP);
		obj_set_prop_loc(road, OBJ_KA, locAmbientColorP, -1, -1);
		obj_set_prop_loc(road, OBJ_KD, locDiffuseColorP, locTextureMapP, -1);
		obj_set_prop_loc(road, OBJ_KS, locSpecularColorP, -1, -1);
	}
	if (lamp) {
		obj_set_vert_loc(lamp, -1, vNormP, vTexCoordP, vPosP);
		obj_set_prop_loc(lamp, OBJ_KA, locAmbientColorP, -1, -1);
		obj_set_prop_loc(lamp, OBJ_KD, locDiffuseColorP, locTextureMapP, -1);
		obj_set_prop_loc(lamp, OBJ_KS, locSpecularColorP, -1, -1);
	}
	if (desert) {
		obj_set_vert_loc(desert, -1, vNormP, vTexCoordP, vPosP);
		obj_set_prop_loc(desert, OBJ_KA, locAmbientColorP, -1, -1);
		obj_set_prop_loc(desert, OBJ_KD, locDiffuseColorP, locTextureMapP, -1);
		obj_set_prop_loc(desert, OBJ_KS, locSpecularColorP, -1, -1);
	}
	if (cubemap) {
		obj_set_vert_loc(cubemap, -1, vNormP, vTexCoordP, vPosP);
		obj_set_prop_loc(cubemap, OBJ_KA, locAmbientColorP, -1, -1);
		obj_set_prop_loc(cubemap, OBJ_KD, locDiffuseColorP, locTextureMapP, -1);
		obj_set_prop_loc(cubemap, OBJ_KS, locSpecularColorP, -1, -1);
	}
}


// CALLBACK FUNCTION TO HANDLE KEYBOARD EVENTS
void keypress(unsigned char key, int x, int y)
{
	switch (key) {
		// SHADER SELECTION
	case '1':
		activeShader = 0;
		bindCurrentShaderLocations(shprg[activeShader]);
		printf("Using Shader: Basic\n");
		break;
	case '2':
		activeShader = 1;
		bindCurrentShaderLocations(shprg[activeShader]);
		printf("Using Shader: Interlaced\n");
		break;
	case '3':
		activeShader = 2;
		bindCurrentShaderLocations(shprg[activeShader]);
		printf("Using Shader: Phong\n");
		break;
	case '4':
		activeShader = 3;
		bindCurrentShaderLocations(shprg[activeShader]);
		printf("Using Shader: Cartoon\n");
		break;

		// ANIMATION AND PROJECTION CONTROLS
	case 'p':
		rotation_knot = !rotation_knot;
		break;
	case 'P':
		isPerspective = !isPerspective;
		if (isPerspective) printf("Projection: Perspective\n");
		else printf("Projection: Orthographic\n");
		break;

		// CAMERA CONTROLS (FOV and Linear Movement)
	case 'f':
		cam.fov -= 2.0f;
		printf("FOV: %f\n", cam.fov);
		break;
	case 'F':
		cam.fov += 2.0f;
		printf("FOV: %f\n", cam.fov);
		break;
	case 'z':
		cam.position.z -= 0.2f;
		break;
	case 'Z':
		cam.position.z += 0.2f;
		break;
	case 'x':
		cam.position.x -= 0.2f;
		break;
	case 'X':
		cam.position.x += 0.2f;
		break;
	case 'y':
		cam.position.y -= 0.2f;
		break;
	case 'Y':
		cam.position.y += 0.2f;
		break;

		// CAMERA CONTROLS (Rotation)
	case 'i':
		cam.rotation.x -= 2.0f;
		break;
	case 'I':
		cam.rotation.x += 2.0f;
		break;
	case 'j':
		cam.rotation.y -= 2.0f;
		break;
	case 'J':
		cam.rotation.y += 2.0f;
		break;
	case 'k':
		cam.rotation.z -= 2.0f;
		break;
	case 'K':
		cam.rotation.z += 2.0f;
		break;

		// SWITCH BETWEEN 1ST PERSON (Driver) AND 3RD PERSON VIEW
	case 'c':
	case 'C':
		isFirstPerson = !isFirstPerson;
		if (isFirstPerson) {
			cam.position = { -0.3f, -0.1f, 0.4f };
			cam.rotation = { 0.0f, 0.0f, 0.0f };
		}
		else {
			cam.position = { 0.0f, 1.0f, 8.0f };
			cam.rotation = { 0.0f, 0.0f, 0.0f };
		}
		break;

		// TOGGLE CAR LIGHTS ON/OFF
	case 'l':
	case 'L':
		carLights = !carLights;
		break;

		// EXIT PROGRAM
	case 'Q':
	case 'q':
#ifdef __APPLE__
		// MAC OS GLUT does not implement LeaveMainLoop().
		cleanUp();
		exit(0);
#else
		glutLeaveMainLoop();
#endif
		break;
	}

	glutPostRedisplay();
}


void cleanUp(void)
{
	printf("Running cleanUp function... ");
	// Free openGL resources
	// ...
	glutDestroyWindow(windowHandle);

	// Free meshes
	// ...

	printf("Done!\n\n");
}


// MAIN LOOP CALLED FOR EVERY FRAME
void display(void)
{
	// CLEAR COLOR AND DEPTH BUFFERS
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// CALCULATE VIEW MATRIX (Based on camera position and rotation)
	V = ViewMatrix(cam.position, cam.rotation);

	// CALCULATE PROJECTION MATRIX (Perspective or Orthographic)
	if (isPerspective) {
		P = PerspectiveProjectionMatrix(cam, screen_width, screen_height);
	}
	else {
		P = OrthographicMatrix(-20.0f, 20.0f, -10.0f, 10.0f, 1.0f, 1000.0f);
	}

	// COMBINED MATRIX (Projection x View) AND ACTIVATE SHADER PROGRAM
	PV = MatMatMul(P, V);
	GLuint program = shprg[activeShader];
	glUseProgram(program);


	// --- SEND GLOBAL DATA TO SHADER (Lighting and Camera) ---

	// CAMERA
	glUniform3f(glGetUniformLocation(program, "viewPos"), cam.position.x, cam.position.y, cam.position.z);

	// LIGHTS
	glUniform3f(glGetUniformLocation(program, "lightPosition"), -20.0f, 10.0f, 10.0f);
	glUniform3f(glGetUniformLocation(program, "ambientLight"), 0.1f, 0.1f, 0.1f);
	glUniform3f(glGetUniformLocation(program, "diffuseLight"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(program, "specularLight"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(program, "shininess"), 64.0f);

	// CAR LIGHTS
	float carLightsPositions[6] = {
		-CAR_LIGHT_STEP_X, CAR_LIGHT_Y, CAR_LIGHT_Z,
		 CAR_LIGHT_STEP_X, CAR_LIGHT_Y, CAR_LIGHT_Z
	};

	glUniform1i(glGetUniformLocation(program, "carLightsOn"), carLights ? 1 : 0);
	glUniform3fv(glGetUniformLocation(program, "carLightsPos"), 2, carLightsPositions);
	glUniform3f(glGetUniformLocation(program, "carLightsDir"), 0.0f, 0.0f, -1.0f);
	glUniform1f(glGetUniformLocation(program, "inner"), cosf(12.5f * PI / 180.0f));
	glUniform1f(glGetUniformLocation(program, "outer"), cosf(17.5f * PI / 180.0f));
	glUniform3f(glGetUniformLocation(program, "carLightsDiffuse"), 1.0f, 0.95f, 0.8f);
	glUniform3f(glGetUniformLocation(program, "carLightsSpecular"), 1.0f, 0.95f, 0.8f);
	glUniform1f(glGetUniformLocation(program, "Constant"), 1.0f);
	glUniform1f(glGetUniformLocation(program, "Linear"), 0.09f);
	glUniform1f(glGetUniformLocation(program, "Quadratic"), 0.032f);

	// POINTVIEW AND WORLD MATRIX
	GLint loc_PV = glGetUniformLocation(program, "PV");
	glUniformMatrix4fv(loc_PV, 1, GL_FALSE, PV.e);
	GLint loc_W = glGetUniformLocation(program, "W");

	// --- 1. DRAW THE SKYBOX (Which functions as an immense floor) ---
	glDisable(GL_DEPTH_TEST); 
	glDisable(GL_CULL_FACE); // TO SEE INNER WALLS

	Matrix S_sky = ScalingMatrix({ 1000.0, 1000.0f, 1000.0f });
	Matrix T_sky = TranslationMatrix({ cam.position.x, cam.position.y, cam.position.z });
	Matrix W_sky = MatMatMul(T_sky, S_sky);

	glUniformMatrix4fv(loc_W, 1, GL_FALSE, W_sky.e);
	obj_render(cubemap);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// --- 2. PREPARE AND DRAW THE ROAD (Infinite Scroll Effect via Tile Snapping) ---

	// DEFINITION OF SCROLLABLE TILES (A, B, C...)
	const int NUM_TILES = 11; 
	const float TILE_LENGTH = 20.0f; 
	const float TOTAL_LENGTH = NUM_TILES * TILE_LENGTH; 

	const float FRONT_PLANE_Z = 15.0f;

	Vector lampPositionsArray[30];
	int numLamps = 0;

	float global_movement = frame * SPEED_BASED_ON_FRAMES;

	// STEP A: GATHER ALL LIGHT POSITIONS FIRST (Lamp spotlights)
	for (int i = 0; i < NUM_TILES; i++)
	{
		// 1. CALCULATE BASE POSITION 
		float base_z = i * TILE_LENGTH;

		// 2. APPLY MOVEMENT AND WRAP 
		float current_z = fmod(base_z + global_movement, TOTAL_LENGTH);

		// 3. SHIFT SO IT SPAWNS IN THE DISTANCE (-Z) 
		float positionZ = current_z - (TOTAL_LENGTH - FRONT_PLANE_Z);

		// STORE POSITIONS
		lampPositionsArray[numLamps++] = { LAMP_LIGHT_X, LAMP_LIGHT_Y, positionZ };
		lampPositionsArray[numLamps++] = { -LAMP_LIGHT_X, LAMP_LIGHT_Y, positionZ };
	}

	// STEP B: SEND THE ENTIRE LIGHT ARRAY TO THE SHADER AT ONCE
	glUniform1i(glGetUniformLocation(program, "numLamps"), numLamps);
	glUniform3fv(glGetUniformLocation(program, "lampPositions"), numLamps, (float*)lampPositionsArray);
	glUniform3f(glGetUniformLocation(program, "lampColor"), 1.0f, 0.8f, 0.5f);
	glUniform3f(glGetUniformLocation(program, "lampDir"), 0.0f, -1.0f, 0.0f);
	glUniform1f(glGetUniformLocation(program, "lampInner"), cosf(25.0f * PI / 180.0f));
	glUniform1f(glGetUniformLocation(program, "lampOuter"), cosf(35.0f * PI / 180.0f));

	// STEP C: NOW, RENDER THE PHYSICAL MODELS (Road and Lamps)
	for (int i = 0; i < NUM_TILES; i++)
	{
		// EXACT SAME CALCULATION FOR POSITION Z
		float base_z = i * TILE_LENGTH;
		float current_z = fmod(base_z + global_movement, TOTAL_LENGTH);
		float positionZ = current_z - (TOTAL_LENGTH - FRONT_PLANE_Z);

		// Draw Road
		Matrix S_road = ScalingMatrix({ 1.0f, 1.0f, 1.0f });
		Matrix T_road = TranslationMatrix({ 0.0f, -1.0f, positionZ });
		Matrix W_road = MatMatMul(T_road, S_road);

		glUniformMatrix4fv(loc_W, 1, GL_FALSE, W_road.e);
		obj_render(road);

		// Draw Right Lamp
		Matrix S_lamp = ScalingMatrix({ 1.0f, 1.0f, 1.0f });
		Matrix T_lamp = TranslationMatrix({ LAMP_X, -1.0f, positionZ });
		Matrix W_lamp = MatMatMul(T_lamp, S_lamp);

		glUniformMatrix4fv(loc_W, 1, GL_FALSE, W_lamp.e);
		obj_render(lamp);

		// Draw Left Lamp (Symmetric, requires rotation)
		Matrix S_lamp_sim = ScalingMatrix({ 1.0f, 1.0f, 1.0f });
		Matrix R_lamp_sim = RotationMatrixY(180.0f * (PI / 180.0f));
		Matrix T_lamp_sim = TranslationMatrix({ -LAMP_X, -1.0f, positionZ });

		Matrix RS_lamp_sim = MatMatMul(R_lamp_sim, S_lamp_sim);
		Matrix W_lamp_sim = MatMatMul(T_lamp_sim, RS_lamp_sim);

		glUniformMatrix4fv(loc_W, 1, GL_FALSE, W_lamp_sim.e);
		obj_render(lamp);
	}


	// --- 3. DRAW THE MAIN CAR ---
	Matrix S_car = ScalingMatrix({ 0.6f, 0.6f, 0.6f });
	Matrix Rx_car = RotationMatrixX(-90.0f * (PI / 180.0f));
	Matrix Ry_car = RotationMatrixY(0.0f * (PI / 180.0f));
	Matrix T_car = TranslationMatrix({ 0.0f, -1.0f, 0.0f });

	// Multiply Scale -> RotateX -> RotateY -> Translate
	Matrix tmp1 = MatMatMul(Rx_car, S_car);
	Matrix tmp2 = MatMatMul(Ry_car, tmp1);
	Matrix W_car = MatMatMul(T_car, tmp2);

	glUniformMatrix4fv(loc_W, 1, GL_FALSE, W_car.e);
	obj_render(car);

	// FORCE EXECUTION OF OPENGL COMMANDS
	glFlush();
}


// FUNCTION TO LOAD RESOURCES AND CONFIGURE INITIAL STATES
void init(void)
{
	// Compile and link the given shader program (vertex shader and fragment shader)
	shprg[0] = prepareShaderProgram("shaders/simple.vert.txt", "shaders/simple.frag.txt");
	shprg[1] = prepareShaderProgram("InterlacedVertexShader.glsl", "InterlacedFragShader.glsl");
	shprg[2] = prepareShaderProgram("shaders/phong.vert.glsl", "shaders/phong.frag.glsl");
	shprg[3] = prepareShaderProgram("shaders/cartoon.vert.glsl", "shaders/cartoon.frag.glsl");

	GLuint program = shprg[activeShader];

	// CREATE OBJECT (Instantiate memory for models)
	car = obj_create(CAR_MODEL_ROUTE);
	road = obj_create(ROAD_MODEL_ROUTE);
	lamp = obj_create(LAMP_MODEL_ROUTE);
	desert = obj_create(DESERT_MODEL_ROUTE);
	cubemap = obj_create(CUBEMAP_MODEL_ROUTE);

	// CHECK FOR SUCCESSFUL .OBJ FILE LOADING
	if (car == NULL) {
		printf("Error loading model car\n");
		exit(-5);
	}
	if (road == NULL) {
		printf("Error loading model road\n");
		exit(-6);
	}
	if (lamp == NULL) {
		printf("Error loading model lamp\n");
		exit(-7);
	}
	if (desert == NULL) {
		printf("Error loading model desert\n");
		exit(-8);
	}
	if (cubemap == NULL) {
		printf("Error loading model cubemap\n");
		exit(-9);
	}

	// PROCESS OBJECT (Internal preprocessing of buffers, VBOs, and VAOs by the obj library)
	obj_proc(car);
	obj_proc(road);
	obj_proc(lamp);
	obj_proc(desert);
	obj_proc(cubemap);

	// BIND CURRENT SHADER LOCATIONS TO THE NEWLY CREATED MODELS
	bindCurrentShaderLocations(program);
}


int main(int argc, char** argv)
{
	unsigned int _glut_mode = GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH;
#ifdef __APPLE__
	_glut_mode |= GLUT_3_2_CORE_PROFILE;
#endif

	glutInitDisplayMode(_glut_mode);
	glutInit(&argc, argv);
	glutInitWindowSize(screen_width, screen_height);
	windowHandle = glutCreateWindow("DVA338 Programming Assignments");

	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keypress);

#ifdef GLUT_CORE_PROFILE 
	glutInitContextVersion(4, 1);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
#endif

	glewExperimental = GL_TRUE;

	GLenum err = glewInit();
	if (GLEW_OK != err) { fprintf(stdout, "Error: %s\n", glewGetErrorString(err)); return 1; }

	// Output OpenGL version info
	fprintf(stdout, "GLEW version: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, "OpenGL version: %s\n", (const char*)glGetString(GL_VERSION));
	fprintf(stdout, "OpenGL vendor: %s\n\n", glGetString(GL_VENDOR));

	// CALL CUSTOM INITIALIZATION (Models, Shaders, Variables)
	init();

	// START TIMER LOOP AND WINDOW EVENT PROCESSING
	glutTimerFunc(16, update, 0); 
	glutMainLoop();

	cleanUp();
	return 0;
}