
#define _CRT_SECURE_NO_WARNINGS
#define CRES 30 
#define M_PI 3.14159265358979323846 

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>   //Omogucava laksu upotrebu OpenGL naredbi
#include <GLFW/glfw3.h>//Olaksava pravljenje i otvaranje prozora (konteksta) sa OpenGL sadrzajem
#include <thread> // Za sleep_for


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


float angleSpeed = 0.003f;     // Brzina pomeranja ugla
float radiusX = -0.9f;          // Poluprečnik putanje (polukrug)
float radiusY = -0.9f;          // Poluprečnik putanje (polukrug)
float sunOffsetX = -0.9f;		// Početna pozicija X (donja leva ivica gornje polovine)
float sunOffsetY = -0.9f;       // Početna pozicija Y (donja leva ivica gornje polovine)
float angle = 0.0f;             // Početni ugao
bool sunIsSet = false;


struct Cloud {
	float x, y;
	float speed;
	float radius;

	Cloud(float startX, float startY, float speed, float radius)
		: x(startX), y(startY), speed(speed), radius(radius) {
	}
	Cloud() : x(0.0f), y(0.0f), speed(0.1f), radius(0.1f) {}
};
Cloud clouds[3];
const int STAR_COUNT = 100;


struct Island {
	float radius; 
	float x;       
	float y;      

	Island(float r, float xPos, float yPos)
		: radius(r), x(xPos), y(yPos) {}
};

bool mouseClickedOnWater = false;
float clickX = 0.0f;
float clickY = 0.0f;
float clickTime = 0.0f;
float maxRadius = 0.5f;


bool mouseClickedOnFire = false;
float startX = 0.0f;
float startY = -0.2f;
float offsetY[5] = { -0.2f, -0.2f, -0.2f, -0.2f, -0.2f};
bool isVisible[5] = { true, false, false, false, false };


float flameLightColor[] = { 1.0f, 0.0f, 0.0f };   
float flameLightIntensity = 0.1f;
float flameRadius = 0.1f;
float flameAngle = 0; 
float flameCenterX = -0.25f;
float flameCenterY = 0.25f;
int numPoints = 10;
float flameLightPosition[20]; 


float timeFactor = 1.0f;  
float initialTimeFactor = 1.0f; 
std::chrono::steady_clock::time_point lastClickTime;
const int debounceDelay = 200;


float sharkSpeed = 0.01f;
const int numSharks = 4; 
float sharkPositions[numSharks][2];
float sharkDirections[numSharks][2]; 
bool sharksMoving[numSharks];
float initialSharkPositions[numSharks][2];
float sharkVertices[numSharks * 6]; 
float sharkTemplate[] = {
  -0.075f, -0.0475f,
 0.075f, -0.0475f,  
 0.0f,   0.175f
};



unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource); 
void generateCircle(float* circle, int offset, float r, float centerX, float centerY);
void bindCircleData(unsigned int VAO, unsigned int VBO, float* data, size_t dataSize);
static unsigned loadImageToTexture(const char* filePath); 
void mouse_callback(GLFWwindow* window, int button, int action, int mods);
void handleMouseClick(float clickX, float clickY, Island* islands, int numIslands, float fireVertices[]);
bool isPointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy);
bool isClickOnFire(float clickX, float clickY, float fireVertices[]);
bool isClickOnWater(float clickX, float clickY, Island* islands, int numIslands);
bool isClickOnIsland(float clickX, float clickY, const Island& island);
void setSkyAndStars(unsigned int starVAO, unsigned int starShaderProgram);
void setClouds(unsigned int* cloudVAO, unsigned int cloudShaderProgram);
void updateClouds();
void updateSun(unsigned int* VAO, unsigned int sunShaderProgram);
void updateSunPosition(unsigned int sunShaderProgram);
void updateMoonPosition(unsigned int sunShaderProgram);
void writeName(unsigned int nameVAO, unsigned int nameVBO, unsigned int nameTexture, unsigned int nameShaderProgram);
void setPalm(unsigned int VAO, unsigned int VBO, unsigned int palmShaderProgram);
void setFire(unsigned int VAO, unsigned int VBO, unsigned int fireShaderProgram, float fireSize);
void moveSharks();
void setSharks(unsigned int sharksVAO, unsigned int sharksVBO, unsigned int sharkShaderProgram, float waterLevel);
void returnSharks();
void beginRedCircle(double r5pom, double r5, unsigned int VAO, unsigned int VBO, unsigned int redCircleShaderProgram);
void generateRounderCircle(float* circle, int offset, float r, float centerX, float centerY, float aspectRatio);
void setRedCircleUniforms(unsigned int shaderProgram);
void setIslands(float fireSize, float waterLevel, unsigned int* VAO, unsigned int* VBO, unsigned int islandsShaderProgram);
void setWater(unsigned int* waterVAO, unsigned int* waterVBO, unsigned int waterShaderProgram, bool waterTransparencyEnabled);
void generateSmokeLetters(unsigned int pomocVAO, unsigned int pomocVBO, unsigned int smokeTexture, unsigned int pTexture, unsigned int oTexture, unsigned int mTexture, unsigned int cTexture, unsigned int pomocShaderProgram);
void updateFlameLight(float flameSize, float time);
void decreaseTimeSpeed();
void increaseTimeSpeed();
void resetTime();
bool isDebounced();


