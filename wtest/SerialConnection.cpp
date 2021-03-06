// SerialConnection.cpp : Defines the entry point for the console application.
//


#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <fob/rs232.h>

#include "BOX.h"
#include "auxiliar.h"


#include <GL/glew.h>
#include <GL/gl.h>
#define SOLVE_FGLUT_WARNING
#include <GL/freeglut.h> 
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include <fob/fob.h>
fob flockofbirds;

void renderObj();
void renderObj2();

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char *vname, const char *fname);
void initObj();
void destroy();


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
unsigned int vshader;
unsigned int fshader;
unsigned int program;

//Variables Uniform
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;
int uColorTex;
int uEmiTex;
//Atributos  
int inPos;
int inColor;
int inNormal;
int inTexCoord;
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
//Texturas  
unsigned int colorTexId;
unsigned int emiTexId;
unsigned int loadTex(const char *fileName);


//VAO  
unsigned int vao;
//VBOs que forman parte del objeto  
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

float fLargest = -1;

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);
glm::mat4	model2 = glm::mat4(1.0f);



#define WORDBYTE 256

typedef union un{
	short data;
	unsigned char binary[2];
}FOBdata;

const char *byte_to_binary(unsigned char x)
{
	static char b[17];
	b[0] = '\0';

	int z;
	for (z = 128; z > 0; z >>= 1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}

	return b;
}
/*
static void
unpack(unsigned char *buffer, short *output, int size)
{
	for (int i = 0, j = 0; i < size; i += 2) {
		//shift ls
		buffer[i] = buffer[i] << 1;

		output[j++] = ((buffer[i + 1] << 8) | buffer[i]) << 1;
	}
}
static float SCALE = 1.0 / 32767.0;
static glm::vec3 unpack_pos(unsigned char *buffer, int size)
{
	//position format 6 bytes of data
	
	//unpack data
	short unpacked[3];
	unpack(buffer, unpacked, 6);

	//not let the user access this data
	

	//scale and copy data
	//map x, y, z from birds to -y, -z, x
	//float pos_scale = m_flock.get_scale( ) * SCALE;
	float pos_scale = 36.0 * SCALE;
	glm::vec3 m_position = glm::vec3(
		static_cast<float>(-unpacked[1]) * pos_scale,
		static_cast<float>(-unpacked[2]) * pos_scale,
		static_cast<float>(unpacked[0]) * pos_scale

		);

	return m_position;
}*/

#ifndef WIN32
typedef char _TCHAR;
#endif


int  n,
cport_nr = 0,        /* /dev/ttyS0 (COM1 on windows) */
bdrate = 115200;       /* 9600 baud */
fob::bird_list birds;
int _tmain(int argc, _TCHAR* argv[])
{
	

	
	unsigned char readbyte;
	char mode[] = { '8', 'N', '1', 0 };

	int i = 2;




	/*if (RS232_OpenComport(cport_nr, bdrate, mode))
	{
		printf("Can not open comport\n");

		return(0);
	}
	
	/*n = RS232_SendByte(cport_nr, '?');
	printf("Result to %d", n);
	n = RS232_SendByte(cport_nr, 'G');
	printf("Result to %d", n);
	n = RS232_SendByte(cport_nr, 'V');
	printf("Result to %d", n);
	n = RS232_SendByte(cport_nr, 'F');
	printf("Result to %d", n);
	n = RS232_SendByte(cport_nr, '@');
	printf("Result to %d", n);*/

	//local vars
	fob::hemisphere hemisphere = fob::DOWN;

	//talk to flock
	flockofbirds.open("FOB", hemisphere);
	if (!flockofbirds) {
		
		std::cerr << "fatal: " << flockofbirds.get_error() << std::endl;
		flockofbirds.close();
		return 1;
	}

	//get a list of birds connected to the machine
	birds = flockofbirds.get_birds();

	//report how many birds are present
	std::cout << "number of birds: " << birds.size() << std::endl;

	//for each bird, set that we want position and button information
	for (unsigned int i = 0; i < birds.size(); ++i) {
		if (!birds[i]->set_mode(fob::POSITION |fob::ORIENTATION)) {
			std::cerr << "fatal: " << flockofbirds.get_error() << std::endl;
			flockofbirds.close();
			return 1;
		}
	}

	//birds configured, set the flock flying
	flockofbirds.fly();


	

	//shutdown the flock
	//flock.close();
	

	
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, NULL);
	initOGL();
	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag");
	initObj();

	glutMainLoop();

	flockofbirds.close();
	destroy();
	

	

	return(0);
}


