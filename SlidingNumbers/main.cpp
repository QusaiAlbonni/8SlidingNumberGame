#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_t.h>

#include <SFML/Audio.hpp>

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <random>
#include <ctime>

#define FALSE false
#define TRUE true


void ShowErrorPopup(const char* message);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void refreshCursor(GLFWwindow* window);

void setup_ImGUI(GLFWwindow* window);
void render_ImGui();
void shutdown_ImGui();
void style_ImGui();

void checkConsoleInput(int argc, char* argv[]);

bool hasDuplicates(int argc, char* argv[]);
int countInversions(std::vector<int> arr, int size);

void drawBoard(int board[3][3], Shader shader, unsigned int textures[], bool move = false, int moveInd = -1, int moveJnd = -1, int destX = -1, int destY = -1);
void resetBoard();
void shuffleBoard();
void startAgain();


bool isSolvable(std::vector<int> puzzle);
bool checkWin(int board[3][3]);
bool checkMove();


glm::ivec2 mapValues(double x, double y);
glm::ivec2 getDest(glm::ivec2 start);


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

unsigned int currentWidth = 800;
unsigned int currentHeight = 800;

bool shouldExitProgram = false;
bool error = false;

double deltaTime = 0;
double lastFrame = 0;
double currentFrame = 0;
double clickFrame = 0;

float mouseX = 0;
float mouseY = 0;
glm::ivec2 boardMove;
glm::ivec2 destMove;
float speed = 0.4f;
float baseSpeed = 0.6;
bool move = FALSE;
bool win = FALSE;

sf::Sound whoosh;
sf::Music winMusic;
sf::Music backgroundMusic;

int board[3][3] =
{ {1, 2, 3},
{4, 5, 6},
{7, -1, 8} };

int originalBoard[3][3] =
{ {1, 2, 3},
{4, 5, 6},
{7, -1, 8} };

std::string texturesNames[11] = 
{
    "1.jpg",
    "2.jpg",
    "3.jpg",
    "4.jpg",
    "5.jpg",
    "6.jpg",
    "7.jpg",
    "8.jpg",
    "board.jpg",
    "background.jpg",
    "you_win.jpg",
};