int main(void)
{

	lastClickTime = std::chrono::steady_clock::now() - std::chrono::milliseconds(debounceDelay);

	if (!glfwInit())
	{
		std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window;
	//unsigned int wWidth = 1000;
	//unsigned int wHeight = 600;
	const char wTitle[] = "[Island]";

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

	float aspectRatio = (float)mode->width / (float)mode->height;

	float heightLimit = (float)mode->height / 2.0f; // Halfway point of the screen (for disappearing)



	window = glfwCreateWindow(mode->width, mode->height, wTitle, primaryMonitor, NULL); // Napravi novi prozor
	//window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL); // Napravi novi prozor
	if (window == NULL) //Ako prozor nije napravljen
	{
		std::cout << "Prozor nije napravljen! :(\n";
		glfwTerminate(); //Gasi GLFW
		return 2; //Vrati kod za gresku
	}
	// Postavljanje novopecenog prozora kao aktivni (sa kojim cemo da radimo)
	glfwMakeContextCurrent(window);

	// Inicijalizacija GLEW biblioteke
	if (glewInit() != GLEW_OK) //Slicno kao glfwInit. GLEW_OK je predefinisani izlazni kod za uspjesnu inicijalizaciju sadrzan unutar biblioteke
	{
		std::cout << "GLEW nije mogao da se ucita! :'(\n";
		return 3;
	}


	unsigned int islandsShaderProgram = createShader("island.vert", "island.frag"); 
	unsigned int sunShaderProgram = createShader("sun.vert", "basic.frag");
	unsigned int palmShaderProgram = createShader("palm.vert", "basic.frag"); 
	unsigned int fireShaderProgram = createShader("fire.vert", "basic.frag");
	unsigned int waterShaderProgram = createShader("basic.vert", "basic.frag");
	unsigned int sharkShaderProgram = createShader("shark.vert", "basic.frag");
	unsigned int redCircleShaderProgram = createShader("basic.vert", "basic.frag");
	unsigned int cloudShaderProgram = createShader("cloud.vert", "basic.frag");
	unsigned int starShaderProgram = createShader("star.vert", "star.frag");
	unsigned int nameShaderProgram = createShader("name.vert", "name.frag");
	unsigned int pomocShaderProgram = createShader("name.vert", "pomoc.frag");

	unsigned int palmVAO;
	unsigned int palmVBO;
	glGenVertexArrays(1, &palmVAO);
	glGenBuffers(1, &palmVBO);

	unsigned int fireVAO;
	unsigned int fireVBO;
	glGenVertexArrays(1, &fireVAO);
	glGenBuffers(1, &fireVBO);

	float palmVertices[] = {

		0.03f + 0.2f, -0.4f + 0.3f,
		-0.03f + 0.2f, -0.4f + 0.3f,
		0.03f + 0.2f,  0.4f + 0.3f,

		-0.03f + 0.2f, -0.4f + 0.3f,
		-0.03f + 0.2f,  0.4f + 0.3f,
		0.03f + 0.2f,  0.4f + 0.3f,

		// Leaf 1
		0.0f + 0.2f,  0.4f + 0.3f,
		-0.2f + 0.2f,  0.2f + 0.3f,
		-0.1f + 0.2f,  0.1f + 0.3f,

		// Leaf 2
		0.0f + 0.2f,  0.4f + 0.3f,
		0.2f + 0.2f,  0.2f + 0.3f,
		0.1f + 0.2f,  0.1f + 0.3f,

		// Leaf 3
		0.0f + 0.2f,  0.4f + 0.3f,
		-0.05f + 0.2f,  0.1f + 0.3f,
		0.05f + 0.2f,  0.1f + 0.3f
	};

	float fireVertices[] = {
	-0.3f, -0.1f,
	-0.1f, -0.1f,
	-0.2f,  0.2f
	};


	glBindVertexArray(palmVAO);
	glBindBuffer(GL_ARRAY_BUFFER, palmVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(palmVertices), palmVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); // Koordinate
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(fireVAO);
	glBindBuffer(GL_ARRAY_BUFFER, fireVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fireVertices), fireVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); // Koordinate
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	unsigned int islandsVAO[3]; 
	unsigned int islandsVBO[3]; 
	glGenVertexArrays(3, islandsVAO);
	glGenBuffers(3, islandsVBO);

	unsigned int sunVAO[2];  //sun and moon
	unsigned int sunVBO[2];
	glGenVertexArrays(2, sunVAO);
	glGenBuffers(2, sunVBO);

	float r1 = 0.5f;
	float r2 = 0.3f;
	float r3 = 0.3f;

	Island islands[] = { {r1, 0.0f, 0.0f}, {r2,0.8f, 0.0f}, {r3, -0.8f, 0.0f} };

	float sunCircle[(CRES + 2) * 2];
	generateRounderCircle(sunCircle, 0, 0.15f, 0.0f, 0.0f, aspectRatio);
	bindCircleData(sunVAO[0], sunVBO[0], sunCircle, sizeof(sunCircle));

	float moonCircle[(CRES + 2) * 2];
	generateRounderCircle(moonCircle, 0, 0.1f, 0.0f, 0.0f, aspectRatio);
	bindCircleData(sunVAO[1], sunVBO[1], moonCircle, sizeof(moonCircle));

	float islandCircle1[(CRES + 2) * 2];
	generateCircle(islandCircle1, 0, r1, 0.0f, 0.0f);
	bindCircleData(islandsVAO[0], islandsVBO[0], islandCircle1, sizeof(islandCircle1));

	float islandCircle2[(CRES + 2) * 2];
	generateCircle(islandCircle2, 0, r2, 0.8f, 0.0f);
	bindCircleData(islandsVAO[1], islandsVBO[1], islandCircle2, sizeof(islandCircle2));

	float islandCircle3[(CRES + 2) * 2];
	generateCircle(islandCircle3, 0, r3, -0.8f, 0.0f);
	bindCircleData(islandsVAO[2], islandsVBO[2], islandCircle3, sizeof(islandCircle3));


	unsigned int waterVAO[1];
	unsigned int waterVBO[1];
	glGenVertexArrays(1, waterVAO);
	glGenBuffers(1, waterVBO);


	float waterVertices[] = {
	-1.0f, -1.0f, // Donji levi ugao
	-1.0f,  1.0f, // Gornji levi ugao
	 1.0f, -1.0f, // Donji desni ugao

	-1.0f,  1.0f, // Gornji levi ugao
	 1.0f,  1.0f, // Gornji desni ugao
	 1.0f, -1.0f  // Donji desni ugao
	};

	glBindVertexArray(waterVAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, waterVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), waterVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	unsigned int redCircleVAO;
	unsigned int redCircleVBO;
	glGenVertexArrays(1, &redCircleVAO);
	glGenBuffers(1, &redCircleVBO);
	float redCircleR = 0.05f;
	float r5pom = 0.05f;

	float r_cloud = 0.1f;
	unsigned int cloudVAO[3];
	unsigned int cloudVBO[3];
	glGenVertexArrays(3, cloudVAO);
	glGenBuffers(3, cloudVBO);

	float cloud1[(CRES + 2) * 2];
	generateCircle(cloud1, 0, r_cloud, 0.0f, 0.0f);
	bindCircleData(cloudVAO[0], cloudVBO[0], cloud1, sizeof(cloud1));

	float cloud2[(CRES + 2) * 2];
	generateCircle(cloud2, 0, r_cloud, 0.0f, 0.0f);
	bindCircleData(cloudVAO[1], cloudVBO[1], cloud2, sizeof(cloud2));

	float cloud3[(CRES + 2) * 2];
	generateCircle(cloud3, 0, r_cloud, 0.0f, 0.0f);
	bindCircleData(cloudVAO[2], cloudVBO[2], cloud3, sizeof(cloud3));

	clouds[0] = Cloud(-1.0f, 0.8f, 0.001f, 0.1f); 
	clouds[1] = Cloud(-0.5f, 0.8f, 0.0015f, 0.12f);
	clouds[2] = Cloud(-0.8f, 0.8f, 0.002f, 0.08f);


	const float centerX = 0.0f;
	const float centerY = -0.2f;
	r1 = 0.7;

	for (int i = 0; i < numSharks; i++) {
		initialSharkPositions[i][0] = r1 * cos((2.0f * 3.141592f / numSharks) * i) + centerX;
		initialSharkPositions[i][1] = r1 * sin((2.0f * 3.141592f / numSharks) * i) + centerY;
	}

	for (int i = 0; i < numSharks; i++) {
		sharkPositions[i][0] = r1 * cos((2.0f * 3.141592f / numSharks) * i) + centerX;
		sharkPositions[i][1] = r1 * sin((2.0f * 3.141592f / numSharks) * i) + centerY;
		sharksMoving[i] = false; 
	}

	for (int i = 0; i < numSharks; i++) {
		float angle = (2.0f * 3.141592f / numSharks) * i;
		float offsetX = r1 * cos(angle) + centerX;
		float offsetY = r1 * sin(angle) + centerY;

		if (angle >= 0.0f && angle <= 3.141592f) {
			offsetY -= 0.1f;
		}

		for (int j = 0; j < 6; j += 2) {
			sharkVertices[i * 6 + j] = sharkTemplate[j] + offsetX;
			sharkVertices[i * 6 + j + 1] = sharkTemplate[j + 1] + offsetY;
		}
	}

	unsigned int sharksVAO, sharksVBO;
	glGenVertexArrays(1, &sharksVAO);
	glGenBuffers(1, &sharksVBO);

	glBindVertexArray(sharksVAO);

	glBindBuffer(GL_ARRAY_BUFFER, sharksVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sharkVertices), sharkVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	bool waterTransparencyEnabled = false;
	bool bKeyPressed = false;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	int width, height, channels;
	unsigned char* image= stbi_load("images/udica.png", &width, &height, &channels, 4);

	GLFWimage cursorImage;
	cursorImage.width = width;
	cursorImage.height = height;
	cursorImage.pixels = image;

	GLFWcursor* cursor = glfwCreateCursor(&cursorImage, 0, 0);

	glfwSetMouseButtonCallback(window, mouse_callback);


	float starVertices[STAR_COUNT * 3];

	for (int i = 0; i < STAR_COUNT; i++) {
		starVertices[i * 3] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
		starVertices[i * 3 + 1] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;

		starVertices[i * 3 + 2] = ((float)rand() / RAND_MAX) * 5.0f + 3.0f;
	}

	unsigned int starVAO, starVBO;
	glGenVertexArrays(1, &starVAO);
	glGenBuffers(1, &starVBO);

	glBindVertexArray(starVAO);
	glBindBuffer(GL_ARRAY_BUFFER, starVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(starVertices), starVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	float nameVertices[] = {
		-1.0f, -1.0f,   0.0f, 0.0f,
		-1.0f + 0.3f, -1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f + 0.1f,  0.0f, 1.0f,

		-1.0f + 0.3f, -1.0f, 1.0f, 0.0f,
		-1.0f + 0.3f, -1.0f + 0.1f, 1.0f, 1.0f,
		-1.0f, -1.0f + 0.1f, 0.0f, 1.0f 
	};

	unsigned int nameVAO, nameVBO;
	glGenVertexArrays(1, &nameVAO);
	glGenBuffers(1, &nameVBO);
	glBindVertexArray(nameVAO);
	glBindBuffer(GL_ARRAY_BUFFER, nameVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(nameVertices), nameVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	unsigned nameTexture = loadImageToTexture("images/ime.png");
	glBindTexture(GL_TEXTURE_2D, nameTexture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	float pomocVertices[] = {
		-1.2f / 5.0f,  0.2f / 5.0f,   0.0f, 0.0f,  // Donji levi ugao kvadrata 1
		-0.7f / 5.0f,  0.2f / 5.0f,   1.0f, 0.0f,  // Donji desni ugao kvadrata 1
		-1.2f / 5.0f,  0.7f / 5.0f,   0.0f, 1.0f,  // Gornji levi ugao kvadrata 1
		-0.7f / 5.0f,  0.2f / 5.0f,   1.0f, 0.0f,  // Donji desni ugao kvadrata 1
		-0.7f / 5.0f,  0.7f / 5.0f,   1.0f, 1.0f,  // Gornji desni ugao kvadrata 1
		-1.2f / 5.0f,  0.7f / 5.0f,   0.0f, 1.0f,  // Gornji levi ugao kvadrata 1

		-1.2f / 5.0f,  0.2f / 5.0f,   0.0f, 0.0f,  // Donji levi ugao kvadrata 1
		-0.7f / 5.0f,  0.2f / 5.0f,   1.0f, 0.0f,  // Donji desni ugao kvadrata 1
		-1.2f / 5.0f,  0.7f / 5.0f,   0.0f, 1.0f,  // Gornji levi ugao kvadrata 1
		-0.7f / 5.0f,  0.2f / 5.0f,   1.0f, 0.0f,  // Donji desni ugao kvadrata 1
		-0.7f / 5.0f,  0.7f / 5.0f,   1.0f, 1.0f,  // Gornji desni ugao kvadrata 1
		-1.2f / 5.0f,  0.7f / 5.0f,   0.0f, 1.0f,  // Gornji levi ugao kvadrata 1

		-1.2f / 5.0f,  0.2f / 5.0f,   0.0f, 0.0f,  // Donji levi ugao kvadrata 1
		-0.7f / 5.0f,  0.2f / 5.0f,   1.0f, 0.0f,  // Donji desni ugao kvadrata 1
		-1.2f / 5.0f,  0.7f / 5.0f,   0.0f, 1.0f,  // Gornji levi ugao kvadrata 1
		-0.7f / 5.0f,  0.2f / 5.0f,   1.0f, 0.0f,  // Donji desni ugao kvadrata 1
		-0.7f / 5.0f,  0.7f / 5.0f,   1.0f, 1.0f,  // Gornji desni ugao kvadrata 1
		-1.2f / 5.0f,  0.7f / 5.0f,   0.0f, 1.0f,  // Gornji levi ugao kvadrata 1

		-1.2f / 5.0f,  0.2f / 5.0f,   0.0f, 0.0f,  // Donji levi ugao kvadrata 1
		-0.7f / 5.0f,  0.2f / 5.0f,   1.0f, 0.0f,  // Donji desni ugao kvadrata 1
		-1.2f / 5.0f,  0.7f / 5.0f,   0.0f, 1.0f,  // Gornji levi ugao kvadrata 1
		-0.7f / 5.0f,  0.2f / 5.0f,   1.0f, 0.0f,  // Donji desni ugao kvadrata 1
		-0.7f / 5.0f,  0.7f / 5.0f,   1.0f, 1.0f,  // Gornji desni ugao kvadrata 1
		-1.2f / 5.0f,  0.7f / 5.0f,   0.0f, 1.0f,  // Gornji levi ugao kvadrata 1

		-1.2f / 5.0f,  0.2f / 5.0f,   0.0f, 0.0f,  // Donji levi ugao kvadrata 1
		-0.7f / 5.0f,  0.2f / 5.0f,   1.0f, 0.0f,  // Donji desni ugao kvadrata 1
		-1.2f / 5.0f,  0.7f / 5.0f,   0.0f, 1.0f,  // Gornji levi ugao kvadrata 1
		-0.7f / 5.0f,  0.2f / 5.0f,   1.0f, 0.0f,  // Donji desni ugao kvadrata 1
		-0.7f / 5.0f,  0.7f / 5.0f,   1.0f, 1.0f,  // Gornji desni ugao kvadrata 1
		-1.2f / 5.0f,  0.7f / 5.0f,   0.0f, 1.0f,  // Gornji levi ugao kvadrata 1

	};



	unsigned int pomocVAO, pomocVBO;
	glGenVertexArrays(1, &pomocVAO);
	glGenBuffers(1, &pomocVBO);

	glBindVertexArray(pomocVAO);

	glBindBuffer(GL_ARRAY_BUFFER, pomocVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pomocVertices), pomocVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	unsigned pTexture = loadImageToTexture("images/P.png");
	glBindTexture(GL_TEXTURE_2D, pTexture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned oTexture = loadImageToTexture("images/O.png");
	glBindTexture(GL_TEXTURE_2D, oTexture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned mTexture = loadImageToTexture("images/M.png");
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned cTexture = loadImageToTexture("images/C.png");
	glBindTexture(GL_TEXTURE_2D, cTexture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned smokeTexture = loadImageToTexture("images/smoke.png");
	glBindTexture(GL_TEXTURE_2D, smokeTexture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	const double TARGET_FRAME_TIME = 1.0 / 60.0;
	double lastFrameTime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		double currentFrameTime = glfwGetTime();
		double deltaTime = currentFrameTime - lastFrameTime;

		if (deltaTime >= TARGET_FRAME_TIME) {

			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(window, GL_TRUE);
			}

			if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
			{
				decreaseTimeSpeed();
			}
			if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)   //nemam kp_add i kp_subtract
			{
				increaseTimeSpeed();
			}
			if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
			{
				resetTime();
			}

			if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && waterTransparencyEnabled && isDebounced()) {
				waterTransparencyEnabled = false;
			}
			else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !waterTransparencyEnabled && isDebounced()) {
				waterTransparencyEnabled = true;
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glfwSetCursor(window, cursor);

			int wWidth = mode->width;
			int wHeight = mode->height;

			glEnable(GL_SCISSOR_TEST);
			glEnable(GL_DEPTH_TEST);


			// --- Top half of the screen (Sky blue) ---

			glViewport(0, wHeight / 2, wWidth, wHeight / 2);
			glScissor(0, wHeight / 2, wWidth, wHeight / 2);

			setSkyAndStars(starVAO, starShaderProgram);

			setClouds(cloudVAO, cloudShaderProgram);

			updateSun(sunVAO, sunShaderProgram);

			glViewport(0, 0, wWidth, wHeight / 2);
			glScissor(0, 0, wWidth, wHeight / 2);


			writeName(nameVAO, nameVBO, nameTexture, nameShaderProgram);

			setPalm(palmVAO, palmVBO, palmShaderProgram);

			float fireSize = sin(glfwGetTime() * timeFactor) * 1.0f + 1.5f;

			setFire(fireVAO, fireVBO, fireShaderProgram, fireSize);

			float waterLevel = abs(sin(glfwGetTime() * timeFactor)) * 0.3f;

			moveSharks();

			setSharks(sharksVAO, sharksVBO, sharkShaderProgram, waterLevel);

			if (mouseClickedOnWater) {
				beginRedCircle(r5pom, redCircleR, redCircleVAO, redCircleVBO, redCircleShaderProgram);
				r5pom += 0.0004f * timeFactor;
			}
			else {
				returnSharks();
				r5pom = redCircleR;

			}

			float flameTime = glfwGetTime() * timeFactor;

			updateFlameLight(fireSize, flameTime);

			setIslands(fireSize, waterLevel, islandsVAO, islandsVBO, islandsShaderProgram);

			setWater(waterVAO, waterVBO, waterShaderProgram, waterTransparencyEnabled);


			glBindVertexArray(0);
			glUseProgram(0);

			glDisable(GL_SCISSOR_TEST);
			glDisable(GL_DEPTH_TEST);

			glViewport(0, 0, wWidth, wHeight);

			if (mouseClickedOnFire) {

				generateSmokeLetters(pomocVAO, pomocVBO, smokeTexture, pTexture, oTexture, mTexture, cTexture, pomocShaderProgram);
			}

			glfwSwapBuffers(window);

			glfwPollEvents();

			lastFrameTime = currentFrameTime;
		}
	}
	glDeleteVertexArrays(3, islandsVAO);
	glDeleteBuffers(3, islandsVBO);

	glDeleteVertexArrays(2, sunVAO);
	glDeleteBuffers(2, sunVBO);

	glDeleteVertexArrays(1, waterVAO);
	glDeleteBuffers(1, waterVBO);

	glDeleteVertexArrays(3, cloudVAO);
	glDeleteBuffers(3, cloudVBO);

	glDeleteVertexArrays(1, &palmVAO);
	glDeleteBuffers(1, &palmVBO);

	glDeleteVertexArrays(1, &fireVAO);
	glDeleteBuffers(1, &fireVBO);

	glDeleteVertexArrays(1, &sharksVAO);
	glDeleteBuffers(1, &sharksVBO);

	glDeleteVertexArrays(1, &starVAO);
	glDeleteBuffers(1, &starVBO);

	glDeleteVertexArrays(1, &nameVAO);
	glDeleteBuffers(1, &nameVBO);

	glDeleteVertexArrays(1, &pomocVAO);
	glDeleteBuffers(1, &pomocVBO);

	glDeleteShader(islandsShaderProgram);
	glDeleteShader(sunShaderProgram);
	glDeleteShader(palmShaderProgram);
	glDeleteShader(fireShaderProgram);
	glDeleteShader(waterShaderProgram);
	glDeleteShader(sharkShaderProgram);
	glDeleteShader(redCircleShaderProgram);
	glDeleteShader(cloudShaderProgram);
	glDeleteShader(starShaderProgram);
	glDeleteShader(nameShaderProgram);

	glDeleteTextures(1, &nameTexture);
	glDeleteTextures(1, &pTexture);
	glDeleteTextures(1, &oTexture);
	glDeleteTextures(1, &mTexture);
	glDeleteTextures(1, &cTexture);
	glDeleteTextures(1, &smokeTexture);


	glfwTerminate();
	return 0;
}


bool isDebounced() {
	auto now = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastClickTime).count();
	if (duration > debounceDelay) {
		lastClickTime = now;
		return true;
	}
	return false;
}

void increaseTimeSpeed() {
	if (isDebounced()) {
		std::cout << "vece";
		timeFactor += 0.3f;
		angleSpeed += 0.001;
		sharkSpeed += 0.001;
	}
}

void decreaseTimeSpeed() {

	if (isDebounced() && timeFactor - 0.3 > 0.0) {
		std::cout << "manje";
		timeFactor -= 0.3f;
		angleSpeed -= 0.001;
		sharkSpeed -= 0.001;
	}
}

void resetTime() {
	timeFactor = initialTimeFactor; 
	angleSpeed = 0.003f; 
	sharkSpeed = 0.01f; 
}


void updateFlameLight(float flameSize, float time) {

	flameAngle = time;

	for (int i = 0; i < numPoints; i++) {
		flameLightPosition[2 * i] = flameCenterX + flameRadius * cos(flameAngle); 
		flameLightPosition[2 * i + 1] = flameCenterY + flameRadius * sin(flameAngle);
	}
}

void setRedCircleUniforms(unsigned int shaderProgram) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	unsigned int clickPosLocation = glGetUniformLocation(shaderProgram, "clickPosition");
	unsigned int timeLocation = glGetUniformLocation(shaderProgram, "time");
	unsigned int startTimeLocation = glGetUniformLocation(shaderProgram, "startTime");
	unsigned int maxRadiusLocation = glGetUniformLocation(shaderProgram, "maxRadius");
	unsigned int colorLocation = glGetUniformLocation(shaderProgram, "color");

	glUniform4f(colorLocation, 1.0f, 0.0f, 0.0f, 0.5f);
	glUniform2f(clickPosLocation, clickX, clickY);
	glUniform1f(timeLocation, glfwGetTime()*timeFactor);
	glUniform1f(startTimeLocation, clickTime);
	glUniform1f(maxRadiusLocation, maxRadius);
}


void updateClouds() {
	for (auto& cloud : clouds) {
		cloud.x += cloud.speed * timeFactor;

		if (cloud.x > 1.0f) {
			cloud.x = -1.0f - (rand() % 10) * 0.1f;
		}
	}
}

void updateSun(unsigned int *VAO, unsigned int sunShaderProgram) {
	if (!sunIsSet) {
		updateSunPosition(sunShaderProgram);
		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2)); 
	}
	else {
		updateMoonPosition(sunShaderProgram);
		glBindVertexArray(VAO[1]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));
	}
}

