// Autor: Nedeljko Tesanovic
// Opis: Zestoko iskomentarisan program koji crta sareni trougao u OpenGL-u

#define _CRT_SECURE_NO_WARNINGS
#define CRES 30 // Circle Resolution = Rezolucija kruga
#define M_PI 3.14159265358979323846  // Definišemo PI ako nije dostupno

 //Biblioteke za stvari iz C++-a (unos, ispis, fajlovi, itd - potrebne za kompajler sejdera) 
#include <iostream>
#include <fstream>
#include <sstream>

//Biblioteke OpenGL-a
#include <GL/glew.h>   //Omogucava laksu upotrebu OpenGL naredbi
#include <GLFW/glfw3.h>//Olaksava pravljenje i otvaranje prozora (konteksta) sa OpenGL sadrzajem
#include <thread> // Za sleep_for


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct Island {
	float radius;  // Radijus ostrva
	float x;       // X koordinata ostrva
	float y;       // Y koordinata ostrva

	// Konstruktor za inicijalizaciju članova strukture
	Island(float r, float xPos, float yPos)
		: radius(r), x(xPos), y(yPos) {}
};

struct Cloud{
	float x, y;       // Pozicija oblaka
	float speed;      // Brzina pomeranja oblaka
	float radius;     // Radijus oblaka (ako koristite tri kruga za oblak, ovo može biti veličina)

	Cloud(float startX, float startY, float speed, float radius)
		: x(startX), y(startY), speed(speed), radius(radius) {}
	Cloud() : x(0.0f), y(0.0f), speed(0.01f), radius(0.1f) {}
};

Cloud clouds[3];  // Staticki niz od 3 oblaka


float angleSpeed = 0.0003f;   // Brzina pomeranja ugla
float radiusX = -0.9f;        // Poluprečnik putanje (polukrug)
float radiusY = -0.9f;        // Poluprečnik putanje (polukrug)
float sunOffsetX = -0.9f;  // Početna pozicija X (donja leva ivica gornje polovine)
float sunOffsetY = -0.9f;  // Početna pozicija Y (donja leva ivica gornje polovine)
float angle = 0.0f;  // Početni ugao


bool mouseClickedOnWater = false;
float clickX = 0.0f;
float clickY = 0.0f;
float clickTime = 0.0f;
float maxRadius = 0.5f;  // Maksimalni radijus kruga

bool sunIsSet = false;

bool mouseClickedOnFire = false;
float startX = 0.0f;
float startY = -0.2f; // Početna pozicija Y za objekat
float offsetY[5] = { -0.2f, -0.2f, -0.2f, -0.2f, -0.2f}; // Offset Y for each letter
bool isVisible[5] = { true, false, false, false, false }; // Visibility for each letter



float flameLightColor[] = { 1.0f, 0.0f, 0.0f };     // Boja svetla (r, g, b)
float flameLightIntensity = 0.1f;

float flameRadius = 0.1f; // Promenljivi poluprečnik
float flameAngle = 0; // Ugao koji se menja tokom vremena

// Koordinate centra plamena
float flameCenterX = -0.25f;
float flameCenterY = 0.25f;

int numPoints = 10;
float flameLightPosition[20]; // Za pozicije svetla

float timeFactor = 1.0f;  // Početna brzina (normalna brzina)
float initialTimeFactor = 1.0f;  // Početna vrednost za resetovanje

unsigned int compileShader(GLenum type, const char* source); //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
unsigned int createShader(const char* vsSource, const char* fsSource); //Pravi objedinjeni sejder program koji se sastoji od Verteks sejdera ciji je kod na putanji vsSource i Fragment sejdera na putanji fsSource
void generateCircle(float* circle, int offset, float r, float centerX, float centerY);
void bindCircleData(unsigned int VAO, unsigned int VBO, float* data, size_t dataSize);
static unsigned loadImageToTexture(const char* filePath); //Ucitavanje teksture, izdvojeno u funkciju
void mouse_callback(GLFWwindow* window, int button, int action, int mods);
void setUniforms(GLuint shaderProgram);
bool isClickOnWater(float clickX, float clickY, const Island* islands, int numIslands);
//void handleMouseClick(float clickX, float clickY, Island* islands, int numIslands);
void updateClouds();
void updateSunPosition(unsigned int sunShaderProgram);
void updateMoonPosition(unsigned int sunShaderProgram);
void generateRounderCircle(float* circle, int offset, float r, float centerX, float centerY, float aspectRatio);

bool isPointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy);
bool isClickOnFire(float clickX, float clickY, float fireVertices[]);
bool isClickOnWater(float clickX, float clickY, Island* islands, int numIslands);
bool isClickOnIsland(float clickX, float clickY, const Island& island);
void handleMouseClick(float clickX, float clickY, Island* islands, int numIslands, float fireVertices[]);
void mouse_callback(GLFWwindow* window, int button, int action, int mods);

void updateFlameLight(float flameSize, float time);
void decreaseTimeSpeed();
void increaseTimeSpeed();
void resetTime();



