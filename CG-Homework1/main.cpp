/*
Type your name and student ID here
	- Name: Huangxj
	- Student ID: 202201130304
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/glfw/glfw3.h"

#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"

#include <iostream>
#include <fstream>

#include <vector>
#include <cmath>

// 创建球形
std::vector<float> vertices;
std::vector<unsigned int> indices;
int stacks = 32, slices = 32;
float radius = 1.0f;

GLint programID;
float x_delta = 0.1f;
float y_delta = 0.1f;
int x_press_num = 0;
int y_press_num = 0;
int z_press_num = 0;
float rotationAngle = 0.0f;
const float rotationSpeed = glm::radians(50.0f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float cameraDistance = 5.0f;

glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;

unsigned int VAO[10], VBO[10], EBO[10];

void get_OpenGL_info() {
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		std::cout << buffer << std::endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID) {
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID) {
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

std::string readShaderCode(const char* fileName) {
	std::ifstream meInput(fileName);
	if (!meInput.good()) {
		std::cout << "File failed to load ... " << fileName << std::endl;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders() {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	//adapter[0] = vertexShaderCode;
	std::string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	//adapter[0] = fragmentShaderCode;
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}

void sendDataToOpenGL() {
	// TODO:
	// create 2D objects and 3D objects and/or lines (points) here and bind to VAOs & VBOs
	// 1. 矩形
	const GLfloat square0[] = {
		-0.5f, -0.5f, +0.0f,  // left
		+0.0f, +1.0f, +0.0f,  // color

		+0.5f, -0.5f, +0.0f,  // right
		+1.0f, +0.0f, +0.0f,

		+0.5f, +0.5f, +0.0f,  // top
		+1.0f, +0.0f, +0.0f,

		-0.5f, +0.5f, +0.0f,  // top
		+1.0f, +0.0f, +0.0f,
	};

	GLuint index0[] = {
		0, 1, 2, // 第一个三角形
		0, 2, 3  // 第二个三角形
	};

	glGenVertexArrays(1, &VAO[0]);
	glGenBuffers(1, &VBO[0]);
	glGenBuffers(1, &EBO[0]);

	// square0
	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square0), square0, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (char*)(3 * sizeof(float)));
	// 该对象包含ebo(索引)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index0), index0, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// 2. 正方体
	const GLfloat cube0[] = {
		// 前面
		-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // 左下
		 0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  // 右下
		 0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // 右上

		-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // 左下
		 0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // 右上
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  // 左上
		// 后面
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,

		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
		// 左面
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
		// 右面
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
		 // 上面
		 -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		  0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
		  0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

		 -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		  0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
		 -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
		 // 下面
		 -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		  0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
		  0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

		 -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		  0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
		 -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &VAO[1]);
	glGenBuffers(1, &VBO[1]);

	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube0), cube0, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (char*)(3 * sizeof(float)));

	glBindVertexArray(0);

	// 3. 钻石
	const GLfloat diamond[] = {
		// 顶部顶点
		0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // 0: 顶点（上）

		// 中间4个顶点
		-0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f,  // 1: 前左
		0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f,  // 2: 前右
		0.5f, 0.0f, -0.5f, 1.0f, 1.0f, 0.0f,  // 3: 后右
		-0.5f, 0.0f, -0.5f, 1.0f, 0.0f, 1.0f,  // 4: 后左

		// 底部顶点
		0.0f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f   // 5: 顶点（下）
	};

	GLuint diamondIndex[] = {
		0, 1, 2,
		0, 2, 3,
		0, 3, 4,
		0, 4, 1,
		5, 1, 2,
		5, 2, 3,
		5, 3, 4,
		5, 4, 1
	};

	glGenVertexArrays(1, &VAO[2]);
	glGenBuffers(1, &VBO[2]);
	glGenBuffers(1, &EBO[1]);

	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(diamond), diamond, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(diamondIndex), diamondIndex, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (char*)(3 * sizeof(float)));

	glBindVertexArray(0);

	// 4. 球体
	for (int i = 0; i <= stacks; ++i) {
		float phi = glm::pi<float>() * float(i) / float(stacks);
		for (int j = 0; j <= slices; ++j) {
			float theta = 2.0f * glm::pi<float>() * float(j) / float(slices);
			float x = radius * sin(phi) * cos(theta);
			float y = radius * sin(phi) * sin(theta);
			float z = radius * cos(phi);
			float u = float(j) / float(slices); // 纹理坐标
			float v = float(i) / float(stacks);
			// 顶点位置
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
			// 法线方向（单位球体）
			vertices.push_back(x / radius);
			vertices.push_back(y / radius);
			vertices.push_back(z / radius);
			// 纹理坐标
			vertices.push_back(u);
			vertices.push_back(v);
		}
	}
	// 生成索引数据（两两三角形）
	for (int i = 0; i < stacks; ++i) {
		for (int j = 0; j < slices; ++j) {
			int first = i * (slices + 1) + j;
			int second = first + slices + 1;

			indices.push_back(first);
			indices.push_back(second);
			indices.push_back(first + 1);

			indices.push_back(second);
			indices.push_back(second + 1);
			indices.push_back(first + 1);
		}
	}

	glGenVertexArrays(1, &VAO[3]);
	glGenBuffers(1, &VBO[3]);
	glGenBuffers(1, &EBO[2]);

	glBindVertexArray(VAO[3]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (char*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (char*)(6 * sizeof(float)));

	glBindVertexArray(0);
}

void paintGL(void) {
	// always run
	// TODO:
	// render your objects and control the transformation here
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST); // 启用深度测试，确保立方体正确显示

	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);

	// 1. 绘制矩形
	modelTransformMatrix = glm::translate(glm::mat4(1.0f),
		glm::vec3(x_delta * x_press_num, y_delta * y_press_num, 0.0f));
	modelTransformMatrix = glm::scale(modelTransformMatrix,
		glm::vec3(0.3f, 0.3f, 0.3f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix,
		rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	GLint modelLoc = glGetUniformLocation(programID, "modelTransformMatrix");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	glBindVertexArray(VAO[0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// 2. 绘制立方体
	modelTransformMatrix = glm::translate(glm::mat4(1.0f),
		glm::vec3(0.5f, 0.0f, 0.0f));
	modelTransformMatrix = glm::scale(modelTransformMatrix,
		glm::vec3(0.3f, 0.3f, 0.3f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix,
		glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix,
		glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix,
		glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix,
		(float)glfwGetTime(), glm::vec3(1.0f, 1.0f, 1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	glBindVertexArray(VAO[1]);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	// 3. 绘制钻石
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(0.0f, -0.5f, 0.0f)); // 位置
	modelTransformMatrix = glm::scale(modelTransformMatrix,
		glm::vec3(0.3f, 0.3f, 0.3f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, (float)glfwGetTime(),
		glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f))); // 绕Y轴
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	glBindVertexArray(VAO[2]);
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// 4. 绘制球体
	float timeValue = glfwGetTime(); // 获取运行时间
	float angularSpeed = 1.0f;       // 角速度（弧度/秒）
	float radius_sphere = 7.0f;             // 圆的半径
	float theta_sphere = angularSpeed * timeValue; // 计算当前角度
	// 计算新位置
	float x_sphere = radius_sphere * cos(theta_sphere);
	float z_sphere = radius_sphere * sin(theta_sphere);
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::scale(modelTransformMatrix,
		glm::vec3(0.3f, 0.3f, 0.3f)); // 缩放
	modelTransformMatrix = glm::translate(modelTransformMatrix, 
		glm::vec3(x_sphere, 0.0f, z_sphere)); // 平移
	modelTransformMatrix = glm::rotate(modelTransformMatrix,
		theta_sphere, glm::vec3(0.0f, 1.0f, 0.0f)); // 旋转
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	glBindVertexArray(VAO[3]);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// TODO:
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		x_press_num -= 1;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		x_press_num += 1;
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		y_press_num += 1;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		y_press_num -= 1;
	}

}
void initCamera(void) {
	projectionMatrix = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 50.0f);

	glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, cameraDistance);  // 摄像机位置
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);  // 观察目标
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);  // 上方向
	viewMatrix = glm::lookAt(cameraPos, cameraTarget, up);

	// 传递到着色器
	GLuint projLoc = glGetUniformLocation(programID, "projectionMatrix");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	GLuint viewLoc = glGetUniformLocation(programID, "viewMatrix");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
}

void updateCamera(void) {
	glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, cameraDistance);  // 摄像机位置
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);  // 观察目标
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);  // 上方向
	viewMatrix = glm::lookAt(cameraPos, cameraTarget, up);
	// 传递到着色器
	GLuint viewLoc = glGetUniformLocation(programID, "viewMatrix");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	cameraDistance -= yoffset * 0.2f;
	cameraDistance = glm::clamp(cameraDistance, 2.0f, 20.0f);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void initializedGL(void) {
	// run only once
	// TODO:
	sendDataToOpenGL();
	installShaders();
}

int main(int argc, char* argv[]) {
	GLFWwindow* window;

	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* do not allow resizing */
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1200, 1200, "Assignment 1", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback); // TODO
	// TODO: mouse callback, etc.

	/* Initialize the glew */
	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}
	get_OpenGL_info();
	initializedGL();

	initCamera(); // 初始化摄像机
	glfwSetScrollCallback(window, scroll_callback); // 绑定滚轮事件

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		paintGL();
		updateCamera(); // 更新摄像机

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			rotationAngle -= rotationSpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			rotationAngle += rotationSpeed * deltaTime;
		}

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