void writeName(unsigned int nameVAO, unsigned int nameVBO,unsigned int nameTexture, unsigned int nameShaderProgram) {
	glUseProgram(nameShaderProgram);
	unsigned uTexLoc = glGetUniformLocation(nameShaderProgram, "uTex");
	glUniform1i(uTexLoc, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, nameTexture);

	glBindVertexArray(nameVAO);
	glBindBuffer(GL_ARRAY_BUFFER, nameVBO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

}

void setPalm(unsigned int palmVAO, unsigned int palmVBO, unsigned int palmShaderProgram) {

	glUseProgram(palmShaderProgram);
	unsigned int ambientLightLocation = glGetUniformLocation(palmShaderProgram, "ambientLight");
	if (sunIsSet) {
		glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f);
	}
	else {
		glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	unsigned int palmColorLocation = glGetUniformLocation(palmShaderProgram, "color");

	glUniform4f(palmColorLocation, 0.0f, 0.5f, 0.0f, 1.0f);

	glBindVertexArray(palmVAO);
	glBindBuffer(GL_ARRAY_BUFFER, palmVBO);

	glDrawArrays(GL_TRIANGLES, 6, 3);
	glDrawArrays(GL_TRIANGLES, 9, 3);
	glDrawArrays(GL_TRIANGLES, 12, 3);


	glUniform4f(palmColorLocation, 0.545f, 0.271f, 0.075f, 1.0f);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void setFire(unsigned int fireVAO, unsigned int fireVBO, unsigned int fireShaderProgram, float fireSize) {

	glUseProgram(fireShaderProgram);

	unsigned int ambientLightLocation = glGetUniformLocation(fireShaderProgram, "ambientLight");
	if (sunIsSet) {
		glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f);
	}
	else {
		glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	unsigned int fireSizeLocation = glGetUniformLocation(fireShaderProgram, "scaleY");
	glUniform1f(fireSizeLocation, fireSize);

	float green = sin(glfwGetTime() * timeFactor * 3.0f) * 0.3f + 0.3f;

	unsigned int fireColorLocation = glGetUniformLocation(fireShaderProgram, "color");
	glUniform4f(fireColorLocation, 1.0f, green, 0.0f, 1.0f);

	glBindVertexArray(fireVAO);
	glBindBuffer(GL_ARRAY_BUFFER, fireVBO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void moveSharks() {
	for (int i = 0; i < numSharks; i++) {
		if (sharksMoving[i]) {

			sharkPositions[i][0] += sharkDirections[i][0] * sharkSpeed;
			sharkPositions[i][1] += sharkDirections[i][1] * sharkSpeed;

			float dx = sharkPositions[i][0] + 0.2f;
			float dy = sharkPositions[i][1] + 0.2f;
			float distanceFromIsland = sqrt(dx * dx + dy * dy);

			if (distanceFromIsland < 0.5f + 0.2f) {
				float avoidDirectionX = dx / distanceFromIsland;
				float avoidDirectionY = dy / distanceFromIsland;

				sharkDirections[i][0] = avoidDirectionX;
				sharkDirections[i][1] = avoidDirectionY;

				sharkPositions[i][0] += avoidDirectionX * sharkSpeed;
				sharkPositions[i][1] += avoidDirectionY * sharkSpeed;
			}

			dx = clickX - sharkPositions[i][0];
			dy = clickY - sharkPositions[i][1];
			float distanceToCircle = sqrt(dx * dx + dy * dy);
			if (distanceToCircle <= maxRadius) {
				sharksMoving[i] = false; 
			}
		}
	}
}
void returnSharks() {
	for (int i = 0; i < numSharks; i++) {
		float dx = initialSharkPositions[i][0] - sharkPositions[i][0];
		float dy = initialSharkPositions[i][1] - sharkPositions[i][1];
		float distance = sqrt(dx * dx + dy * dy);

		if (distance > 0.01f) {
			float islandDx = sharkPositions[i][0];
			float islandDy = sharkPositions[i][1];
			float distanceFromIsland = sqrt(islandDx * islandDx + islandDy * islandDy);

			if (distanceFromIsland < 0.5f) { 

				float avoidDirectionX = islandDx / distanceFromIsland;
				float avoidDirectionY = islandDy / distanceFromIsland;

				sharkPositions[i][0] += avoidDirectionX * sharkSpeed;
				sharkPositions[i][1] += avoidDirectionY * sharkSpeed;
			}
			else {
				sharkPositions[i][0] += (dx / distance) * sharkSpeed;
				sharkPositions[i][1] += (dy / distance) * sharkSpeed;
			}
		}
		else {
			sharkPositions[i][0] = initialSharkPositions[i][0];
			sharkPositions[i][1] = initialSharkPositions[i][1];
			sharksMoving[i] = false;
		}
	}
}

void setSharks(unsigned int sharksVAO, unsigned int sharksVBO, unsigned int sharkShaderProgram, float waterLevel) {
	glUseProgram(sharkShaderProgram);

	unsigned int ambientLightLocation = glGetUniformLocation(sharkShaderProgram, "ambientLight");
	if (sunIsSet) {
		glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); 
	}
	else {
		glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	unsigned int sharkColorLocation = glGetUniformLocation(sharkShaderProgram, "color");
	glUniform4f(sharkColorLocation, 0.5f, 0.5f, 0.5f, 1.0f);

	unsigned int waterLevelLocation = glGetUniformLocation(sharkShaderProgram, "waterLevel");
	glUniform1f(waterLevelLocation, waterLevel);

	unsigned int sharkTimeLocation = glGetUniformLocation(sharkShaderProgram, "time");
	glUniform1f(sharkTimeLocation, glfwGetTime() * timeFactor);

	unsigned int sharkSpeedLocation = glGetUniformLocation(sharkShaderProgram, "speed");
	glUniform1f(sharkSpeedLocation, 0.8f * timeFactor);

	for (int i = 0; i < numSharks; i++) {
		float offsetX = sharkPositions[i][0];
		float offsetY = sharkPositions[i][1];
		for (int j = 0; j < 6; j += 2) {
			sharkVertices[i * 6 + j] = sharkTemplate[j] + offsetX;
			sharkVertices[i * 6 + j + 1] = sharkTemplate[j + 1] + offsetY;
		}
	}
	glBindVertexArray(sharksVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sharksVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sharkVertices), sharkVertices, GL_DYNAMIC_DRAW);

	glDrawArrays(GL_TRIANGLES, 0, numSharks * 3);
}

void beginRedCircle(double r5pom, double r5, unsigned int VAO, unsigned int VBO, unsigned int redCircleShaderProgram) {
	for (int i = 0; i < numSharks; i++) {
		float dx = clickX - sharkPositions[i][0];
		float dy = clickY - sharkPositions[i][1];
		float length = sqrt(dx * dx + dy * dy);
		sharkDirections[i][0] = dx / length;
		sharkDirections[i][1] = dy / length;
		sharksMoving[i] = true; 
	}

	float elapsedTime = glfwGetTime() * timeFactor - clickTime;
	float circle5[(CRES + 2) * 2];
	generateCircle(circle5, 0, r5pom, clickX, clickY);
	bindCircleData(VAO, VBO, circle5, sizeof(circle5));

	glUseProgram(redCircleShaderProgram);

	unsigned int ambientLightLocation = glGetUniformLocation(redCircleShaderProgram, "ambientLight");
	if (sunIsSet) {
		glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); 
	}
	else {
		glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	setRedCircleUniforms(redCircleShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));

	if (elapsedTime > 5.0) {
		mouseClickedOnWater = false;
	}
}

void setIslands(float fireSize, float waterLevel, unsigned int *VAO, unsigned int *VBO, unsigned int islandsShaderProgram) {
	glUseProgram(islandsShaderProgram);

	unsigned int flameLightPositionLoc = glGetUniformLocation(islandsShaderProgram, "flameLightPosition");
	unsigned int flameLightColorLoc = glGetUniformLocation(islandsShaderProgram, "flameLightColor");
	unsigned int flameLightIntensityLoc = glGetUniformLocation(islandsShaderProgram, "flameLightIntensity");

	unsigned int fireSizeLocation = glGetUniformLocation(islandsShaderProgram, "scaleY");
	glUniform1f(fireSizeLocation, fireSize);

	glUniform3fv(flameLightPositionLoc, 1, flameLightPosition);  
	glUniform3fv(flameLightColorLoc, 1, flameLightColor);      
	glUniform1f(flameLightIntensityLoc, flameLightIntensity);       


	unsigned int ambientLightLocation = glGetUniformLocation(islandsShaderProgram, "ambientLight");
	if (sunIsSet) {
		glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); 
	}
	else {
		glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	unsigned int islandOffsetLocation = glGetUniformLocation(islandsShaderProgram, "offset");

	unsigned int islandColorLocation = glGetUniformLocation(islandsShaderProgram, "color");
	glUniform4f(islandColorLocation, 194.0f / 255.0f, 178.0f / 255.0f, 128.0f / 255.0f, 1.0f);

	unsigned int waterLevelLocation = glGetUniformLocation(islandsShaderProgram, "waterLevel");
	glUniform1f(waterLevelLocation, waterLevel);


	glUniform2f(islandOffsetLocation, 0.0f, -0.2f);
	glBindVertexArray(VAO[0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));

	glUniform2f(islandOffsetLocation, -0.1f, 0.6f);
	glBindVertexArray(VAO[1]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));

	glUniform2f(islandOffsetLocation, 0.2f, 0.5f);
	glBindVertexArray(VAO[2]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));
}

