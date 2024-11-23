// Autor: Nedeljko Tesanovic
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

unsigned int compileShader(GLenum type, const char* source); //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
unsigned int createShader(const char* vsSource, const char* fsSource); //Pravi objedinjeni sejder program koji se sastoji od Verteks sejdera ciji je kod na putanji vsSource i Fragment sejdera na putanji fsSource
void generateCircle(float* circle, int offset, float r, float centerX, float centerY);

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

  /*  GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);*/

    //window = glfwCreateWindow(mode->width, mode->height, wTitle, primaryMonitor, NULL); // Napravi novi prozor
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL); // Napravi novi prozor
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

    unsigned int shaderProgram = createShader("island.vert", "island.frag"); // Pretpostavljamo da ovo pravi shader program sa uniformom `offset`

    unsigned int VAO[1]; 
    glGenVertexArrays(1, VAO);
    unsigned int VBO[1];
    glGenBuffers(1, VBO);


    float circle[(CRES + 2) * 2 * 4]; // Za tri kruga (3 puta broj tačaka)

    float r1 = 0.4f; // Poluprečnik prvog kruga
    float r2 = 0.2f; // Poluprečnik drugog kruga
    float r3 = 0.2f; // Poluprečnik trećeg kruga
    float r4 = 0.1f; //poluprecnik sunca

    generateCircle(circle, 0, r4, 0.0f, 0.0f);  //sunce

    int offset = (CRES + 2) * 2;
    generateCircle(circle, offset, r1, 0.0f, 0.0f);

    offset += (CRES + 2) * 2;
    generateCircle(circle, offset, r2, 0.7f, 0.0f);

    offset += (CRES + 2) * 2;
    generateCircle(circle, offset, r3, -0.7f, 0.0f);


    // Bind VAO, VBO, and send data to GPU
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++

    glClearColor(0, 0, 1, 1); //Podesavanje boje pozadine: RGBA (R - Crvena, G - Zelena, B - Plava, A = neprovidno; Opseg od 0 do 1, gdje je 0 crno a 1 svijetlo)

    while (!glfwWindowShouldClose(window)) // Infinite loop
    {
        // User input (Escape to close the window)
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        // Getting window dimensions
       /* int wWidth = mode->width;
        int wHeight = mode->height;*/

        glEnable(GL_SCISSOR_TEST);


        // --- Top half of the screen (Sky blue) ---
        glViewport(0, wHeight / 2, wWidth, wHeight / 2); // Set viewport for the top half
        glScissor(0, wHeight / 2, wWidth, wHeight / 2); // Restrict drawing to the top half
        glClearColor(0.529, 0.808, 0.922, 1); // Sky blue color
        glClear(GL_COLOR_BUFFER_BIT); // Clear the top half of the screen

        glUseProgram(shaderProgram);

        GLint offsetLocation = glGetUniformLocation(shaderProgram, "offset");
        glUniform2f(offsetLocation, -0.8f, 0.8f);

        GLint colorLocation = glGetUniformLocation(shaderProgram, "color");
        glUniform3f(colorLocation, 1.0f, 1.0f, 0.0f); 

        // Bind and draw the bottom circle
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, (CRES + 2)); // Prvi krug, tj sunce
        glBindVertexArray(0);
        glUseProgram(0);

        glViewport(0, 0, wWidth, wHeight / 2); // Set viewport for the bottom half
        glScissor(0, 0, wWidth, wHeight / 2); // Restrict drawing to the bottom half
        glClearColor(0, 0, 0.5, 1); // Dark blue color for the water
        glClear(GL_COLOR_BUFFER_BIT); // Clear the top half of the screen


        // Use shader program to draw the bottom half circle (water)
        glUseProgram(shaderProgram);

        glUniform2f(offsetLocation, 0.0f, 0.0f);
        glUniform3f(colorLocation, 194.0f / 255.0f, 178.0f / 255.0f, 128.0f / 255.0f);

        // Bind and draw the bottom circle
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLE_FAN, (CRES + 2), (CRES + 2)); // Prvi krug
        glDrawArrays(GL_TRIANGLE_FAN, (CRES + 2) * 2, (CRES + 2)); // Drugi krug
        glDrawArrays(GL_TRIANGLE_FAN, (CRES + 2) * 3, (CRES + 2)); // Treci krug
        glBindVertexArray(0);
        glUseProgram(0);

        glDisable(GL_SCISSOR_TEST);


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