int main(void)
{

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ INICIJALIZACIJA ++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// Pokretanje GLFW biblioteke
	// Nju koristimo za stvaranje okvira prozora
	if (!glfwInit()) // !0 == 1; glfwInit inicijalizuje GLFW i vrati 1 ako je inicijalizovana uspjesno, a 0 ako nije
	{
		std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
		return 1;
	}

	//Odredjivanje OpenGL verzije i profila (3.3, programabilni pajplajn)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Stvaranje prozora
	GLFWwindow* window; //Mjesto u memoriji za prozor
	//unsigned int wWidth = 1000;
	//unsigned int wHeight = 600;
	const char wTitle[] = "[Island]";

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

	float aspectRatio = (float)mode->width / (float)mode->height;

	float heightLimit = (float)mode->height / 2.0f; // Halfway point of the screen (for disappearing)



	window = glfwCreateWindow(mode->width, mode->height, wTitle, primaryMonitor, NULL); // Napravi novi prozor
	//window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL); // Napravi novi prozor
	// glfwCreateWindow( sirina, visina, naslov, monitor na koji ovaj prozor ide preko citavog ekrana (u tom slucaju umjesto NULL ide glfwGetPrimaryMonitor() ), i prozori sa kojima ce dijeliti resurse )
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


	unsigned int islandsShaderProgram = createShader("island.vert", "island.frag"); // Pretpostavljamo da ovo pravi shader program sa uniformom `offset`
	unsigned int sunShaderProgram = createShader("sun.vert", "basic.frag"); // Pretpostavljamo da ovo pravi shader program sa uniformom `offset`
	unsigned int palmShaderProgram = createShader("palm.vert", "basic.frag"); // Pretpostavljamo da ovo pravi shader program sa uniformom `offset`
	unsigned int fireShaderProgram = createShader("fire.vert", "basic.frag"); // Pretpostavljamo da ovo pravi shader program sa uniformom `offset`
	unsigned int waterShaderProgram = createShader("basic.vert", "basic.frag"); // Pretpostavljamo da ovo pravi shader program sa uniformom `offset`
	unsigned int sharkShaderProgram = createShader("shark.vert", "basic.frag");
	unsigned int redCircleShaderProgram = createShader("basic.vert", "basic.frag");
	unsigned int cloudShaderProgram = createShader("cloud.vert", "basic.frag");
	unsigned int starShaderProgram = createShader("star.vert", "star.frag");
	unsigned int nameShaderProgram = createShader("name.vert", "name.frag");
	unsigned int pomocShaderProgram = createShader("name.vert", "pomoc.frag");

	unsigned int VAO[8]; // Jedan VAO za svaki krug, pamlu i vatru
	unsigned int VBO[8]; // Jedan VBO za svaki krug, palmu i vatru
	glGenVertexArrays(8, VAO);
	glGenBuffers(8, VBO);

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


	float r1 = 0.5f; // Poluprečnik prvog kruga
	float r2 = 0.3f; // Poluprečnik drugog kruga
	float r3 = 0.3f; // Poluprečnik trećeg kruga
	float r_colod = 0.1f;
	
	Island islands[] = { {r1, 0.0f, 0.0f}, {r2,0.8f, 0.0f}, {r3, -0.8f, 0.0f} };

	float circle1[(CRES + 2) * 2];
	generateRounderCircle(circle1, 0, 0.15f, 0.0f, 0.0f, aspectRatio); // Sunce
	bindCircleData(VAO[0], VBO[0], circle1, sizeof(circle1));

	float circle2[(CRES + 2) * 2];
	generateCircle(circle2, 0, r1, 0.0f, 0.0f); // Prvi krug
	bindCircleData(VAO[1], VBO[1], circle2, sizeof(circle2));

	float circle3[(CRES + 2) * 2];
	generateCircle(circle3, 0, r2, 0.8f, 0.0f); // Drugi krug
	bindCircleData(VAO[2], VBO[2], circle3, sizeof(circle3));

	float circle4[(CRES + 2) * 2];
	generateCircle(circle4, 0, r3, -0.8f, 0.0f); // Treći krug
	bindCircleData(VAO[3], VBO[3], circle4, sizeof(circle4));

	float circle5[(CRES + 2) * 2];
	generateRounderCircle(circle5, 0, 0.1f, 0.0f, 0.0f, aspectRatio); // Mesec
	bindCircleData(VAO[7], VBO[7], circle5, sizeof(circle5));

	unsigned int cloudVAO[3]; // Jedan VAO za svaki krug, pamlu i vatru
	unsigned int cloudVBO[3]; // Jedan VBO za svaki krug, palmu i vatru
	glGenVertexArrays(3, cloudVAO);
	glGenBuffers(3, cloudVBO);


	float cloud1[(CRES + 2) * 2];
	generateCircle(cloud1, 0, r_colod, 0.0f, 0.0f); // oblaci
	bindCircleData(cloudVAO[0], cloudVBO[0], cloud1, sizeof(cloud1));

	float cloud2[(CRES + 2) * 2];
	generateCircle(cloud2, 0, r_colod, 0.0f, 0.0f);
	bindCircleData(cloudVAO[1], cloudVBO[1], cloud2, sizeof(cloud2));

	float cloud3[(CRES + 2) * 2];
	generateCircle(cloud3, 0, r_colod, 0.0f, 0.0f);
	bindCircleData(cloudVAO[2], cloudVBO[2], cloud3, sizeof(cloud3));

	clouds[0] = Cloud(-1.0f, 0.8f, 0.0001f, 0.1f);  // Prvi oblak
	clouds[1] = Cloud(-0.5f, 0.8f, 0.00015f, 0.12f); // Drugi oblak
	clouds[2] = Cloud(-0.8f, 0.8f, 0.0002f, 0.08f);  // Treći oblak



	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++

	glClearColor(0, 0, 1, 1); //Podesavanje boje pozadine: RGBA (R - Crvena, G - Zelena, B - Plava, A = neprovidno; Opseg od 0 do 1, gdje je 0 crno a 1 svijetlo)

	float palmVertices[] = {
		0.03f + 0.2f, -0.4f + 0.3f,  // Donji desni ugao
		-0.03f + 0.2f, -0.4f + 0.3f, // Donji levi ugao
		0.03f + 0.2f,  0.4f + 0.3f,  // Gornji desni ugao

		-0.03f + 0.2f, -0.4f + 0.3f, // Donji levi ugao
		-0.03f + 0.2f,  0.4f + 0.3f, // Gornji levi ugao
		0.03f + 0.2f,  0.4f + 0.3f,

		// Leaf 1 - Levo
		0.0f + 0.2f,  0.4f + 0.3f,  // Centar
		-0.2f + 0.2f,  0.2f + 0.3f,  // Levo
		-0.1f + 0.2f,  0.1f + 0.3f,  // Levo dalje

		// Leaf 2 - Desno
		0.0f + 0.2f,  0.4f + 0.3f,  // Centar
		0.2f + 0.2f,  0.2f + 0.3f,  // Desno
		0.1f + 0.2f,  0.1f + 0.3f,  // Desno dalje

		// Leaf 3 - Između levo i desno, ali niže
		0.0f + 0.2f,  0.4f + 0.3f,  // Centar
		-0.05f + 0.2f,  0.1f + 0.3f,  // Niže, između levo i desno
		0.05f + 0.2f,  0.1f + 0.3f   // Gore Desno
	};

	float fireVertices[] = {
	-0.3f, -0.1f,
	-0.1f, -0.1f,
	-0.2f,  0.2f
	};


	// Postavljanje za palmu
	glBindVertexArray(VAO[4]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(palmVertices), palmVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); // Koordinate
	glEnableVertexAttribArray(0);

	glBindVertexArray(VAO[5]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fireVertices), fireVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); // Koordinate
	glEnableVertexAttribArray(0);


	const int numSharks = 4; // Broj ajkula
	const float centerX = 0.0f;
	const float centerY = -0.2f;
	r1 = 0.7;

	float sharkPositions[numSharks][2]; // Trenutne pozicije ajkula (x, y)
	float sharkDirections[numSharks][2]; // Pravci kretanja ajkula (x, y)
	bool sharksMoving[numSharks]; // Da li se ajkula kreće
	float sharkSpeed = 0.001f; // Brzina ajkula

	float initialSharkPositions[numSharks][2]; // Početne pozicije ajkula
	for (int i = 0; i < numSharks; i++) {
		initialSharkPositions[i][0] = r1 * cos((2.0f * 3.141592f / numSharks) * i) + centerX;
		initialSharkPositions[i][1] = r1 * sin((2.0f * 3.141592f / numSharks) * i) + centerY;
	}

	for (int i = 0; i < numSharks; i++) {
		sharkPositions[i][0] = r1 * cos((2.0f * 3.141592f / numSharks) * i) + centerX;
		sharkPositions[i][1] = r1 * sin((2.0f * 3.141592f / numSharks) * i) + centerY;
		sharksMoving[i] = false; // Početno stanje: ajkule miruju
	}

	float sharkTemplate[] = {
	  -0.075f, -0.0475f, 0.0f,  // Donji levi (uvećano)
	 0.075f, -0.0475f, 0.0f,  // Donji desni (uvećano)
	 0.0f,   0.175f,   0.0f   // Gornji vrh (uvećano)
	};

	// Vertex podaci za sve ajkule
	float sharkVertices[numSharks * 9]; // 3 verteksa po ajkuli * 3 komponente (x, y, z)

	for (int i = 0; i < numSharks; i++) {
		float angle = (2.0f * 3.141592f / numSharks) * i; // Ugao za trenutnu ajkulu
		float offsetX = r1 * cos(angle) + centerX; // X koordinata
		float offsetY = r1 * sin(angle) + centerY; // Y koordinata

		// Dodajte pomeraj za ajkule koje se nalaze sa prednje strane
		if (angle >= 0.0f && angle <= 3.141592f) { // Ugao od 0 do 180 stepeni (prednja strana)
			offsetY -= 0.1f; // Spustite ajkule na prednjem delu kruga (pomeraj prema dole)
		}

		// Ažuriranje verteksa za trenutnu ajkulu
		for (int j = 0; j < 9; j += 3) {
			sharkVertices[i * 9 + j] = sharkTemplate[j] + offsetX;  // X koordinata
			sharkVertices[i * 9 + j + 1] = sharkTemplate[j + 1] + offsetY;  // Y koordinata
			sharkVertices[i * 9 + j + 2] = sharkTemplate[j + 2];  // Z koordinata
		}
	}

	unsigned int sharksVAO, sharksVBO;
	glGenVertexArrays(1, &sharksVAO);
	glGenBuffers(1, &sharksVBO);

	glBindVertexArray(sharksVAO);

	glBindBuffer(GL_ARRAY_BUFFER, sharksVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sharkVertices), sharkVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	bool waterTransparencyEnabled = false; // Početno stanje
	bool bKeyPressed = false; // Da li je taster trenutno pritisnut

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	int width, height, channels;
	unsigned char* image= stbi_load("images/udica.png", &width, &height, &channels, 4);

	GLFWimage cursorImage;
	cursorImage.width = width;
	cursorImage.height = height;
	cursorImage.pixels = image; // Pixel podaci

	GLFWcursor* cursor = glfwCreateCursor(&cursorImage, 0, 0);

	glfwSetMouseButtonCallback(window, mouse_callback);

	float r5 = 0.05f;  // poluprecnik crvenog kruga
	float r5pom = 0.05f;  // poluprecnik crvenog kruga


	const int STAR_COUNT = 100;
	float starVertices[STAR_COUNT * 3]; // X, Y, i Veličina za svaku zvezdu

	for (int i = 0; i < STAR_COUNT; i++) {
		// X i Y između -1 i 1
		starVertices[i * 3] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;    // X
		starVertices[i * 3 + 1] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; // Y

		// Veličina između 3 i 8
		starVertices[i * 3 + 2] = ((float)rand() / RAND_MAX) * 5.0f + 3.0f; // Veličina između 3 i 8
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
		// Koordinate (X, Y)        // Teksturne koordinate (S, T)
		-1.0f, -1.0f,   0.0f, 0.0f,  // Donji desni ugao
		-1.0f + 0.3f, -1.0f,  1.0f, 0.0f,  // Donji levi ugao (širina povećana)
		-1.0f, -1.0f + 0.1f,  0.0f, 1.0f,  // Gornji desni ugao (visina smanjena)

		-1.0f + 0.3f, -1.0f,          1.0f, 0.0f,  // Donji levi ugao (širina povećana)
		-1.0f + 0.3f, -1.0f + 0.1f,    1.0f, 1.0f,  // Gornji levi ugao (visina smanjena)
		-1.0f, -1.0f + 0.1f,          0.0f, 1.0f   // Gornji desni ugao (visina smanjena)
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

	// Prvo vezujemo VAO (Vertex Array Object)
	glBindVertexArray(pomocVAO);

	// Zatim vezujemo VBO (Vertex Buffer Object)
	glBindBuffer(GL_ARRAY_BUFFER, pomocVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pomocVertices), pomocVertices, GL_STATIC_DRAW);

	// Definišemo kako se podaci (vertices) interpretiraju
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Otpajanje VBO i VAO
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



	while (!glfwWindowShouldClose(window)) // Infinite loop
	{
		// User input (Escape to close the window)
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			decreaseTimeSpeed();
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			increaseTimeSpeed();
		}
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		{
			resetTime();
		}

		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && waterTransparencyEnabled) {
			waterTransparencyEnabled = false;
		}
		else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !waterTransparencyEnabled) {
			waterTransparencyEnabled = true;

		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glfwSetCursor(window, cursor);

		// Getting window dimensions
		int wWidth = mode->width;
		int wHeight = mode->height;

		glEnable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST);


		// --- Top half of the screen (Sky blue) ---
		glViewport(0, wHeight / 2, wWidth, wHeight / 2); // Set viewport for the top half
		glScissor(0, wHeight / 2, wWidth, wHeight / 2); // Restrict drawing to the top half

		if (sunIsSet) {
			glClearColor(0.01, 0.1, 0.2, 1); // Još tamnija nijansa plave
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(starShaderProgram); // Šejder program za zvezde

			glUniform1f(glGetUniformLocation(starShaderProgram, "time"), glfwGetTime()*timeFactor);
			glUniform3f(glGetUniformLocation(starShaderProgram, "skyColor"), 0.01, 0.1, 0.2);

			glBindVertexArray(starVAO);
			glDrawArrays(GL_POINTS, 0, STAR_COUNT);
		}
		else {
			glClearColor(0.529, 0.808, 0.922, 1); // Sky blue color
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		}



		glUseProgram(cloudShaderProgram);

		unsigned int ambientLightLocation = glGetUniformLocation(cloudShaderProgram, "ambientLight");

		if (sunIsSet) {
			glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); // Blago tamnija svetlost
		}
		else {
			glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f); // Pun sjaj dok je sunce
		}

		unsigned int cloudOffsetLocation = glGetUniformLocation(cloudShaderProgram, "offset");
		unsigned int cloudColorLocation = glGetUniformLocation(cloudShaderProgram, "color");
		glUniform4f(cloudColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);

		updateClouds();

		for (int i = 0; i < 3; ++i) {
			Cloud& cloud = clouds[i];
			glUniform2f(cloudOffsetLocation, cloud.x, cloud.y);
			glBindVertexArray(cloudVAO[i]);
			glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));  // Crtanje svakog oblaka (tri kruga)
		}

		if (!sunIsSet) {
			updateSunPosition(sunShaderProgram);
			glBindVertexArray(VAO[0]);
			glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2)); // Prvi krug, tj sunce
		}
		else {
			updateMoonPosition(sunShaderProgram);
			glBindVertexArray(VAO[7]);
			glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));
			//mesec izlazi
		}

		glViewport(0, 0, wWidth, wHeight / 2); // Set viewport for the bottom half
		glScissor(0, 0, wWidth, wHeight / 2); // Restrict drawing to the bottom half

		glUseProgram(nameShaderProgram);
		unsigned uTexLoc = glGetUniformLocation(nameShaderProgram, "uTex");
		glUniform1i(uTexLoc, 0);

		glActiveTexture(GL_TEXTURE0); //tekstura koja se bind-uje nakon ovoga ce se koristiti sa SAMPLER2D uniformom u sejderu koja odgovara njenom indeksu
		glBindTexture(GL_TEXTURE_2D, nameTexture);

		glBindVertexArray(nameVAO);
		glBindBuffer(GL_ARRAY_BUFFER, nameVBO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		

		glUseProgram(palmShaderProgram);

		ambientLightLocation = glGetUniformLocation(palmShaderProgram, "ambientLight");
		if (sunIsSet) {
			glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); // Blago tamnija svetlost
		}
		else {
			glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f); // Pun sjaj dok je sunce
		}

		unsigned int palmColorLocation = glGetUniformLocation(palmShaderProgram, "color");

		glUniform4f(palmColorLocation, 0.0f, 0.5f, 0.0f, 1.0f); // Tamnija zelena

		glBindVertexArray(VAO[4]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);

		glDrawArrays(GL_TRIANGLES, 6, 3);
		glDrawArrays(GL_TRIANGLES, 9, 3);
		glDrawArrays(GL_TRIANGLES, 12, 3);


		glUniform4f(palmColorLocation, 0.545f, 0.271f, 0.075f, 1.0f); // Braon boja
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glUseProgram(fireShaderProgram);

		ambientLightLocation = glGetUniformLocation(fireShaderProgram, "ambientLight");
		if (sunIsSet) {
			glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); // Blago tamnija svetlost
		}
		else {
			glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f); // Pun sjaj dok je sunce
		}

		float fireSize = sin(glfwGetTime()* timeFactor) * 1.0f + 1.5f;

		unsigned int fireSizeLocation = glGetUniformLocation(fireShaderProgram, "scaleY");
		glUniform1f(fireSizeLocation, fireSize);

		float green = sin(glfwGetTime() * timeFactor * 3.0f) * 0.3f + 0.3f;

		unsigned int fireColorLocation = glGetUniformLocation(fireShaderProgram, "color");
		glUniform4f(fireColorLocation, 1.0f, green, 0.0f, 1.0f);

		glBindVertexArray(VAO[5]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		float waterLevel = abs(sin(glfwGetTime()*timeFactor)) * 0.3f;

		for (int i = 0; i < numSharks; i++) {
			if (sharksMoving[i]) {
				// Pomeranje ajkule prema trenutnom pravcu
				sharkPositions[i][0] += sharkDirections[i][0] * sharkSpeed;
				sharkPositions[i][1] += sharkDirections[i][1] * sharkSpeed;

				// Proverite da li je ajkula unutar radijusa ostrva
				float dx = sharkPositions[i][0] + 0.2f;
				float dy = sharkPositions[i][1] + 0.2f;
				float distanceFromIsland = sqrt(dx * dx + dy * dy);

				if (distanceFromIsland < 0.5f + 0.2f) { // Dodatna margina za izbegavanje
					// Izračunavanje novog pravca za izbegavanje ostrva
					float avoidDirectionX = dx / distanceFromIsland;
					float avoidDirectionY = dy / distanceFromIsland;

					// Ažuriranje pravca ajkule da zaobiđe ostrvo
					sharkDirections[i][0] = avoidDirectionX;
					sharkDirections[i][1] = avoidDirectionY;

					// Pomerite ajkulu u pravcu izbegavanja
					sharkPositions[i][0] += avoidDirectionX * sharkSpeed;
					sharkPositions[i][1] += avoidDirectionY * sharkSpeed;
				}

				// Proverite da li je ajkula stigla do cilja (kruga)
				dx = clickX - sharkPositions[i][0];
				dy = clickY - sharkPositions[i][1];
				float distanceToCircle = sqrt(dx * dx + dy * dy);
				if (distanceToCircle <= maxRadius) {
					sharksMoving[i] = false; // Ajkula stiže do cilja
				}
			}
		}



		glUseProgram(sharkShaderProgram);

		ambientLightLocation = glGetUniformLocation(sharkShaderProgram, "ambientLight");
		if (sunIsSet) {
			glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); // Blago tamnija svetlost
		}
		else {
			glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f); // Pun sjaj dok je sunce
		}

		unsigned int sharkColorLocation = glGetUniformLocation(sharkShaderProgram, "color");
		glUniform4f(sharkColorLocation, 0.0f, 0.03, 1.0f, 1.0f);

		unsigned int waterLevelLocation = glGetUniformLocation(sharkShaderProgram, "waterLevel");
		glUniform1f(waterLevelLocation, waterLevel);

		unsigned int sharkTimeLocation = glGetUniformLocation(sharkShaderProgram, "time");
		glUniform1f(sharkTimeLocation, glfwGetTime()*timeFactor);

		unsigned int sharkSpeedLocation = glGetUniformLocation(sharkShaderProgram, "speed");
		glUniform1f(sharkSpeedLocation, 0.8f);

		float sharkVertices[numSharks * 9];
		for (int i = 0; i < numSharks; i++) {
			float offsetX = sharkPositions[i][0];
			float offsetY = sharkPositions[i][1];
			for (int j = 0; j < 9; j += 3) {
				sharkVertices[i * 9 + j] = sharkTemplate[j] + offsetX;
				sharkVertices[i * 9 + j + 1] = sharkTemplate[j + 1] + offsetY;
				sharkVertices[i * 9 + j + 2] = sharkTemplate[j + 2];
			}
		}

		// Bindovanje novih podataka
		glBindVertexArray(sharksVAO);
		glBindBuffer(GL_ARRAY_BUFFER, sharksVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(sharkVertices), sharkVertices, GL_DYNAMIC_DRAW);

		//glBindVertexArray(sharksVAO);
		//glBindBuffer(GL_ARRAY_BUFFER, sharksVBO);
		glDrawArrays(GL_TRIANGLES, 0, numSharks * 3); // 3 verteksa po ajkuli * broj ajkula



		// Use shader program to draw the bottom half circle (water)

		if (mouseClickedOnWater) {

			for (int i = 0; i < numSharks; i++) {
				float dx = clickX - sharkPositions[i][0];
				float dy = clickY - sharkPositions[i][1];
				float length = sqrt(dx * dx + dy * dy); // Udaljenost od centra
				sharkDirections[i][0] = dx / length;   // Normalizovani pravac
				sharkDirections[i][1] = dy / length;
				sharksMoving[i] = true; // Pokrenite ajkulu
			}

			float elapsedTime = glfwGetTime()*timeFactor - clickTime; // Proteklo vreme od klika
			// Generisanje podataka za krug
			float circle5[(CRES + 2) * 2];
			generateCircle(circle5, 0, r5pom, clickX, clickY);
			r5pom += 0.00004f;
			bindCircleData(VAO[6], VBO[6], circle5, sizeof(circle5));

			// Renderovanje kruga
			glUseProgram(redCircleShaderProgram);

			ambientLightLocation = glGetUniformLocation(redCircleShaderProgram, "ambientLight");
			if (sunIsSet) {
				glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); // Blago tamnija svetlost
			}
			else {
				glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f); // Pun sjaj dok je sunce
			}
			setUniforms(redCircleShaderProgram);
			glBindVertexArray(VAO[6]);
			glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));

			if (elapsedTime > 5.0) {
				mouseClickedOnWater = false;
				r5pom = r5;
			}
		}
		else {
			for (int i = 0; i < numSharks; i++) {
				float dx = initialSharkPositions[i][0] - sharkPositions[i][0];
				float dy = initialSharkPositions[i][1] - sharkPositions[i][1];
				float distance = sqrt(dx * dx + dy * dy);

				if (distance > 0.01f) { // Ako nije blizu početne pozicije
					// Proverite da li ajkula prelazi preko ostrva
					float islandDx = sharkPositions[i][0]; // Pozicija ajkule u odnosu na ostrvo
					float islandDy = sharkPositions[i][1];
					float distanceFromIsland = sqrt(islandDx * islandDx + islandDy * islandDy);

					if (distanceFromIsland < 0.5f) { // Unutar radijusa ostrva sa marginom
						// Izračunavanje pravca izbegavanja
						float avoidDirectionX = islandDx / distanceFromIsland;
						float avoidDirectionY = islandDy / distanceFromIsland;

						// Pomerite ajkulu u pravcu izbegavanja
						sharkPositions[i][0] += avoidDirectionX * sharkSpeed;
						sharkPositions[i][1] += avoidDirectionY * sharkSpeed;
					}
					else {
						// Nastavite ka početnoj poziciji ako niste blizu ostrva
						sharkPositions[i][0] += (dx / distance) * sharkSpeed;
						sharkPositions[i][1] += (dy / distance) * sharkSpeed;
					}
				}
				else {
					// Ako je stigla do početne pozicije
					sharkPositions[i][0] = initialSharkPositions[i][0];
					sharkPositions[i][1] = initialSharkPositions[i][1];
					sharksMoving[i] = false; // Zaustavite kretanje ajkula
				}
			}


		}
		

		glUseProgram(islandsShaderProgram);

		float flameTime = glfwGetTime()*timeFactor;

		updateFlameLight(fireSize, flameTime);

		unsigned int flameLightPositionLoc = glGetUniformLocation(islandsShaderProgram, "flameLightPosition");
		unsigned int flameLightColorLoc = glGetUniformLocation(islandsShaderProgram, "flameLightColor");
		unsigned int flameLightIntensityLoc = glGetUniformLocation(islandsShaderProgram, "flameLightIntensity");

		fireSizeLocation = glGetUniformLocation(islandsShaderProgram, "scaleY");
		glUniform1f(fireSizeLocation, fireSize);

		glUniform3fv(flameLightPositionLoc, 1, flameLightPosition);  // Pozicija svetla
		glUniform3fv(flameLightColorLoc, 1, flameLightColor);        // Boja svetla
		glUniform1f(flameLightIntensityLoc, flameLightIntensity);          // Intenzitet svetla


		ambientLightLocation = glGetUniformLocation(islandsShaderProgram, "ambientLight");
		if (sunIsSet) {
			glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); // Blago tamnija svetlost
		}
		else {
			glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f); // Pun sjaj dok je sunce
		}

		unsigned int islandOffsetLocation = glGetUniformLocation(islandsShaderProgram, "offset");

		unsigned int islandColorLocation = glGetUniformLocation(islandsShaderProgram, "color");
		glUniform4f(islandColorLocation, 194.0f / 255.0f, 178.0f / 255.0f, 128.0f / 255.0f, 1.0f);

		waterLevelLocation = glGetUniformLocation(islandsShaderProgram, "waterLevel");
		glUniform1f(waterLevelLocation, waterLevel);

		// Bind and draw the bottom circles

		glUniform2f(islandOffsetLocation, 0.0f, -0.2f);
		glBindVertexArray(VAO[1]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));

		glUniform2f(islandOffsetLocation, -0.1f, 0.6f);
		glBindVertexArray(VAO[2]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));

		glUniform2f(islandOffsetLocation, 0.2f, 0.5f);
		glBindVertexArray(VAO[3]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2));


		glUseProgram(waterShaderProgram);

		ambientLightLocation = glGetUniformLocation(waterShaderProgram, "ambientLight");
		if (sunIsSet) {
			glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); // Blago tamnija svetlost
		}
		else {
			glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f); // Pun sjaj dok je sunce
		}

		unsigned int waterColorLocation = glGetUniformLocation(waterShaderProgram, "color");

		if (waterTransparencyEnabled) {
			glUniform4f(waterColorLocation, 0.0f, 0.0f, 0.5f, 0.3f);  // Providna plava voda (alfa = 0.3)
		}
		else {
			glUniform4f(waterColorLocation, 0.0f, 0.0f, 0.5f, 1.0f);  // Neprozirna plava voda (alfa = 1.0)
		}



		// Renderujte vodu kao pozadinu
		glBindVertexArray(waterVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, waterVBO[0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);


		glBindVertexArray(0);
		glUseProgram(0);

		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_DEPTH_TEST);

		// Aktiviraj shader program
		glViewport(0, 0, wWidth, wHeight);  // Set viewport for the full screen

		if (mouseClickedOnFire) {
			unsigned int textTextureLoc = glGetUniformLocation(pomocShaderProgram, "textTexture");
			unsigned int smokeTextureLoc = glGetUniformLocation(pomocShaderProgram, "smokeTexture");
			unsigned int translationLoc = glGetUniformLocation(pomocShaderProgram, "translation");

			glUseProgram(pomocShaderProgram);

			// Aktiviraj i poveži dim teksturu
			glUniform1i(smokeTextureLoc, 5); // Tekstura na lokaciji 5
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, smokeTexture);

			// Pojavljivanje objekata od poslednjeg ka prvom
			// Assuming these variables are initialized

			for (int i = 0; i < 5; i++) {
				if (isVisible[i]) { // Prvi element uvek treba da bude vidljiv na početku
					// Animacija trenutnog objekta
					offsetY[i] += 0.0002f; // Povećaj Y poziciju

					if (offsetY[i] > 0.5f) { // Ako dostigne limit, sakrij ga i prikaži sledeći
						isVisible[i] = false;
						if (i + 1 < 5) { // Proveri da li postoji sledeći element
							isVisible[i + 1] = true;
						}
					}
					else if (offsetY[i] > -0.06f) {
						if (i + 1 < 5) { // Proveri da li postoji sledeći element
							isVisible[i + 1] = true;
						}
					}

					// Podesi translaciju za trenutno slovo
					glUniform2f(translationLoc, startX, startY + offsetY[i]);

					// Podesi teksturu za trenutno slovo
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
						glDrawArrays(GL_TRIANGLES, i * 6, 6); // Ispravno skaliraj indeks
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
				float initialOffsetY[5] = { -0.2f, -0.2f, -0.2f, -0.2f, -0.2f }; // Početne vrednosti za Y pomeranje
				bool initialIsVisible[5] = { true, false, false, false, false }; // Početna vidljivost
				for (int i = 0; i < 5; i++) {
					offsetY[i] = initialOffsetY[i]; // Postavi offsetY na početne vrednosti
					isVisible[i] = initialIsVisible[i];
					mouseClickedOnFire = false;
					startX = 0.0f;
					startY = -0.2f;
				}
			}

		}

		// Swap buffers to update the screen
		glfwSwapBuffers(window);

		// Handle events (keyboard, mouse, etc.)
		glfwPollEvents();
	}
	// Delete VAOs and VBOs for islands
	glDeleteVertexArrays(7, VAO);
	glDeleteBuffers(7, VBO);

	// Delete VAO and VBO for water
	glDeleteVertexArrays(1, waterVAO);
	glDeleteBuffers(1, waterVBO);

	// Delete VAOs and VBOs for clouds
	glDeleteVertexArrays(3, cloudVAO);
	glDeleteBuffers(3, cloudVBO);

	// Delete VAO and VBO for palm
	glDeleteVertexArrays(1, &VAO[4]);
	glDeleteBuffers(1, &VBO[4]);

	// Delete VAO and VBO for fire
	glDeleteVertexArrays(1, &VAO[5]);
	glDeleteBuffers(1, &VBO[5]);

	// Delete VAO and VBO for sharks
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


	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++

	//Sve OK - batali program
	glfwTerminate();
	return 0;
}