void setWater(unsigned int *waterVAO, unsigned int *waterVBO, unsigned int waterShaderProgram, bool waterTransparencyEnabled) {
	glUseProgram(waterShaderProgram);

	unsigned int ambientLightLocation = glGetUniformLocation(waterShaderProgram, "ambientLight");
	if (sunIsSet) {
		glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f);
	}
	else {
		glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	unsigned int waterColorLocation = glGetUniformLocation(waterShaderProgram, "color");

	if (waterTransparencyEnabled) {
		glUniform4f(waterColorLocation, 0.0f, 0.0f, 0.5f, 0.3f);
	}
	else {
		glUniform4f(waterColorLocation, 0.0f, 0.0f, 0.5f, 1.0f);
	}

	glBindVertexArray(waterVAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, waterVBO[0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void generateSmokeLetters(unsigned int pomocVAO, unsigned int pomocVBO, unsigned int smokeTexture, unsigned int pTexture,
	unsigned int oTexture, unsigned int mTexture, unsigned int cTexture, unsigned int pomocShaderProgram) {
	unsigned int textTextureLoc = glGetUniformLocation(pomocShaderProgram, "textTexture");
	unsigned int smokeTextureLoc = glGetUniformLocation(pomocShaderProgram, "smokeTexture");
	unsigned int translationLoc = glGetUniformLocation(pomocShaderProgram, "translation");

	glUseProgram(pomocShaderProgram);

	glUniform1i(smokeTextureLoc, 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, smokeTexture);

	for (int i = 0; i < 5; i++) {
		if (isVisible[i]) { 
			offsetY[i] += 0.002f * timeFactor;

			if (offsetY[i] > 0.5f) {
				isVisible[i] = false;
				if (i + 1 < 5) { 
					isVisible[i + 1] = true;
				}
			}
			else if (offsetY[i] > -0.06f) {
				if (i + 1 < 5) { 
					isVisible[i + 1] = true;
				}
			}

			glUniform2f(translationLoc, startX, startY + offsetY[i]);

			switch (i) {
			case 0: // C
				glUniform1i(textTextureLoc, 4);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, cTexture);
				break;
			case 1: // O
				glUniform1i(textTextureLoc, 2);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, oTexture);
				break;
			case 2: // M
				glUniform1i(textTextureLoc, 3);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, mTexture);
				break;
			case 3: // O
				glUniform1i(textTextureLoc, 2);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, oTexture);
				break;
			case 4: // P
				glUniform1i(textTextureLoc, 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, pTexture);
				break;
			}

			if (isVisible[i]) {
				glBindVertexArray(pomocVAO);
				glBindBuffer(GL_ARRAY_BUFFER, pomocVBO);
				glDrawArrays(GL_TRIANGLES, i * 6, 6);
			}
		}
	}
	bool animationCompleted = true;
	for (int i = 0; i < 5; i++) {
		if (isVisible[i]) {
			animationCompleted = false;
			break;
		}
	}

	if (animationCompleted) {
		float initialOffsetY[5] = { -0.2f, -0.2f, -0.2f, -0.2f, -0.2f }; 
		bool initialIsVisible[5] = { true, false, false, false, false }; 
		for (int i = 0; i < 5; i++) {
			offsetY[i] = initialOffsetY[i];
			isVisible[i] = initialIsVisible[i];
			mouseClickedOnFire = false;
			startX = 0.0f;
			startY = -0.2f;
		}
	}
}

