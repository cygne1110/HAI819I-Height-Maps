#include <iostream>
#include <cstdlib>
#include <ctime>

#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace glm;

#include <shader.hpp>
#include <texture.hpp>

#define FRAME_COOLDOWN 20;

// settings
u32 SCR_WIDTH = 800;
u32 SCR_HEIGHT = 600;

f32 lastX = 400, lastY = 300;
f32 fov = 70.0;

f32 deltaTime = 0.0f; // Time between current frame and last frame
f32 lastFrame = 0.0f; // Time of last frame

bool firstMouse = true;
f32 yaw = -90.0;
f32 pitch = 0.0;

vec3 camera_position = vec3(5.0f, 5.0f, 5.0f);
vec3 camera_target = vec3(0.0f, 1.0f, 0.0f);
vec3 camera_up = vec3(0.0f, 1.0f, 0.0f);
vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);

i32 RESOLUTION = 256;
i32 CURR_COOLDOWN = 0;
bool RES_UPDATED = true;

enum CameraMode {

    ORBIT,
    FREE

};

i32 CURR_MODE = ORBIT;

f32 rotate_speed = 0.0;
mat4 rotate_camera = mat4(1.0f);

void framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height);
void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos);
void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset);
void processInput(GLFWwindow *window);
void createSurface(i32 resolution, std::vector<u32> &indices, std::vector<vec3> &indexed_vertices, std::vector<vec2> &uvs);

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // initializing window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Height Maps", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glewInit();

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    srand(time(NULL));

    // get screen size
    i32 w, h;
    glfwGetFramebufferSize(window, &w, &h);
    SCR_WIDTH = w;
    SCR_HEIGHT = h;

    // update window size
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    mat4 Projection;
    mat4 View;
    mat4 Model;
    mat4 MVP;

    // SHADERS
    ShaderProgram shaderProgram("shaders/vertex_shader.vert", "shaders/fragment_shader.frag");
    shaderProgram.link();
    shaderProgram.use();

    Texture grass("data/textures/grass.png");
    Texture rock("data/textures/rock.png");
    Texture snowrocks("data/textures/snowrocks.png");

    Texture heightMap("data/height_maps/hmap_mountain.png");

    glUniform1i(glGetUniformLocation(shaderProgram.getID(), "textureGrass"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram.getID(), "textureRock"), 1);
    glUniform1i(glGetUniformLocation(shaderProgram.getID(), "textureSnow"), 2);
    glUniform1i(glGetUniformLocation(shaderProgram.getID(), "heightMap"), 3);

    grass.generate();
    rock.generate();
    snowrocks.generate();

    heightMap.generate(true);

    Model = mat4(1.0f);
    Model = translate(Model, vec3(0.0f, 0.0f, 0.0f));
    Model = scale(Model, vec3(4.0f));

    std::vector<u32> indices;
    std::vector<vec3> indexed_vertices;
    std::vector<vec2> uvs;

    GLuint vertexattributes, vertexbuffer, uvbuffer, elementbuffer;
    glGenVertexArrays(1, &vertexattributes);
    glGenBuffers(1, &vertexbuffer);
    glGenBuffers(1, &uvbuffer);
    glGenBuffers(1, &elementbuffer);

    GLuint MatrixID = glGetUniformLocation(shaderProgram.getID(), "mvp");

    // render loop
    while(!glfwWindowShouldClose(window)) {

        f32 currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        if(CURR_MODE == FREE) {
            View = lookAt(camera_position, camera_position + camera_front, camera_up);
            Projection = perspective(radians(fov), (f32)SCR_WIDTH / (f32)SCR_HEIGHT, 0.0001f, 100.0f);
        } else {
            vec4 tmp = rotate_camera * vec4(camera_position.x, camera_position.y, camera_position.z, 1.0);
            camera_position = vec3(tmp.x, tmp.y, tmp.z);
            View = lookAt(camera_position, camera_target, camera_up);
            Projection = perspective(radians(45.0f), (f32)SCR_WIDTH / (f32)SCR_HEIGHT, 0.0001f, 100.0f);
        }
    
        MVP = Projection * View * Model;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        if(RES_UPDATED) {

            RES_UPDATED = false;

            indices.clear();
            indexed_vertices.clear();
            uvs.clear();

            createSurface(RESOLUTION, indices, indexed_vertices, uvs);

            glBindVertexArray(vertexattributes);

            // VERTICES
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(vec3), &indexed_vertices[0], GL_STATIC_DRAW);

            // 1rst attribute buffer : vertices
            glVertexAttribPointer(
                0,        // attribute
                3,        // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0,        // stride
                (void*)0  // array buffer offset
            );
            glEnableVertexAttribArray(0);

            // UVs
            glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
            glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);

            // 2nd attribute buffer : UVs
            glVertexAttribPointer(
                1,        // attribute
                2,        // size : U+V => 2
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0,        // stride
                (void*)0 // array buffer offset
            );
            glEnableVertexAttribArray(1);

            // ELEMENT BUFFER OBJECT
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), &indices[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

        }

        // make the background purple
        glClearColor(48.f/255.f, 31.f/255.f, 67.f/255.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        grass.bind(0);
        rock.bind(1);
        snowrocks.bind(2);
        heightMap.bind(3);

        shaderProgram.use();

        // Index buffer
        glBindVertexArray(vertexattributes);

        // Draw the triangles !
        glDrawElements(
            GL_TRIANGLES,      // mode
            indices.size(),    // count
            GL_UNSIGNED_INT,   // type
            (void*)0           // element array buffer offset
        );

        if(CURR_COOLDOWN > 0) CURR_COOLDOWN--;
        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    shaderProgram.stop();
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // goodbye
    glfwTerminate();
    return 0;

}

void framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {

    // ESCAPE closes the window
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if(CURR_MODE == FREE) {
        const f32 camera_speed = 4.0f * deltaTime; // adjust accordingly
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera_position += camera_speed * camera_front;
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera_position -= camera_speed * camera_front;
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera_position -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera_position += glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera_position += camera_up * camera_speed;
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera_position -= camera_up * camera_speed;
    }

    if(CURR_COOLDOWN == 0) {

        if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            switch(CURR_MODE) {
                case ORBIT:
                    CURR_MODE = FREE;
                    std::cout << "Camera is now in free mode\n";
                    camera_position = vec3(0.0, 3.0, 5.0);
                    camera_up = vec3(0.0, 1.0, 0.0);
                    camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
                    rotate_camera = rotate(mat4(1.0f), radians(0.f), vec3(0.0, 1.0, 0.0));
                    break;
                case FREE:
                    CURR_MODE = ORBIT;
                    std::cout << "Camera is now in orbit mode\n";
                    camera_position = vec3(5.0f, 5.0f, 5.0f);
                    camera_target = vec3(0.0, 1.0, 0.0);                   
                    camera_up = vec3(0.0f, 1.0f, 0.0f);
                    rotate_speed = 0.0;
                    break;
            }
            CURR_COOLDOWN = FRAME_COOLDOWN;
        }

        if(glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS && RESOLUTION < 512) {
            RESOLUTION *= 2;
            std::cout << "Terrain resolution increased to " << RESOLUTION << "\n";
            CURR_COOLDOWN = FRAME_COOLDOWN;
            RES_UPDATED = true;
        }

        if(glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS && RESOLUTION > 4) {
            RESOLUTION /= 2;
            std::cout << "Terrain resolution decreased to " << RESOLUTION << "\n";
            CURR_COOLDOWN = FRAME_COOLDOWN;
            RES_UPDATED = true;
        }

        if(CURR_MODE == ORBIT) {

            if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && rotate_speed < 10.0) {
                rotate_speed++;
                std::cout << "Orbit speed increased to " << rotate_speed << "\n";
                CURR_COOLDOWN = FRAME_COOLDOWN;
                rotate_camera = rotate(mat4(1.0f), radians(rotate_speed), vec3(0.0, 1.0, 0.0));
            }

            if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && rotate_speed > 0.0) {
                rotate_speed--;
                std::cout << "Orbit speed decreased to " << rotate_speed << "\n";
                CURR_COOLDOWN = FRAME_COOLDOWN;
                rotate_camera = rotate(mat4(1.0f), radians(rotate_speed), vec3(0.0, 1.0, 0.0));
            }

        }

    }

}

void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset) {

    if(CURR_MODE == FREE) {
        fov -= (f32)yoffset;

        if (fov < 1.0f)
            fov = 1.0f;
        if (fov > 90.0f)
            fov = 90.0f;
    }

}

void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
    
    if(CURR_MODE == FREE) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        f32 xoffset = xpos - lastX;
        f32 yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        f32 sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;

        pitch += yoffset;
        if(pitch > 89.0f)
        pitch = 89.0f;
        if(pitch < -89.0f)
        pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        camera_front = glm::normalize(direction);
    }

}

void createSurface(i32 resolution, std::vector<u32> &indices, std::vector<vec3> &indexed_vertices, std::vector<vec2> &uvs) {
    // Create a grid of vertices
    for (i32 i = 0; i < resolution; i++) {
        for (i32 j = 0; j < resolution; j++) {
            // f64 z = (rand() % 256)/(f64)(255*2);
            f64 y = 0;
            // f64 z = ((f32)i / (f32)(resolution-1));
            indexed_vertices.push_back(vec3((f32)i / (f32)(resolution-1) - 0.5f, y, (f32)j / (f32)(resolution-1) - 0.5f));
            uvs.push_back(vec2(
                (f32)i / ((f32)resolution - 1.0f),
                (f32)j / ((f32)resolution - 1.0f)));
        }
    }

    // Create the triangles
    for (i32 i = 0; i < resolution - 1; i++) {
        for (i32 j = 0; j < resolution - 1; j++) {
            indices.push_back(i * resolution + j);
            indices.push_back((i + 1) * resolution + j);
            indices.push_back(i * resolution + j + 1);
            indices.push_back(i * resolution + j + 1);
            indices.push_back((i + 1) * resolution + j);
            indices.push_back((i + 1) * resolution + j + 1);
        }
    }
}