void increaseTimeSpeed() {
	timeFactor += 0.1f;
	angleSpeed += 0.00001;
	for (auto& cloud : clouds) {
		cloud.x += cloud.speed + 0.001; // Pomera oblak sa levog na desni kraj ekrana
	}
}

void decreaseTimeSpeed() {
	timeFactor -= 0.01f;  
	angleSpeed -= 0.00001;
	for (auto& cloud : clouds) {
		cloud.x += cloud.speed - 0.001; // Pomera oblak sa levog na desni kraj ekrana
	}

}

void resetTime() {
	timeFactor = initialTimeFactor;  // Vraća brzinu na početnu vrednost
	angleSpeed = 0.0003f;   // Brzina pomeranja ugla
	for (auto& cloud : clouds) {
		cloud.x += 0.01f; // Pomera oblak sa levog na desni kraj ekrana
	}
	angle = 0;
}


void updateFlameLight(float flameSize, float time) {

	flameAngle = time;  // Ugao koji se menja tokom vremena

	for (int i = 0; i < numPoints; i++) {
		flameLightPosition[2 * i] = flameCenterX + flameRadius * cos(flameAngle);  // X koordinata
		flameLightPosition[2 * i + 1] = flameCenterY + flameRadius * sin(flameAngle);  // Y koordinata
	}
}