void setSkyAndStars(unsigned int starVAO, unsigned int starShaderProgram) {
	if (sunIsSet) {
		glClearColor(0.01, 0.1, 0.2, 1);
		glUseProgram(starShaderProgram);

		glUniform1f(glGetUniformLocation(starShaderProgram, "time"), glfwGetTime() * timeFactor);
		glUniform3f(glGetUniformLocation(starShaderProgram, "skyColor"), 0.01, 0.1, 0.2);

		glBindVertexArray(starVAO);
		glDrawArrays(GL_POINTS, 0, STAR_COUNT);
	}
	else {
		glClearColor(0.529, 0.808, 0.922, 1);

	}
}

void setClouds(unsigned int *cloudVAO,unsigned int cloudShaderProgram) {

	glUseProgram(cloudShaderProgram);

	unsigned int ambientLightLocation = glGetUniformLocation(cloudShaderProgram, "ambientLight");

	if (sunIsSet) {
		glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f);
	}
	else {
		glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	unsigned int cloudOffsetLocation = glGetUniformLocation(cloudShaderProgram, "offset");
	unsigned int cloudColorLocation = glGetUniformLocation(cloudShaderProgram, "color");
	glUniform4f(cloudColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);

	updateClouds();

	for (int i = 0; i < 3; ++i) {
		Cloud& cloud = clouds[i];
		glUniform2f(cloudOffsetLocation, cloud.x, cloud.y);
		glBindVertexArray(cloudVAO[i]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));
	}
}

