﻿// Autor: Nedeljko Tesanovic
// Opis: Zestoko iskomentarisan program koji crta sareni trougao u OpenGL-u

#define _CRT_SECURE_NO_WARNINGS
#define CRES 30 // Circle Resolution = Rezolucija kruga

 //Biblioteke za stvari iz C++-a (unos, ispis, fajlovi, itd - potrebne za kompajler sejdera) 
#include <iostream>
#include <fstream>
#include <sstream>

//Biblioteke OpenGL-a
#include <GL/glew.h>   //Omogucava laksu upotrebu OpenGL naredbi
#include <GLFW/glfw3.h>//Olaksava pravljenje i otvaranje prozora (konteksta) sa OpenGL sadrzajem

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int compileShader(GLenum type, const char* source); //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
unsigned int createShader(const char* vsSource, const char* fsSource); //Pravi objedinjeni sejder program koji se sastoji od Verteks sejdera ciji je kod na putanji vsSource i Fragment sejdera na putanji fsSource
void generateCircle(float* circle, int offset, float r, float centerX, float centerY);
void bindCircleData(unsigned int VAO, unsigned int VBO, float* data, size_t dataSize);
static unsigned loadImageToTexture(const char* filePath); //Ucitavanje teksture, izdvojeno u funkciju


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
	unsigned int wWidth = 1200;
	unsigned int wHeight = 800;
	const char wTitle[] = "[Island]";

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

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
	unsigned int sunShaderProgram = createShader("sun.vert", "island.frag"); // Pretpostavljamo da ovo pravi shader program sa uniformom `offset`
	unsigned int palmShaderProgram = createShader("palm.vert", "island.frag"); // Pretpostavljamo da ovo pravi shader program sa uniformom `offset`
	unsigned int fireShaderProgram = createShader("fire.vert", "fire.frag"); // Pretpostavljamo da ovo pravi shader program sa uniformom `offset`
	unsigned int waterShaderProgram = createShader("basic.vert", "island.frag"); // Pretpostavljamo da ovo pravi shader program sa uniformom `offset`


	unsigned int VAO[6]; // Jedan VAO za svaki krug, pamlu i vatru
	unsigned int VBO[6]; // Jedan VBO za svaki krug, palmu i vatru
	glGenVertexArrays(6, VAO);
	glGenBuffers(6, VBO);

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
	float r4 = 0.1f; // Poluprečnik sunca

	float circle1[(CRES + 2) * 2];
	generateCircle(circle1, 0, r4, 0.0f, 0.0f); // Sunce
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


	bool waterTransparencyEnabled = false; // Početno stanje
	bool bKeyPressed = false; // Da li je taster trenutno pritisnut

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	int width, height, channels;
	unsigned char* image= stbi_load("C:/Users/Sonja/Desktop/udica.png", &width, &height, &channels, 4);

	GLFWimage cursorImage;
	cursorImage.width = width;
	cursorImage.height = height;
	cursorImage.pixels = image; // Pixel podaci

	GLFWcursor* cursor = glfwCreateCursor(&cursorImage, 0, 0);

	while (!glfwWindowShouldClose(window)) // Infinite loop
	{
		// User input (Escape to close the window)
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && waterTransparencyEnabled) {
			waterTransparencyEnabled = false;
		}
		else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !waterTransparencyEnabled) {
			waterTransparencyEnabled = true;

		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSetCursor(window, cursor);

		// Getting window dimensions
		int wWidth = mode->width;
		int wHeight = mode->height;

		glEnable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST);


		// --- Top half of the screen (Sky blue) ---
		glViewport(0, wHeight / 2, wWidth, wHeight / 2); // Set viewport for the top half
		glScissor(0, wHeight / 2, wWidth, wHeight / 2); // Restrict drawing to the top half
		glClearColor(0.529, 0.808, 0.922, 1); // Sky blue color

		glUseProgram(sunShaderProgram);

		unsigned int sunOffsetLocation = glGetUniformLocation(sunShaderProgram, "offset");
		glUniform2f(sunOffsetLocation, -0.8f, 0.8f);

		unsigned int sunColorLocation = glGetUniformLocation(sunShaderProgram, "color");
		glUniform4f(sunColorLocation, 1.0f, 1.0f, 0.0f, 1.0f);

		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2)); // Prvi krug, tj sunce
		glBindVertexArray(0);
		glUseProgram(0);

		glViewport(0, 0, wWidth, wHeight / 2); // Set viewport for the bottom half
		glScissor(0, 0, wWidth, wHeight / 2); // Restrict drawing to the bottom half

		glUseProgram(palmShaderProgram);

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

		float fireSize = sin(glfwGetTime()) * 1.0f + 1.5f;

		unsigned int fireSizeLocation = glGetUniformLocation(fireShaderProgram, "scaleY");
		glUniform1f(fireSizeLocation, fireSize);

		float green = sin(glfwGetTime() * 3.0f) * 0.3f + 0.3f;

		unsigned int fireColorLocation = glGetUniformLocation(fireShaderProgram, "color");
		glUniform4f(fireColorLocation, 1.0f, green, 0.0f, 1.0f);

		unsigned int lightPosLocation = glGetUniformLocation(fireShaderProgram, "lightPos");
		glUniform3f(lightPosLocation, 0.0f, 0.5f, 0.0f);

		unsigned int lightIntensityLocation = glGetUniformLocation(fireShaderProgram, "lightIntensity");
		glUniform1f(lightIntensityLocation, sin(glfwGetTime()) * 0.5f + 1.0f);

		unsigned int normalLocation = glGetUniformLocation(fireShaderProgram, "normal");
		glUniform3f(normalLocation, 0.0f, 1.0f, 0.0f);


		glBindVertexArray(VAO[5]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
		glDrawArrays(GL_TRIANGLES, 0, 3);


		// Use shader program to draw the bottom half circle (water)
		glUseProgram(islandsShaderProgram);

		unsigned int islandOffsetLocation = glGetUniformLocation(islandsShaderProgram, "offset");

		unsigned int islandColorLocation = glGetUniformLocation(islandsShaderProgram, "color");
		glUniform4f(islandColorLocation, 194.0f / 255.0f, 178.0f / 255.0f, 128.0f / 255.0f, 1.0f);

		float waterLevel = abs(sin(glfwGetTime())) * 0.3f;

		unsigned int waterLevelLocation = glGetUniformLocation(islandsShaderProgram, "waterLevel");
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


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(waterShaderProgram);

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


		// Swap buffers to update the screen
		glfwSwapBuffers(window);

		// Handle events (keyboard, mouse, etc.)
		glfwPollEvents();
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++

	//Sve OK - batali program
	glfwTerminate();
	return 0;
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