void setUniforms(GLuint shaderProgram) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLint clickPosLocation = glGetUniformLocation(shaderProgram, "clickPosition");
	GLint timeLocation = glGetUniformLocation(shaderProgram, "time");
	GLint startTimeLocation = glGetUniformLocation(shaderProgram, "startTime");
	GLint maxRadiusLocation = glGetUniformLocation(shaderProgram, "maxRadius");
	unsigned int colorLocation = glGetUniformLocation(shaderProgram, "color");

	glUniform4f(colorLocation, 1.0f, 0.0f, 0.0f, 0.5f);
	glUniform2f(clickPosLocation, clickX, clickY);
	glUniform1f(timeLocation, glfwGetTime()*timeFactor);
	glUniform1f(startTimeLocation, clickTime);
	glUniform1f(maxRadiusLocation, maxRadius);
}


void updateClouds() {
	for (auto& cloud : clouds) {
		cloud.x += cloud.speed; // Pomera oblak sa levog na desni kraj ekrana

		if (cloud.x > 1.0f) {
			cloud.x = -1.0f - (rand() % 10) * 0.1f;
		}
	}
}

bool down = false;

void updateSunPosition(unsigned int sunShaderProgram) {

	if (!down) {
		sunOffsetX = radiusX + 0.9f * sin(angle);  // X pozicija
		sunOffsetY = radiusY + 0.9f * sin(angle);  // Y pozicija
	}
	else {
		sunOffsetX = sin(angle) + 0.9f * sin(angle);  // X pozicija
		sunOffsetY = -sin(angle) - 0.9f * sin(angle);  // Y pozicija
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
		glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); // Blago tamnija svetlost
	}
	else {
		glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f); // Pun sjaj dok je sunce
	}
	unsigned int sunOffsetLocation = glGetUniformLocation(sunShaderProgram, "offset");
	glUniform2f(sunOffsetLocation, sunOffsetX, sunOffsetY);

	unsigned int sunColorLocation = glGetUniformLocation(sunShaderProgram, "color");
	glUniform4f(sunColorLocation, 1.0f, 1.0f, 0.0f, 1.0f);
}