bool down = false;

void updateSunPosition(unsigned int sunShaderProgram) {

	if (!down) {
		sunOffsetX = radiusX + 0.9f * sin(angle); 
		sunOffsetY = radiusY + 0.9f * sin(angle);
	}
	else {
		sunOffsetX = sin(angle) + 0.9f * sin(angle);
		sunOffsetY = -sin(angle) - 0.9f * sin(angle);
	}

	angle += angleSpeed;

	if (angle > M_PI / 2.0f && !down) {
		down = true;  
		angle = 0;
	}
	if (angle > M_PI/ 2.0f && down) {
		angle = 0;  
		down = false; 
		sunIsSet = true;
	}

	glUseProgram(sunShaderProgram);
	unsigned int ambientLightLocation = glGetUniformLocation(sunShaderProgram, "ambientLight");
	if (sunIsSet) {
		glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f);
	}
	else {
		glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	unsigned int sunOffsetLocation = glGetUniformLocation(sunShaderProgram, "offset");
	glUniform2f(sunOffsetLocation, sunOffsetX, sunOffsetY);

	unsigned int sunColorLocation = glGetUniformLocation(sunShaderProgram, "color");
	glUniform4f(sunColorLocation, 1.0f, 1.0f, 0.0f, 1.0f);
}