//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv){

	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE); 

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas GLSL");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)  {
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	const GLubyte *oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	if (glewIsSupported("GL_EXT_texture_filter_anisotropic")) {

		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
		printf("App::Load > %.0fx Anistropic filtering supported.", fLargest);
	}


	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);



}
void initOGL(){
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 1000.0f);
	view = glm::mat4(1.0f);
	view[3].z = -25;

	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
}
void destroy(){
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	if (inPos != -1)
		glDeleteBuffers(1, &posVBO);
	if (inColor != -1)
		glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1)
		glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1)
		glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);

}


void initShader(const char *vname, const char *fname){

	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)  {   //Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		char *logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;   delete logString;
		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");

	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");


}
void initObj(){
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)  {
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float)* 3, cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}


	if (inColor != -1)  {
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float)* 3, cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	if (inNormal != -1)  {
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float)* 3, cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}


	if (inTexCoord != -1)  {
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float)* 2, cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeNTriangleIndex*sizeof(unsigned int)* 3, cubeTriangleIndex, GL_STATIC_DRAW);

	model = glm::mat4(1.0f);


}

GLuint loadShader(const char *fileName, GLenum type){
	unsigned int fileLen;
	char *source = loadStringFromFile(fileName, fileLen);
	//////////////////////////////////////////////  
	//Creación y compilación del Shader  
	GLuint shader;  shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar **)&source, (const GLint *)&fileLen);
	glCompileShader(shader);
	delete source;


	//Comprobamos que se compiló bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)  {
		//Calculamos una cadena de error 
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char *logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete logString;
		glDeleteShader(shader);
		exit(-1);
	}

	return shader;

}
unsigned int loadTex(const char *fileName){

	unsigned char *map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);
	if (!map)
	{
		std::cout << "Error cargando el fichero: " << fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)map);

	delete[] map;
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	return texId;
}

void renderFunc(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);
	//pintado del  objeto!!!!
	renderObj();
	renderObj2();
	glUseProgram(0);


	glutSwapBuffers();

}


void renderObj()
{
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));
	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));
	//Texturas  
	if (uColorTex != -1)  {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}
	if (uEmiTex != -1)  {
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}


	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

}


void renderObj2()
{
	glm::mat4 modelView = view * model2;
	glm::mat4 modelViewProj = proj * view * model2;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));
	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));
	//Texturas  
	if (uColorTex != -1)  {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}
	if (uEmiTex != -1)  {
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}


	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

}