void updateMoonPosition(unsigned int sunShaderProgram) {

	if (!down) {
		sunOffsetX = -radiusX - 0.9f * sin(angle);  // X pozicija
		sunOffsetY = radiusY + 0.9f * sin(angle);  // Y pozicija	
	}
	else {
		sunOffsetX = -sin(angle) - 0.9f * sin(angle);  // X pozicija
		sunOffsetY = -sin(angle) - 0.9f * sin(angle);  // Y pozicija
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
		glUniform4f(ambientLightLocation, 0.6f, 0.6f, 0.7f, 1.0f); // Blago tamnija svetlost
	}
	else {
		glUniform4f(ambientLightLocation, 1.0f, 1.0f, 1.0f, 1.0f); // Pun sjaj dok je sunce
	}
	unsigned int sunOffsetLocation = glGetUniformLocation(sunShaderProgram, "offset");
	glUniform2f(sunOffsetLocation, sunOffsetX, sunOffsetY);

	unsigned int sunColorLocation = glGetUniformLocation(sunShaderProgram, "color");
	glUniform4f(sunColorLocation, 0.98f, 0.95f, 0.88f, 1.0f);

}


// Provera da li tačka leži unutar trougla
bool isPointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy) {
	float denominator = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy);
	float alpha = ((by - cy) * (px - cx) + (cx - bx) * (py - cy)) / denominator;
	float beta = ((cy - ay) * (px - cx) + (ax - cx) * (py - cy)) / denominator;
	float gamma = 1.0f - alpha - beta;

	return (alpha >= 0 && beta >= 0 && gamma >= 0);
}