void updateMoonPosition(unsigned int sunShaderProgram) {

	if (!down) {
		sunOffsetX = -radiusX - 0.9f * sin(angle);  
		sunOffsetY = radiusY + 0.9f * sin(angle);
	}
	else {
		sunOffsetX = -sin(angle) - 0.9f * sin(angle);
		sunOffsetY = -sin(angle) - 0.9f * sin(angle);
	}

	angle += angleSpeed;

	if (angle > M_PI / 2.0f && !down) {
		down = true;
		angle = 0;
	}
	if (angle > M_PI / 2.0f && down) {
		angle = 0;
		down = false;
		sunIsSet = false;
	}

	glUseProgram(sunShaderProgram);
	unsigned int ambientLightLocation = glGetUniformLocation(sunShaderProgram, "ambientLight");
	if (sunIsSet) {
		glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f);
	}
	else {
		glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	unsigned int sunOffsetLocation = glGetUniformLocation(sunShaderProgram, "offset");
	glUniform2f(sunOffsetLocation, sunOffsetX, sunOffsetY);

	unsigned int sunColorLocation = glGetUniformLocation(sunShaderProgram, "color");
	glUniform4f(sunColorLocation, 0.98f, 0.95f, 0.88f, 1.0f);

}


bool isPointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy) {
	float denominator = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy);
	float alpha = ((by - cy) * (px - cx) + (cx - bx) * (py - cy)) / denominator;
	float beta = ((cy - ay) * (px - cx) + (ax - cx) * (py - cy)) / denominator;
	float gamma = 1.0f - alpha - beta;

	return (alpha >= 0 && beta >= 0 && gamma >= 0);
}

