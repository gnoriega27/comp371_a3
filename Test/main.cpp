//example skeleton code 2019 winter comp371
//modified from http://learnopengl.com/

#include <GL/glew.h>    // include GL Extension Wrangler
#include <GLFW/glfw3.h>    // include GLFW helper library
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "objloaderIndex.h"  //include the object loader
#include "shaderloader.h"
using namespace std;

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 800;
GLFWwindow *window;

//Camera
glm::vec3 cam_pos = glm::vec3(0, 0, 500);
glm::vec3 cam_dir = glm::vec3(0, 0, -1);
glm::vec3 cam_up = glm::vec3(0, 1, 0);

//MVP Matrices
glm::mat4 modl_matrix = glm::mat4(1.0f);
glm::mat4 view_matrix = glm::lookAt(cam_pos, cam_pos + cam_dir, cam_up);
glm::mat4 proj_matrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.f);


glm::vec3 objectColor = glm::vec3(1, 0.5, 0.31);

//Booleans that are going to be sent to shaders
bool multipleLights = true;
bool shadow = false;




//Model
glm::vec3 transl = glm::vec3(0, 0, 0);

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    std::cout << key << std::endl;
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        cam_pos += cam_dir;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        cam_pos -= cam_dir;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        cam_pos += glm::cross(cam_up, cam_dir);
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        cam_pos -= glm::cross(cam_up, cam_dir);
    }
    if (key == GLFW_KEY_I && action == GLFW_PRESS) {
        transl.y += 1;
    }
    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        transl.y -= 1;
    }
}

