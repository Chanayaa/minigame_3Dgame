#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>
#include <learnopengl/filesystem.h>

#include <vector>
#include <iostream>
#include <ctime>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

float deltaTime = 0;
float lastFrame = 0;

glm::vec3 playerPos(0, 0, 0);
float playerSpeed = 5.0f;

float yaw = -90;
float pitch = -20;

float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;

bool firstMouse = true;
bool ePressed = false;

float cameraDistance = 3;

int totalAbnormal = 3;
int foundCount = 0;

int wrongGuess = 0;
int maxWrong = 5;

bool gameWin = false;
bool gameLose = false;
bool resultPrint = false;

enum FurnitureType
{
    CHAIR,
    TABLE,
    LAMP
};

struct Furniture
{
    Model* model;

    glm::vec3 pos;

    float scale;

    float abnormalScale = 1.0f;

    glm::vec3 abnormalRotationAxis = glm::vec3(0, 1, 0);
    float abnormalRotationAngle = 0;

    FurnitureType type;

    bool abnormal = false;
    bool fixed = false;
};

std::vector<Furniture> furnitures;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sens = 0.15f;

    yaw += xoffset * sens;
    pitch += yoffset * sens;

    if (pitch > 45) pitch = 45;
    if (pitch < -30) pitch = -30;
}

void processInput(GLFWwindow* window)
{
    float speed = playerSpeed * deltaTime;

    glm::vec3 forward;

    forward.x = cos(glm::radians(yaw));
    forward.z = sin(glm::radians(yaw));
    forward.y = 0;

    forward = glm::normalize(forward);

    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) playerPos += forward * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) playerPos -= forward * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) playerPos -= right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) playerPos += right * speed;
}

void spawnFurniture(Model& chair, Model& table, Model& lamp)
{
    furnitures.push_back({ &chair,{2,0,2},0.01f,1,{0,1,0},0,CHAIR });
    furnitures.push_back({ &chair,{-2,0,1},0.01f,1,{0,1,0},0,CHAIR });

    furnitures.push_back({ &table,{1,1,-4},0.005f,1,{0,1,0},0,TABLE });
    furnitures.push_back({ &table,{-3,1,-1},0.005f,1,{0,1,0},0,TABLE });

    furnitures.push_back({ &lamp,{3,0,1},0.05f,1,{0,1,0},0,LAMP });
    furnitures.push_back({ &lamp,{-1,0,3},0.05f,1,{0,1,0},0,LAMP });

    furnitures.push_back({ &chair,{0,0,-3},0.01f,1,{0,1,0},0,CHAIR });
    furnitures.push_back({ &table,{2,1,-1},0.005f,1,{0,1,0},0,TABLE });

    furnitures.push_back({ &lamp,{-3,0,2},0.05f,1,{0,1,0},0,LAMP });
    furnitures.push_back({ &chair,{3,0,-1},0.01f,1,{0,1,0},0,CHAIR });
}

void generateAbnormal()
{
    srand(time(0));

    int count = 0;

    while (count < totalAbnormal)
    {
        int i = rand() % furnitures.size();

        if (!furnitures[i].abnormal)
        {
            furnitures[i].abnormal = true;

            int type = rand() % 2;

            // ---------- SCALE ----------
            if (type == 0)
            {
                float scale = 1.0f + (rand() % 100) / 100.0f;
                // 1.0 ? 2.0

                furnitures[i].abnormalScale = scale;
            }

            // ---------- ROTATION ----------
            if (type == 1)
            {
                float angle = 30.0f + rand() % 150;
                // 30° ? 180°

                glm::vec3 axis;

                if (furnitures[i].type == LAMP)
                {
                    // Lamp cannot rotate on Y
                    axis.x = rand() % 2;
                    axis.y = 0;
                    axis.z = rand() % 2;

                    if (axis.x == 0 && axis.z == 0)
                        axis.x = 1;
                }
                else
                {
                    axis.x = rand() % 2;
                    axis.y = rand() % 2;
                    axis.z = rand() % 2;

                    if (axis == glm::vec3(0, 0, 0))
                        axis.y = 1;
                }

                axis = glm::normalize(axis);

                furnitures[i].abnormalRotationAxis = axis;
                furnitures[i].abnormalRotationAngle = angle;
            }

            count++;
        }
    }
}

void checkInteraction()
{
    for (auto& f : furnitures)
    {
        float dist = glm::length(playerPos - f.pos);

        if (dist < 2.0f)
        {
            if (f.abnormal && !f.fixed)
            {
                f.fixed = true;
                foundCount++;

                std::cout << "Found " << foundCount << "/" << totalAbnormal << "\n";

                if (foundCount >= totalAbnormal)
                    gameWin = true;
            }
            else
            {
                wrongGuess++;

                std::cout << "Wrong " << wrongGuess << "/" << maxWrong << "\n";

                if (wrongGuess >= maxWrong)
                    gameLose = true;
            }

            return;
        }
    }
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Find Abnormal", NULL, NULL);

    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    Shader shader("1.model_loading.vs", "1.model_loading.fs");

    Model room(FileSystem::getPath("resources/objects/room3/basement.obj"));
    Model player(FileSystem::getPath("resources/objects/player/Style human obj.obj"));
    Model chair(FileSystem::getPath("resources/objects/chair/old_chair.obj"));
    Model table(FileSystem::getPath("resources/objects/table/Desk OBJ.obj"));
    Model lamp(FileSystem::getPath("resources/objects/lamp2/lamp.obj"));

    spawnFurniture(chair, table, lamp);
    generateAbnormal();

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (!gameWin && !gameLose)
            processInput(window);

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed)
        {
            if (!gameWin && !gameLose)
                checkInteraction();

            ePressed = true;
        }

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
            ePressed = false;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.5, 0.5, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::vec3 front;

        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        front = glm::normalize(front);

        glm::vec3 camPos = playerPos - front * cameraDistance + glm::vec3(0, 2, 0);

        glm::mat4 view = glm::lookAt(camPos, playerPos + glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

        glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        shader.setMat4("view", view);
        shader.setMat4("projection", proj);

        glm::mat4 model = glm::mat4(1);
        shader.setMat4("model", model);
        room.Draw(shader);

        model = glm::mat4(1);
        model = glm::translate(model, playerPos);
        model = glm::rotate(model, glm::radians(-yaw + 90), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.1f));

        shader.setMat4("model", model);
        player.Draw(shader);

        for (auto& f : furnitures)
        {
            glm::mat4 m = glm::mat4(1);

            m = glm::translate(m, f.pos);

            float s = f.scale;

            if (f.abnormal && !f.fixed)
                s *= f.abnormalScale;

            m = glm::scale(m, glm::vec3(s));

            if (f.abnormal && !f.fixed)
            {
                m = glm::rotate(
                    m,
                    glm::radians(f.abnormalRotationAngle),
                    f.abnormalRotationAxis
                );
            }

            shader.setMat4("model", m);

            f.model->Draw(shader);
        }

        if (!resultPrint) {
            if (gameWin) {
                std::cout << "YOU WIN\n";
                resultPrint = true;
            }
            if (gameLose) {
                std::cout << "YOU LOSE\n";
                resultPrint = true;
            }
        }
            

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