bool isClickOnFire(float clickX, float clickY, float fireVertices[]) {
	float ax = fireVertices[0], ay = fireVertices[1];
	float bx = fireVertices[2], by = fireVertices[3];
	float cx = fireVertices[4], cy = fireVertices[5];

	return isPointInTriangle(clickX, clickY, ax, ay, bx, by, cx, cy);
}

void handleMouseClick(float clickX, float clickY, Island* islands, int numIslands, float fireVertices[]) {
	if (isClickOnFire(clickX, clickY, fireVertices)) {
		mouseClickedOnFire = true;

	}
	else if (isClickOnWater(clickX, clickY, islands, numIslands)) {
		mouseClickedOnWater = true;

	}
}

bool isClickOnWater(float clickX, float clickY, Island* islands, int numIslands) {
	for (int i = 0; i < numIslands; ++i) {
		if (isClickOnIsland(clickX, clickY, islands[i])) {
			return false;
		}
	}
	return true;
}

bool isClickOnIsland(float clickX, float clickY, const Island& island) {
	float dx = clickX - island.x;
	float dy = clickY - island.y;
	return (dx * dx + dy * dy) <= (island.radius * island.radius);
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		int windowWidth, windowHeight;
		glfwGetWindowSize(window, &windowWidth, &windowHeight);

		clickX = (xpos / windowWidth) * 2.0 - 1.0;
		clickY = -((ypos / windowHeight) * 2.0 - 1.0);

		clickTime = glfwGetTime()*timeFactor;

		Island islands[] = {
			Island(0.5f, 0.0f, 0.1f),  
			Island(0.3f, 0.8f, 0.1f),  
			Island(0.4f, -0.8f, 0.1f)
		};

		int numIslands = sizeof(islands) / sizeof(islands[0]);

		float fireVertices[] = {
		-0.3f - 0.1f, -0.1f,
		-0.1f + 0.1f, -0.1f,
		-0.2f,  0.2f - 0.8f           
		};

		handleMouseClick(clickX, clickY, islands, numIslands, fireVertices);
	}
}

static unsigned loadImageToTexture(const char* filePath) {
	int TextureWidth;
	int TextureHeight;
	int TextureChannels;
	unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
	if (ImageData != NULL)
	{
		stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

		GLint InternalFormat = -1;
		switch (TextureChannels) {
		case 1: InternalFormat = GL_RED; break;
		case 2: InternalFormat = GL_RG; break;
		case 3: InternalFormat = GL_RGB; break;
		case 4: InternalFormat = GL_RGBA; break;
		default: InternalFormat = GL_RGB; break;
		}

		unsigned int Texture;
		glGenTextures(1, &Texture);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(ImageData);
		return Texture;
	}
	else
	{
		std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
		stbi_image_free(ImageData);
		return 0;
	}
}

void bindCircleData(unsigned int VAO, unsigned int VBO, float* data, size_t dataSize) {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void generateCircle(float* circle, int offset, float r, float centerX, float centerY) {

	circle[offset] = centerX;
	circle[offset + 1] = centerY;

	for (int i = 0; i <= CRES; i++) {
		circle[offset + 2 + 2 * i] = r * cos((3.141592 / 180) * (i * 360 / CRES)) + centerX; // Xi
		circle[offset + 2 + 2 * i + 1] = r * sin((3.141592 / 180) * (i * 360 / CRES)) + centerY; // Yi
	}
}

void generateRounderCircle(float* circle, int offset, float r, float centerX, float centerY, float aspectRatio) {
	int index = offset;

	for (int i = 0; i <= CRES; i++) {
		float angle = (2.0f * 3.141592f * i) / CRES; 

		float x = r * cos(angle) + centerX; 
		float y = r * sin(angle) + centerY;

		if (aspectRatio > 1.0f) {
			y *= aspectRatio;
		}
		else if (aspectRatio < 1.0f) {
			x *= aspectRatio;
		}

		circle[index++] = x; 
		circle[index++] = y;
	}
}

unsigned int compileShader(GLenum type, const char* source)
{
	std::string content = "";
	std::ifstream file(source);
	std::stringstream ss;
	if (file.is_open())
	{
		ss << file.rdbuf();
		file.close();
		std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
	}
	else {
		ss << "";
		std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
	}
	std::string temp = ss.str();
	const char* sourceCode = temp.c_str(); //Izvorni kod sejdera koji citamo iz fajla na putanji "source"

	int shader = glCreateShader(type); //Napravimo prazan sejder odredjenog tipa (vertex ili fragment)

	int success; //Da li je kompajliranje bilo uspjesno (1 - da)
	char infoLog[512]; //Poruka o gresci (Objasnjava sta je puklo unutar sejdera)
	glShaderSource(shader, 1, &sourceCode, NULL); //Postavi izvorni kod sejdera
	glCompileShader(shader); //Kompajliraj sejder

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success); //Provjeri da li je sejder uspjesno kompajliran
	if (success == GL_FALSE)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog); //Pribavi poruku o gresci
		if (type == GL_VERTEX_SHADER)
			printf("VERTEX");
		else if (type == GL_FRAGMENT_SHADER)
			printf("FRAGMENT");
		printf(" sejder ima gresku! Greska: \n");
		printf(infoLog);
	}
	return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
	//Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource

	unsigned int program; //Objedinjeni sejder
	unsigned int vertexShader; //Verteks sejder (za prostorne podatke)
	unsigned int fragmentShader; //Fragment sejder (za boje, teksture itd)

	program = glCreateProgram(); //Napravi prazan objedinjeni sejder program

	vertexShader = compileShader(GL_VERTEX_SHADER, vsSource); //Napravi i kompajliraj vertex sejder
	fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource); //Napravi i kompajliraj fragment sejder

	//Zakaci verteks i fragment sejdere za objedinjeni program
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program); //Povezi ih u jedan objedinjeni sejder program
	glValidateProgram(program); //Izvrsi provjeru novopecenog programa

	int success;
	char infoLog[512];
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success); //Slicno kao za sejdere
	if (success == GL_FALSE)
	{
		glGetShaderInfoLog(program, 512, NULL, infoLog);
		std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
		std::cout << infoLog << std::endl;
	}

	//Posto su kodovi sejdera u objedinjenom sejderu, oni pojedinacni programi nam ne trebaju, pa ih brisemo zarad ustede na memoriji
	glDetachShader(program, vertexShader);
	glDeleteShader(vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(fragmentShader);

	return program;
}