int init() {
    std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    //WINDOW
    window = glfwCreateWindow(WIDTH, HEIGHT, "Assignment 1", nullptr, nullptr);

    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW Window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
// The MAIN function, from here we start the application and run the game loop
int main()
{
    if (init() != 0)
        return EXIT_FAILURE;
    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    glEnable(GL_DEPTH_TEST);


    //--------------------------------SHADOW MAPPING SECTION BEGIN--------------------------------
    
    //Framebuffer
    
    //Generating the framebuffer
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    
    //Generating 2D texture to use as framebuffer's depth buffer.
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    //GL_DEPTH__COMPONENT because we only care about the depth values
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    //Binding it
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); //FBO is not complete without a color buffer, so we tell OpenGL that we are not going to render color data.
    glReadBuffer(GL_NONE); //FBO is not complete without a color buffer, so we tell OpenGL that we are not going to render color data.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //--------------------------------SHADOW MAPPING SECTION END----------------------------------
    
    GLuint shader = loadSHADER("/Users/gabrielnoriega/Downloads/Test/Test/shader.vertex", "/Users/gabrielnoriega/Downloads/Test/Test/shader.fragment");
    GLuint depth_shader = loadSHADER("/Users/gabrielnoriega/Downloads/Test/Test/simple_depth.vertex", "/Users/gabrielnoriega/Downloads/Test/Test/simple_depth.fragment");
    glUseProgram(shader);
    
    
    

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<int> indices;
    std::vector<glm::vec2> UVs;
    loadOBJ("/Users/gabrielnoriega/Downloads/Test/Test/heracles.obj", indices, vertices, normals, UVs); //read the vertices from the cube.obj file
    
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    GLuint vertices_VBO;
    glGenBuffers(1, &vertices_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    GLuint normals_VBO;
    glGenBuffers(1, &normals_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices.front(), GL_STATIC_DRAW);

    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

    
    
    //Plane VAO,VBO, and EBO
    std::vector<int> indices_plane;
    std::vector<glm::vec3> vertices_plane;
    std::vector<glm::vec3> normals_plane;
    std::vector<glm::vec2> UVs_plane;
    loadOBJ("/Users/gabrielnoriega/Downloads/Test/Test/plane.obj", indices_plane, vertices_plane, normals_plane, UVs_plane); //read the vertices from the plane.obj file
    
    //Setting up the VAO,VBO and EBO
    GLuint VAO_plane;
    glGenVertexArrays(1, &VAO_plane);
    glBindVertexArray(VAO_plane);
    
    GLuint vertices_VBO_plane;
    glGenBuffers(1, &vertices_VBO_plane);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO_plane);
    glBufferData(GL_ARRAY_BUFFER, vertices_plane.size() * sizeof(glm::vec3), &vertices_plane.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    GLuint normals_VBO_plane;
    glGenBuffers(1, &normals_VBO_plane);
    glBindBuffer(GL_ARRAY_BUFFER, normals_VBO_plane);
    glBufferData(GL_ARRAY_BUFFER, normals_plane.size() * sizeof(glm::vec3), &normals_plane.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    
    GLuint EBO_plane;
    glGenBuffers(1, &EBO_plane);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_plane);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_plane.size() * sizeof(int), &indices_plane.front(), GL_STATIC_DRAW);
    
    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
    
    
    
    //Getting Uniform locations and sending to shader
    GLuint mm_loc = glGetUniformLocation(shader, "mm");
    GLuint vm_loc = glGetUniformLocation(shader, "vm");
    GLuint pm_loc = glGetUniformLocation(shader, "pm");
    
    glUniformMatrix4fv(mm_loc, 1, GL_FALSE, glm::value_ptr(modl_matrix));
    glUniformMatrix4fv(vm_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(pm_loc, 1, GL_FALSE, glm::value_ptr(proj_matrix));
    
    glUniform3fv(glGetUniformLocation(shader, "objectColor"), 1, glm::value_ptr(objectColor));
    glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(cam_pos));
    
    
    //Material
    glUniform3fv(glGetUniformLocation(shader, "material.ambient"), 1, glm::value_ptr(glm::vec3(0.25)));
    glUniform3fv(glGetUniformLocation(shader, "material.diffuse"), 1, glm::value_ptr(glm::vec3(0.75)));
    glUniform3fv(glGetUniformLocation(shader, "material.specular"), 1, glm::value_ptr(glm::vec3(1.0f)));
    
    //Spot Light
    glUniform3fv(glGetUniformLocation(shader, "spotLight.ambient"), 1, glm::value_ptr(glm::vec3(0.1f)));
    glUniform3fv(glGetUniformLocation(shader, "spotLight.diffuse"), 1, glm::value_ptr(glm::vec3(0.8, 0.2, 0.2)));
    glUniform3fv(glGetUniformLocation(shader, "spotLight.specular"), 1, glm::value_ptr(glm::vec3(0.8, 0.2, 0.2)));
    glUniform3fv(glGetUniformLocation(shader, "spotLight.position"), 1, glm::value_ptr(glm::vec3(0.0, 20.0, 10.0)));
    glUniform3fv(glGetUniformLocation(shader, "spotLight.direction"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
    glUniform1f(glGetUniformLocation(shader, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
    glUniform1f(glGetUniformLocation(shader, "spotLight.outerCutOff"), glm::cos(glm::radians(17.5f)));
    
    
    
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        
        
        //--------------------------------FIRST PASS SHADOW MAPPING SECTION BEGIN--------------------------------
        
        //Setting up the light space transform for first pass (Rendering scene from lights POV)
        glm::vec3 lightPos = glm::vec3(0.0f, 20.0f, 10.0f);
        float near_plane = 1.0f, far_plane = 10000.0f;
        //glm::mat4 lightProjection = glm::perspective<float>(glm::radians(45.0f), 1.0f, near_plane, far_plane);
        glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
        //Trasnforming each object so they're visible from the light's POV
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceTransform = lightProjection * lightView;
        
        //Building and compiling our simpleShaders
//        GLuint simpleShader = loadSHADER("simpleVertex.shader", "simpleFragment.shader");
        glUseProgram(depth_shader);
        glUniformMatrix4fv(glGetUniformLocation(depth_shader, "lightSpaceTransform"), 1, GL_FALSE, glm::value_ptr(lightSpaceTransform));
        glUniformMatrix4fv(glGetUniformLocation(depth_shader, "mm"), 1, GL_FALSE, glm::value_ptr(modl_matrix));
        
//        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        //Binding and Drawing Heracles
        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, (int) indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        //Reset viewport
//        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //--------------------------------FIRST PASS SHADOW MAPPING SECTION END-----------------------------------
        
        //--------------------------------SECOND PASS SHADOW MAPPING SECTION BEGIN--------------------------------
        
//        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(shader);
        glm::mat4 proj_matrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.f);
        glm::mat4 view_matrix = glm::lookAt(cam_pos, cam_pos + cam_dir, cam_up);
        glUniformMatrix4fv(mm_loc, 1, GL_FALSE, glm::value_ptr(modl_matrix));
        glUniformMatrix4fv(vm_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glUniformMatrix4fv(pm_loc, 1, GL_FALSE, glm::value_ptr(proj_matrix));
        
        glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(cam_pos));
        glUniform3fv(glGetUniformLocation(shader, "spotLight.position"), 1, glm::value_ptr(lightPos));
        glUniformMatrix4fv(glGetUniformLocation(shader, "lightSpaceTransform"), 1, GL_FALSE, glm::value_ptr(lightSpaceTransform));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        
        //Binding and Drawing Heracles
        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, (int) indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        //Binding and Drawing Plane
        glBindVertexArray(VAO_plane);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_plane);
        glm::mat4 modl_matrix_plane = glm::mat4(1.0f);
        glUniformMatrix4fv(mm_loc, 1, GL_FALSE, glm::value_ptr(modl_matrix_plane));
        glDrawElements(GL_TRIANGLES, (int) indices_plane.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        //--------------------------------SECOND PASS SHADOW MAPPING SECTION END--------------------------------
        
        //Updating the information about the MVP matrices, booleans, light/object/material colors.
        glUniformMatrix4fv(mm_loc, 1, GL_FALSE, glm::value_ptr(modl_matrix));
        glUniformMatrix4fv(vm_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glUniformMatrix4fv(pm_loc, 1, GL_FALSE, glm::value_ptr(proj_matrix));
        
//        //Booleans
//        glUniform1f(glGetUniformLocation(shader, "multipleLights"), multipleLights);
//        glUniform1f(glGetUniformLocation(shader, "shadow"), shadow);
//
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }


    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

//
//// Window dimensions
//const GLuint WIDTH = 800, HEIGHT = 800;
//GLFWwindow *window;
//
//// Camera
//double xpos_old = 0;
//double ypos_old = 0;
//glm::mat4 view_matrix;
//
//glm::vec3 camera_position = glm::vec3(-54, 4, 14);
//glm::vec3 camera_direction = glm::vec3(1, 0, 0);
//glm::vec3 camera_up = glm::vec3(0, 1, 0);
//bool mouseLeftClickActive = false;
//
//
//
///*
// Transformations
// */
//glm::vec3 xAxis = glm::vec3(1, 0, 0);
//glm::vec3 yAxis = glm::vec3(0, 1, 0);
//glm::vec3 zAxis = glm::vec3(0, 0, 1);
//
//int rotationAngle = 40;
//
//int targetAngle = 0;
//
//glm::vec3 meshRotation = xAxis;
//
//glm::vec3 scale = glm::vec3(1, 1, 1);
//
//glm::mat4 modl_matrix = glm::mat4(1.0f);
//
//glm::vec3 translation = glm::vec3(0, 0, 0);
//
////
//
//
//int redChannel = 1, greenChannel = 1, blueChannel = 1;
//int lightColor = 1;
//bool areChannelsOn = true;
//bool useNormalsFlag = false;
//bool grayScaleOn = false;
//bool lightsOn = true;
//bool isPhongEnabled = true;
//
//void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
//{
//    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
//        mouseLeftClickActive = true;
//
//    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
//        mouseLeftClickActive = false;
//}
//
//static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
//{
//    if (mouseLeftClickActive) {
//        if (ypos > ypos_old) {
//            if(mouseLeftClickActive)
//                camera_position += camera_direction;
//        } else {
//            if (mouseLeftClickActive)
//                camera_position -= camera_direction;
//        }
//        ypos_old = ypos;
//    }
//}
//
//// Is called whenever a key is pressed/released via GLFW
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
//{
//    std::cout << key << std::endl;
//    if (key == GLFW_KEY_ESCAPE)
//        glfwSetWindowShouldClose(window, GL_TRUE);
//
//    // 1. Move forward, backwards, left and right
//
//    if (key == GLFW_KEY_W ) {
//        camera_position += camera_direction;
//    }
//    if (key == GLFW_KEY_S) {
//        camera_position -= camera_direction;
//    }
//    if (key == GLFW_KEY_A) {
//        camera_position += glm::cross(camera_up, camera_direction);
//    }
//    if (key == GLFW_KEY_D ) {
//        camera_position -= glm::cross(camera_up, camera_direction);
//    }
//
//    // 2. Rotate camera
//
//    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
//        // Rotate x-axis (+)
//        camera_direction.x += 0.1f;
//    }
//
//    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
//        // Rotate x-axis (-)
//        camera_direction.x += -0.1f;
//    }
//
//    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
//        // Rotate y-axis (+)
//        camera_direction.y += 0.1f;
//    }
//
//    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {\
//        // Rotate y-axis (-)
//        camera_direction.y += -0.1f;
//    }
//
//    // Rotation of object
//
//    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
//        targetAngle = rotationAngle;
//        meshRotation = xAxis;
//    }
//    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
//        targetAngle = rotationAngle;
//        meshRotation = yAxis;
//    }
//    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
//        targetAngle = rotationAngle;
//        meshRotation = zAxis;
//    }
//
//    // Translation of object
//
//    if (key == GLFW_KEY_J && action == GLFW_PRESS) {
//        translation.x += 1;
//    }
//    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
//        translation.x -= 1;
//    }
//    if (key == GLFW_KEY_I && action == GLFW_PRESS) {
//        translation.y += 1;
//    }
//    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
//        translation.y -= 1;
//    }
//    if (key == GLFW_KEY_PAGE_UP && action == GLFW_PRESS) {
//        translation.z += 1;
//    }
//    if (key == GLFW_KEY_PAGE_DOWN && action == GLFW_PRESS) {
//        translation.z -= 1;
//    }
//
//
//    // Scaling of object
//
//    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
//        scale += 0.1f * scale;
//    }
//    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
//        scale -= 0.1f * scale;
//    }
//
//    // // A2
//
//
//    //
//
//    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
//        // Toggle the Red Channel on/off
//        redChannel = (redChannel) ? 0 : 1;
//    }
//    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
//        // Toggle the Green Channel on/off
//        greenChannel = (greenChannel) ? 0 : 1;
//    }
//    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
//        // Toggle the Blue Channel on/off
//        blueChannel = (blueChannel) ? 0 : 1;
//    }
//
//    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
//        // All channels on
//        redChannel = 1;
//        greenChannel = 1;
//        blueChannel = 1;
//    }
//
//    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
//        // Change model
//        isPhongEnabled = isPhongEnabled ? false : true;
//    }
//
//
//    if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
//        // Toggle the light on/off
//        lightsOn = lightsOn ? false : true;
//    }
//
//
//    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
//        // Toggle the normals as color on/off
//        useNormalsFlag = useNormalsFlag ? false : true;
//    }
//
//    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
//        // Toggle the gray scale color on/off
//        grayScaleOn = grayScaleOn ? false : true;
//    }
//
//}
//
//int init() {
//    std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
//    glfwInit();
//
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
//
//    //WINDOW
//    window = glfwCreateWindow(WIDTH, HEIGHT, "Assignment 1", nullptr, nullptr);
//
//    if (nullptr == window)
//    {
//        std::cout << "Failed to create GLFW Window" << std::endl;
//        glfwTerminate();
//        return EXIT_FAILURE;
//    }
//
//    glfwMakeContextCurrent(window);
//
//    glEnable(GL_DEPTH_TEST);
//    glewExperimental = GL_TRUE;
//
//    if (GLEW_OK != glewInit())
//    {
//        std::cout << "Failed to initialize GLEW" << std::endl;
//        return EXIT_FAILURE;
//    }
//    return 0;
//}
//// The MAIN function, from here we start the application and run the game loop
//int main()
//{
//    if (init() != 0)
//        return EXIT_FAILURE;
//    // Set the required callback functions
//    glfwSetKeyCallback(window, key_callback);
//    glfwSetMouseButtonCallback(window, mouse_button_callback);
//    glfwSetCursorPosCallback(window, cursor_position_callback);
//    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
//    glewExperimental = GL_TRUE;
//    // Initialize GLEW to setup the OpenGL Function pointers
//    if (glewInit() != GLEW_OK)
//    {
//        std::cout << "Failed to initialize GLEW" << std::endl;
//        return -1;
//    }
//
//    // Build and compile our shader program
//    // Vertex shader
//
//    GLuint shader = loadSHADER("vertex.shader", "fragment.shader");
//    glUseProgram(shader);
//
//
//    std::vector<int> indices;
//    std::vector<glm::vec3> vertices;
//    std::vector<glm::vec3> normals;
//    std::vector<glm::vec2> UVs;
//    //loadOBJ("cube.obj", indices, vertices, normals, UVs); //read the vertices from the cube.obj file
//    loadOBJ("heracles.obj", indices, vertices, normals, UVs); //read the vertices from the cube.obj file
//
//
//
//    GLuint VAO;
//    glGenVertexArrays(1, &VAO);
//    glBindVertexArray(VAO);
//
//    GLuint vertices_VBO;
//    glGenBuffers(1, &vertices_VBO);
//    glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
//    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
//    glEnableVertexAttribArray(0);
//
//    GLuint normals_VBO;
//    glGenBuffers(1, &normals_VBO);
//    glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
//    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
//    glEnableVertexAttribArray(1);
//
//    GLuint EBO;
//    glGenBuffers(1, &EBO);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices.front(), GL_STATIC_DRAW);
//
//
//    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
//
//
//
//    glm::mat4 proj_matrix = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 700.0f);
//
//    GLuint mm_location = glGetUniformLocation(shader, "mm");
//    GLuint vm_location = glGetUniformLocation(shader, "vm");
//    GLuint pm_location = glGetUniformLocation(shader, "pm");
//
//    GLuint phong_enabled_location = glGetUniformLocation(shader, "phong_enabled");
//    GLuint use_normals_location = glGetUniformLocation(shader, "useNormals");
//
//    glUniformMatrix4fv(pm_location, 1, 0, glm::value_ptr(proj_matrix));
//
//    GLuint object_color_location = glGetUniformLocation(shader, "object_color");
//    GLuint light_color_location = glGetUniformLocation(shader, "light_color");
//    GLuint grayScaleOn_location = glGetUniformLocation(shader, "gray_scale_on");
//    GLuint lights_on_location = glGetUniformLocation(shader, "lights_on");
//
//
//    glm::vec3 modl_vector_object_color = glm::vec3(redChannel, greenChannel, blueChannel);
//    glm::vec3 modl_vector_light_color = glm::vec3(lightColor, lightColor, lightColor);
//
//
//    glUniform1i(phong_enabled_location, isPhongEnabled);
//    glUniform1i(use_normals_location, useNormalsFlag);
//    glUniform1i(grayScaleOn_location, grayScaleOn);
//    glUniform1i(lights_on_location, lightsOn);
//
//    glUniform3fv(glGetUniformLocation(shader, "light_position"), 1, glm::value_ptr(glm::vec3(0.0f, 20.0f, 5.0f)));
//    glUniform3fv(glGetUniformLocation(shader, "view_position"), 1, glm::value_ptr(camera_position));
//
//
//
//    // Game loop
//    int i = 0;
//    while (!glfwWindowShouldClose(window))
//    {
//        i++;
//
//        view_matrix = glm::lookAt(camera_position, camera_position + camera_direction, camera_up);
//        glUniformMatrix4fv(vm_location, 1, 0, glm::value_ptr(view_matrix));
//
//        glm::mat4 rotator = glm::rotate(glm::mat4(1.0f), targetAngle / 180.f, meshRotation);
//        glm::mat4 translator = glm::translate(glm::mat4(1.0f), translation);
//        glm::mat4 scalor = glm::scale(glm::mat4(1.0f), scale);
//        modl_matrix = scalor * translator * rotator * modl_matrix;
//        // Reset transformation values
//        glUniformMatrix4fv(mm_location, 1, GL_FALSE, glm::value_ptr(modl_matrix));
//        targetAngle = 0;
//        translation = glm::vec3(0, 0, 0);
//        scale = glm::vec3(1, 1, 1);
//        //
//
//        modl_vector_object_color = glm::vec3(redChannel, greenChannel, blueChannel);
//        modl_vector_light_color = glm::vec3(lightColor, lightColor, lightColor);
//
//        glUniform1i(phong_enabled_location, isPhongEnabled);
//        glUniform1i(use_normals_location, useNormalsFlag);
//        glUniform1i(grayScaleOn_location, grayScaleOn);
//        glUniform3fv(object_color_location, 1, glm::value_ptr(modl_vector_object_color));
//        glUniform3fv(light_color_location, 1, glm::value_ptr(modl_vector_light_color));
//        glUniform1i(lights_on_location, lightsOn);
//
//        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
//        glfwPollEvents();
//
//        // Render
//        // Clear the colorbuffer
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        glBindVertexArray(VAO);
//        glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, 10);
//        //glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
//        //glDrawArrays(GL_TRIANGLES, 0, vertices.size());
//
//        glBindVertexArray(0);
//
//
//        // Swap the screen buffers
//        glfwSwapBuffers(window);
//    }
//
//    // Terminate GLFW, clearing any resources allocated by GLFW.
//    glfwTerminate();
//    return 0;
//}