// Funkcija za proveru klika na vatru
bool isClickOnFire(float clickX, float clickY, float fireVertices[]) {
	float ax = fireVertices[0], ay = fireVertices[1];
	float bx = fireVertices[2], by = fireVertices[3];
	float cx = fireVertices[4], cy = fireVertices[5];

	return isPointInTriangle(clickX, clickY, ax, ay, bx, by, cx, cy);
}

// Funkcija za obradu klika
void handleMouseClick(float clickX, float clickY, Island* islands, int numIslands, float fireVertices[]) {
	if (isClickOnFire(clickX, clickY, fireVertices)) {
		mouseClickedOnFire = true;

	}
	else if (isClickOnWater(clickX, clickY, islands, numIslands)) {
		mouseClickedOnWater = true;

	}
}

// Funkcija za proveru klika na vodu (nije ni vatru ni ostrvo)
bool isClickOnWater(float clickX, float clickY, Island* islands, int numIslands) {
	for (int i = 0; i < numIslands; ++i) {
		if (isClickOnIsland(clickX, clickY, islands[i])) {
			return false; // Klik je na ostrvu
		}
	}
	return true; // Klik je na vodi
}

// Provera da li je klik na ostrvo
bool isClickOnIsland(float clickX, float clickY, const Island& island) {
	float dx = clickX - island.x;
	float dy = clickY - island.y;
	return (dx * dx + dy * dy) <= (island.radius * island.radius); // Proveravamo da li je unutar kruga
}