void resizeFunc(int width, int height){
	glViewport(0, 0, width, height);
	glutPostRedisplay();
}
void idleFunc(){
	static unsigned char buf[WORDBYTE];
	unsigned char response[12];
	FOBdata Xcoord, Ycoord, Zcoord;
	FOBdata Xangle, Yangle, Zangle;



	//buf = (unsigned char *)malloc(sizeof(unsigned char)*WORDBYTE);
	
		/*n = RS232_PollComport(cport_nr, buf, WORDBYTE);
		//n = RS232_PollComport(cport_nr, &readbyte, 1);

		if (n > 0)
		{
			//buf[n] = 0;   
			/*int j = -1;
			for (int i = 0; i < n; i++)
			{

				if ((buf[n - i] >> 7) & 1)  // replace unreadable control-codes by dots 
				{
					//printf("%s -> ", byte_to_binary(buf[n - i]));
					j = 0;
					//buf[n - i] &= ~(1 << 7);


				}
				if (j >= 0){
					printf("%s -> \n", byte_to_binary(buf[n - i]));
					printf("%c -> \n", buf[n - i]);
					response[j] = buf[n - i];
					j++;
				}
				if (j == 6)
					break;


			}*/
			/*short Coordvector[3];
			unpack(response, Coordvector, 6);
			glm::vec3 position = unpack_pos(buf,1);
			model = glm::mat4(1.0f);
			model[3].x = position.x;
			model[3].y = position.y;
			model[3].z = position.z;*/

			/*printf("%f %f %f \n", 
				(Coordvector[0] * 36.0f) / 32768.0f,
				(Coordvector[1] * 36.0f) / 32768.0f,
				(Coordvector[2] * 36.0f) / 32768.0f);*/


			
			//*/
		//}
		
			

	/*
		    printf("%x %x -> ", response[1], response[0]);
			//printf("%s %s -> ", byte_to_binary(((short)response[1]) << 9), byte_to_binary(response[0] << 2));
			Xcoord.binary[0] = response[1] << 9;
			Xcoord.binary[1] = response[0] << 2;
			printf("Xcoord: %x\n", Xcoord.data);
			//printf("Xcoord%f \n", (Xcoord.data*36.0f) / 32768.0f);

			printf("%x %x -> ", response[3], response[2]);
			//printf("%s %s -> ", byte_to_binary(response[3]), byte_to_binary(response[2]));
			//printf("%s %s -> ", byte_to_binary(response[3] << 9), byte_to_binary(response[2] << 2));
			Ycoord.binary[0] = response[3] << 9;
			Ycoord.binary[1] = response[2] << 2;
			printf("Ycoord: %d\n", Ycoord.data);
			//printf("Ycoord %f \n", (Ycoord.data*36.0f) / 32768.0f);

			printf("%x %x -> ", response[5], response[4]);
			//printf("%s %s -> ", byte_to_binary(response[5]), byte_to_binary(response[4]));
			//printf("%s %s -> ", byte_to_binary(response[5] << 9), byte_to_binary(response[4] << 2));
			Zcoord.binary[0] = response[5] << 9;
			Zcoord.binary[1] = response[4] << 2;
			printf("Zcoord: %d\n", Zcoord.data);
			//printf("Zcoord %f \n", (Zcoord.data*36.0f) / 32768.0f);


			/*printf("%s %s -> ", byte_to_binary(response[1]), byte_to_binary(response[0  ]));
			Xangle.binary[0] = response[7] << 9;
			Xangle.binary[1] = response[6] << 2;
			//printf("Xangle: %d\n", Xangle.data);
			printf("CXangle%f \n", (Xangle.data*36.0f) / 32768.0f);


			//printf("%s %s -> ", byte_to_binary(response[3]), byte_to_binary(response[2]));
			Yangle.binary[0] = response[9] << 9;
			Yangle.binary[1] = response[8] << 2;
			//printf("Yangle: %d\n", Yangle.data);
			printf("Yangle %f \n", ( Yangle.data*36.0f) / 32768.0f);


			//printf("%s %s -> ", byte_to_binary(response[5]), byte_to_binary(response[4]));
			Zangle.binary[0] = response[11] << 9;
			Zangle.binary[1] = response[10] << 2;
			//printf("Zangle: %d\n", Zangle.data);
			printf("Zangle%f \n", (Zangle.data*36.0f) / 32768.0f);*/


			//printf("number of bytes: %d\n", n);
			//printf("received %i bytes: %x\n", n, buf);
			//printf("received %i bytes: %x\n", n, readbyte);

			/*model = glm::mat4(1.0f);
			model[3].x = Coordvector[0] * 36.0f / 32767.0f;
			model[3].y = Coordvector[1] * 36.0f / 32767.0f;
			model[3].z = Coordvector[2] * 36.0f / 32767.0f;
			//static float angle = 0.0f;
			//angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.01f;
			//model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));
			
		}*/

			//everything looks good, tell the user what's going on
		/*std::cerr << birds.size() << " birds opened on " << std::endl;
		std::cerr << "Reporting position/button information" << std::endl;
		std::cerr << "(Press CTRL-C to quit)" << std::endl;*/

		//report position and orientation for each bird
		//float pos[3];
		glm::vec3 pos;
		glm::vec3 angles;
		glm::mat4 matrix=glm::mat4(1.0f);;
		unsigned char button;
		std::string button_str;

		
			for (unsigned int i = 0; i < birds.size(); ++i) {
				//get position and button information from the bird
				//birds[i]->get_position(pos);
				//birds[i]->get_rawangles(angles);
				birds[i]->get_matrix(matrix);
				//button = birds[i]->get_buttons();

				//get a string to describe the button pressed
				//button_str = get_button_str(button);

				//report
				//std::cout << "x: " << pos[0] << "y:  " << pos[1]  "z :"<< pos[2] << std::endl;
				if (i==0)
				{
					/*model = glm::mat4(1.0f);
					model[3].x = pos[0];
					model[3].y = pos[1];
					model[3].z = pos[2];*/
					model = matrix;
				}
				else
				{
					model2 = glm::mat4(1.0f);
					model2[3].x = pos[0];
					model2[3].y = pos[1];
					model2[3].z = pos[2];
				}

			}

			
			
	

		//getchar();
			


			//model = glm::rotate(model,)
			

	//free(buf);
	glutPostRedisplay();

}
void keyboardFunc(unsigned char key, int x, int y){}
void mouseFunc(int button, int state, int x, int y){}