int main(int argc, char* argv[])
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sliding Numbers (Puzzle)", NULL, NULL);
    if (window == NULL)
    {
#ifdef _DEBUG
        std::cout << "Failed to create GLFW window" << std::endl;
#endif
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
#ifdef _DEBUG
        std::cout << "Failed to initialize GLAD" << std::endl;
#endif // _DEBUG
        return -1;
    }


    setup_ImGUI(window);
    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Button] = ImVec4(0.4f, 0.4f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.5f, 0.5f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.3f, 0.3f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.4f, 0.1f, 0.0f);

    style.FrameRounding = 4.0f;

    
    checkConsoleInput(argc, argv);
    
    Shader ourShader("./shaders/vs/vertshader.vs", "./shaders/fs/fragshader.fs");


    float vertices[] = {
         0.6f,  0.6f, 0.0f, 1.0f, 1.0f,
         0.6f, -0.6f, 0.0f, 1.0f, 0.0f,
        -0.6f, -0.6f, 0.0f, 0.0f, 0.0f,
        -0.6f,  0.6f, 0.0f, 0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);



    sf::SoundBuffer sbuffer;
    if (!sbuffer.loadFromFile("./assets/Audio/whoosh.wav"))
    {
#ifdef _DEBUG
        std::cout << "I failed sound" << "\n";
#else
        std::cout << "MISSING FILES REINSTALL GAME" << std::endl;
#endif // _DEBUG
    }
    whoosh = sf::Sound(sbuffer);
    whoosh.setVolume(30.0f);

    winMusic.openFromFile("./assets/Audio/win_trumpets.ogg");
    winMusic.setVolume(20.0f);
    bool winMusicStarted = FALSE;

    backgroundMusic.openFromFile("./assets/Audio/background_music.mp3");
    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(50.0f);



    unsigned int textures[11];
    glGenTextures(11, textures);
    std::string path = "./assets/SlidingPuzzle/";
    for (int i = 0; i < 11; i++)
    {
        
        glBindTexture(GL_TEXTURE_2D, textures[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load((path + texturesNames[i]).c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
#ifdef _DEBUG
            std::cout << "Failed to load texture" << std::endl;
#else
            std::cout << "MISSING FILES REINSTALL GAME" << std::endl;
#endif // _DEBUG
        }
        stbi_image_free(data);
    }



    ourShader.use();
    shouldExitProgram = error;
    backgroundMusic.play();
    while (!glfwWindowShouldClose(window) && !shouldExitProgram)
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        refreshCursor(window);
        processInput(window);
        
        // ImGui
        style_ImGui();
        //
        
        glBindVertexArray(VAO);

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::scale(transform, glm::vec3(1.66f, 1.66f, 1.66f));
        ourShader.setMat4("transform", transform);
        glBindTexture(GL_TEXTURE_2D, textures[9]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        

        transform = glm::mat4(1.0f);
        transform = glm::scale(transform, glm::vec3(1.2f, 1.2f, 1.2f));
        unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
        glBindTexture(GL_TEXTURE_2D, textures[8]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        win = checkWin(board);
        if (!win)
        {
            if (backgroundMusic.getStatus() == sf::SoundSource::Stopped)
            {
                backgroundMusic.play();
            }
            winMusic.stop();
            winMusicStarted = FALSE;
            drawBoard(board, ourShader, textures, move, boardMove.x, boardMove.y, destMove.x, destMove.y);
            if (speed >= 0.4f && move)
            {
                speed = baseSpeed * deltaTime;
                move = FALSE;
                std::swap(board[boardMove.x][boardMove.y], board[destMove.x][destMove.y]);
            }
            else
                speed += baseSpeed * deltaTime;
        }
        else
        {
            transform = glm::mat4(1.0f);
            transform = glm::scale(transform, glm::vec3(1.66666f, 1.66666f, 1.66666f));
            ourShader.setMat4("transform", transform);
            glBindTexture(GL_TEXTURE_2D, textures[10]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            if (winMusic.getStatus() == sf::SoundSource::Stopped && !winMusicStarted) {
                backgroundMusic.stop();
                winMusic.play();
                winMusicStarted = TRUE;
            }
        }

        render_ImGui();
        glfwSwapBuffers(window);
        
    }


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    shutdown_ImGui();
    glfwTerminate();
    return 0;
}



void drawBoard(int board[3][3], Shader shader,unsigned int textures[], bool move, int moveInd, int moveJnd, int destX, int destY) {
    float x = -0.4f;
    float y = 0.4f;


    glm::mat4 transform = glm::mat4(1.0f);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            float speedX = 0;
            float speedY = 0;
            if (board[i][j] != -1) {
                glBindTexture(GL_TEXTURE_2D, textures[board[i][j] - 1]);
                transform = glm::mat4(1.0f);
                bool det = move && i == moveInd && moveJnd == j;
                speedX = det ? speed * -(moveJnd - destY) : 0;
                speedY = det ? speed * (moveInd - destX) : 0;
                transform = glm::translate(transform, glm::vec3(x + speedX, y + speedY, 0.0f));
                transform = glm::scale(transform, glm::vec3(0.33f, 0.33f, 0.33f));
                shader.setMat4("transform", transform);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
            x += 0.4f;
        }
        x = -0.4f;
        y -= 0.4f;
    }
}

void resetBoard()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            board[i][j] = originalBoard[i][j];
        }
    }
}



void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (glfwGetTime() - clickFrame >= 0.3f && !move) {
            clickFrame = glfwGetTime();
            
            if (checkMove() && !win)
            {
                destMove = getDest(boardMove);
                if (destMove.x != -1 && destMove.y != -1)
                {
                    move = TRUE;
                    whoosh.play();
                }
            }
#ifdef _DEBUG
            std::cout << "Cursor Position at (" << mouseX << " : " << mouseY << ")\n";
#endif // DEBUG
        }
    }
}

void refreshCursor(GLFWwindow* window)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec3 win(xpos, ypos, 0);
    glm::vec4 viewport(0, 0, currentWidth, currentHeight);

    glm::vec3 realpos = glm::unProject(win, glm::mat4(1.0f), glm::mat4(1.0f), viewport);

    float worldSpacex = realpos.x;
    float worldSpacey = -realpos.y;
    mouseX = worldSpacex;
    mouseY = worldSpacey;
}

bool checkWin(int board[3][3])
{
    int counter = 1;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (board[i][j] != counter && !(i == 2 && j == 2))
            {
                return false;
            }
            counter++;
        }
    }
    return true;
}

bool checkMove()
{
    glm::ivec2 varifiedInd = mapValues(mouseX, mouseY);
#ifdef _DEBUG
    std::cout << varifiedInd.x << "\t" << varifiedInd.y << "\n";
#endif // _DEBUG
    if (varifiedInd.x == -1 || varifiedInd.y == -1)
    {
        return false;
    }
    else
    {
        boardMove = varifiedInd;
        return true;
    }
}

void startAgain()
{
    resetBoard();
    win = false;
}