// Mouse callback funkcija
void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos); // Dobijamo koordinate miša u prozorskom sistemu

		int windowWidth, windowHeight;
		glfwGetWindowSize(window, &windowWidth, &windowHeight); // Dimenzije prozora aplikacije

		clickX = (xpos / windowWidth) * 2.0 - 1.0;   // Mapiranje na [-1, 1]
		clickY = -((ypos / windowHeight) * 2.0 - 1.0); // Obrnuto Y

		clickTime = glfwGetTime()*timeFactor;  // Započni vreme kada je kliknut taster

		// Definicija ostrva
		Island islands[] = {
			Island(0.5f, 0.0f, 0.1f),  // Ostrvo 1
			Island(0.3f, 0.8f, 0.1f),  // Ostrvo 2
			Island(0.4f, -0.8f, 0.1f)  // Ostrvo 3
		};

		int numIslands = sizeof(islands) / sizeof(islands[0]); // Broj ostrva

		float fireVertices[] = {
		-0.3f - 0.1f, -0.1f, // A
		-0.1f + 0.1f, -0.1f, // B
		-0.2f,  0.2f - 0.8f             // C (spušteno za 'tolerance' na Y osi)
		};

		// Obrada klika
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
		//Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
		stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

		// Provjerava koji je format boja ucitane slike
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
		// oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
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
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	/*glBindVertexArray(0);*/
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

	// Pravite krug koristeći parametarsku formulu
	for (int i = 0; i <= CRES; i++) {
		float angle = (2.0f * 3.141592f * i) / CRES;  // Ugao u radijanima

		// Skalirajte X i Y koordinate u zavisnosti od aspect ratio
		float x = r * cos(angle) + centerX;  // X koordinata
		float y = r * sin(angle) + centerY;  // Y koordinata

		// Ako je aspectRatio > 1, ekran je širi, pa skaliraj samo Y
		if (aspectRatio > 1.0f) {
			y *= aspectRatio;
		}
		// Ako je aspectRatio < 1, ekran je viši, pa skaliraj samo X
		else if (aspectRatio < 1.0f) {
			x *= aspectRatio;
		}

		// Dodaj tačke u array
		circle[index++] = x;  // X koordinata
		circle[index++] = y;  // Y koordinata
	}
}




unsigned int compileShader(GLenum type, const char* source)
{
	//Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
	//Citanje izvornog koda iz fajla
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