void shuffleBoard()
{
 
    std::srand(std::time(0));
    int rows = 3;
    int cols = 3;

 
    std::vector<int> flattenedArray;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            flattenedArray.push_back(board[i][j]);
        }
    }

    do
    {
        std::shuffle(flattenedArray.begin(), flattenedArray.end(), std::default_random_engine(std::rand()));
    } while (std::is_sorted(flattenedArray.begin(), flattenedArray.begin() + 8) || !isSolvable(flattenedArray));
    


    auto it = flattenedArray.begin();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            originalBoard[i][j] = *it;
            ++it;
        }
    }
    resetBoard();
    win = false;

}

bool isSolvable(std::vector<int> puzzle) {

    int inversions = countInversions(puzzle, 9);

    return (inversions % 2 == 0);
}

glm::ivec2 mapValues(double x, double y) {

    glm::ivec2 result;

    if (-0.6 <= x && x < -0.2) {
        result.y = 0;
    }
    else if (-0.2 <= x && x <= 0.2) {
        result.y = 1;
    }
    else if (0.2 < x && x <= 0.6) {
        result.y = 2;
    }
    else {
        result.y = -1;
    }

    if (0.6 >= y && y > 0.2) {
        result.x = 0;
    }
    else if (0.2 >= y && y > -0.2) {
        result.x = 1;
    }
    else if (-0.2 >= y && y > -0.6) {
        result.x = 2;
    }
    else {
        result.x = -1;
    }
    return result;
}

glm::ivec2 getDest(glm::ivec2 start) {
    int row = start.x;
    int col = start.y;
    if (row - 1 >= 0 && board[row - 1][col] == -1) {
        return glm::ivec2(row - 1, col);
    }
    else if (col + 1 < 3 && board[row][col + 1] == -1) {
        return glm::ivec2(row, col + 1);
    }
    else if (row + 1 < 3 && board[row + 1][col] == -1) {
        return glm::ivec2(row + 1, col);
    }
    else if (col - 1 >= 0 && board[row][col - 1] == -1) {
        return glm::ivec2(row, col - 1);
    }
    else
        return glm::vec2(-1, -1);
}


void ShowErrorPopup(const char* message) {
    ImGui::OpenPopup("Error");
    if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", message);
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void setup_ImGUI(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
}

void render_ImGui()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void shutdown_ImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


void style_ImGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImVec2 buttonSize(100, 30);
    ImGui::NewFrame();


    ImGui::SetNextWindowPos(ImVec2(300, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin("window", NULL );


    ImGui::SetNextWindowPos(ImVec2(10, 10));
    if (ImGui::Button("Start Again", buttonSize))
    {
        startAgain();
    }

    ImGui::SameLine();
    ImGui::SetNextWindowPos(ImVec2(10, 120));
    if (ImGui::Button("Shuffle", buttonSize))
    {
        shuffleBoard();
    }

    ImGui::SameLine();
    ImGui::End();
}


void checkConsoleInput(int argc, char* argv[])
{
    bool useArgs = false;
#ifdef _DEBUG
    std::cout << argc << "\n";
#endif // _DEBUG
    error = hasDuplicates(argc, argv);
    for (int i = 0; i < argc; i++)
    {
#ifdef _DEBUG
        std::cout << *argv[i] << "\t";
#endif // _DEBUG

    }
    if (argc == 1)
    {
        std::filesystem::path executablePath = argv[0];
        if (!executablePath.is_absolute() && !(*argv[0] == '.'))
        {
            error = true;

        }
    }
    else
    {
        if (argc == 9)
        {
            for (int i = 1; i < argc; i++)
            {
                if (*argv[i] < 49 || *argv[i] >56)
                {

                    error = true;
                    break;
                }
                std::cout << *argv[i] - 48 << " ";
            }
            if (!error)
            {
                useArgs = true;
            }
        }
        else
        {
            error = true;
        }


        if (useArgs && !error)
        {
            int count = 1;
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    if (!(i == 2 && j == 2))
                    {
                        originalBoard[i][j] = *argv[count] - 48;
                        std::cout << originalBoard[i][j] << "\t";
                        count++;
                    }

                }
            }
            originalBoard[2][2] = -1;
            resetBoard();
        }
    }
}
bool hasDuplicates(int argc, char* argv[])
{
    std::unordered_set<std::string> uniqueArguments;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        if (uniqueArguments.find(arg) != uniqueArguments.end()) {
#ifdef _DEBUG
            std::cout << "Duplicate argument found: " << arg << std::endl;
#endif // DEBUG
            return true;
        }
        else {
 
            uniqueArguments.insert(arg);
        }
    }

    return false;
}
int countInversions(std::vector<int> arr, int size) {
    int inversions = 0;
    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            if (arr[i] > arr[j] && arr[i] != -1 && arr[j] != -1) {
                inversions++;
            }
        }
    }

    return inversions;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    currentWidth = width;
    currentHeight = height;
    glViewport(0, 0, width, height);
}

