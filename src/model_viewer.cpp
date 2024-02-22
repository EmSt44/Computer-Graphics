// Model viewer code for the assignments in Computer Graphics 1TD388/1MD150.
//
// Modify this and other source files according to the tasks in the instructions.
//

#include "gltf_io.h"
#include "gltf_scene.h"
#include "gltf_render.h"
#include "cg_utils.h"
#include "cg_trackball.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <iostream>

// Struct for our application context
struct Context {
    int width = 512;
    int height = 512;
    GLFWwindow *window;
    gltf::GLTFAsset asset;
    gltf::DrawableList drawables;
    cg::Trackball trackball;
    GLuint program;
    GLuint emptyVAO;
    float elapsedTime;
    std::string gltfFilename = "gargo.gltf";
    glm::vec3 background_color;
    glm::vec3 diffuseColor;
    glm::vec3 lightPosition;
    glm::vec3 specularColor;
    glm::vec3 ambientColor;
    float specularPower;
    bool ortho;
    bool shaderToggle = true; //initialize as true
    bool gammaToggle = true; //initialize as true
    uint cubemap[9];
    bool cubemapToggle = true; //initialize as true
    int textureIndex = 0; //initialize as 0, can be 0 to 8
    // Add more variables here...
};

// Global variable for zooming the fov of the perspective matrix
float zoomFactor = 1.0f;

// Returns the absolute path to the src/shader directory
std::string shader_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/src/shaders/";
}

// Returns absolute path to assets/cubemaps directory
std::string cubemap_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/assets/cubemaps/";
}

// Returns the absolute path to the assets/gltf directory
std::string gltf_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/assets/gltf/";
}

void do_initialization(Context &ctx)
{
    ctx.program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");

    ctx.cubemap[0] = cg::load_cubemap(cubemap_dir() + "/RomeChurch/"); //load in cubemap
    ctx.cubemap[1] = cg::load_cubemap(cubemap_dir() + "/RomeChurch/prefiltered/2048");
    ctx.cubemap[2] = cg::load_cubemap(cubemap_dir() + "/RomeChurch/prefiltered/512");
    ctx.cubemap[3] = cg::load_cubemap(cubemap_dir() + "/RomeChurch/prefiltered/128");
    ctx.cubemap[4] = cg::load_cubemap(cubemap_dir() + "/RomeChurch/prefiltered/32");
    ctx.cubemap[5] = cg::load_cubemap(cubemap_dir() + "/RomeChurch/prefiltered/8");
    ctx.cubemap[6] = cg::load_cubemap(cubemap_dir() + "/RomeChurch/prefiltered/2");
    ctx.cubemap[7] = cg::load_cubemap(cubemap_dir() + "/RomeChurch/prefiltered/0.5");
    ctx.cubemap[8] = cg::load_cubemap(cubemap_dir() + "/RomeChurch/prefiltered/0.125");

    gltf::load_gltf_asset(ctx.gltfFilename, gltf_dir(), ctx.asset);
    gltf::create_drawables_from_gltf_asset(ctx.drawables, ctx.asset);
}

void draw_scene(Context &ctx)
{
    // Activate shader program
    glUseProgram(ctx.program);

    // Set render state
    glEnable(GL_DEPTH_TEST);  // Enable Z-buffering

    // Define per-scene uniforms
    glUniform1f(glGetUniformLocation(ctx.program, "u_time"), ctx.elapsedTime);
    // ...

    // Draw scene
    for (unsigned i = 0; i < ctx.asset.nodes.size(); ++i) {
        const gltf::Node &node = ctx.asset.nodes[i];
        const gltf::Drawable &drawable = ctx.drawables[node.mesh];

        // Define per-object uniforms

        //Identity matrix
        glm::mat4 identityMatrix(1.0f);

        //model matrix
        glm::mat4 rotationMatrix = glm::mat4_cast(node.rotation);

        glm::mat4 translationMatrix = glm::translate(identityMatrix, node.translation);

        glm::mat4 scaleMatrix = glm::scale(identityMatrix, node.scale);

        glm::mat4 model = node.matrix;

        model = translationMatrix*rotationMatrix*scaleMatrix*model;

        glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_model"), 1, GL_FALSE, &model[0][0]);

        // View matrix
        glm::mat4 trackballView = glm::mat4(ctx.trackball.orient);

        glm::mat4 camera = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 view = camera*trackballView; //order can be switched, trackballview first = move camera, camera first = move model
        
        glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_view"), 1, GL_FALSE, &view[0][0]);

        //projection matrix
        glm::mat4 projection = identityMatrix;

        float fov = 45.0f;

        //Switch between perspective and orthographic projection
        if (ctx.ortho){
            projection = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.0f, 10.0f);
        }
        else {
           projection = glm::perspective(glm::radians(fov*zoomFactor), 1.0f, 0.1f, 100.0f); 
        }
        
        glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_projection"), 1, GL_FALSE, &projection[0][0]);

        //Cubemap
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_CUBE_MAP, ctx.cubemap[ctx.textureIndex]);

        glUniform1i(glGetUniformLocation(ctx.program, "u_cubemap"), 1 &GL_TEXTURE0);

        //shader uniforms passing
        glUniform3fv(glGetUniformLocation(ctx.program, "u_diffuseColor"), 1,  &ctx.diffuseColor[0]);
        glUniform3fv(glGetUniformLocation(ctx.program, "u_lightPosition"), 1,  &ctx.lightPosition[0]);
        glUniform3fv(glGetUniformLocation(ctx.program, "u_ambientColor"), 1,  &ctx.ambientColor[0]);
        glUniform3fv(glGetUniformLocation(ctx.program, "u_specularColor"), 1,  &ctx.specularColor[0]);
        glUniform1f(glGetUniformLocation(ctx.program, "u_specularPower"), ctx.specularPower);
        //pass shader toggle
        glUniform1i(glGetUniformLocation(ctx.program, "u_shaderToggle"), 1 &ctx.shaderToggle);
        //pass gamma toggle
        glUniform1i(glGetUniformLocation(ctx.program, "u_gammaToggle"), 1 &ctx.gammaToggle);
        //pass cubemap toggle
        glUniform1i(glGetUniformLocation(ctx.program, "u_cubemapToggle"), 1 &ctx.cubemapToggle);
        // ...

        // Draw object
        glBindVertexArray(drawable.vao);
        glDrawElements(GL_TRIANGLES, drawable.indexCount, drawable.indexType,
                       (GLvoid *)(intptr_t)drawable.indexByteOffset);
        glBindVertexArray(0);
    }

    // Clean up
    cg::reset_gl_render_state();
    glUseProgram(0);
}

void do_rendering(Context &ctx)
{
    // Clear render states at the start of each frame
    cg::reset_gl_render_state();

    // Clear color and depth buffers
    glClearColor(ctx.background_color.x, ctx.background_color.y, ctx.background_color.z, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_scene(ctx);
}

void reload_shaders(Context *ctx)
{
    glDeleteProgram(ctx->program);
    ctx->program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");
}

void error_callback(int /*error*/, const char *description)
{
    std::cerr << description << std::endl;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_R && action == GLFW_PRESS) { reload_shaders(ctx); }
}

void char_callback(GLFWwindow *window, unsigned int codepoint)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_CharCallback(window, codepoint);
    if (ImGui::GetIO().WantTextInput) return;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) return;

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ctx->trackball.center = glm::vec2(x, y);
        ctx->trackball.tracking = (action == GLFW_PRESS);
    }
}

void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    // Forward event to ImGui
    if (ImGui::GetIO().WantCaptureMouse) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    cg::trackball_move(ctx->trackball, float(x), float(y));
}

void scroll_callback(GLFWwindow *window, double x, double y)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_ScrollCallback(window, x, y);
    if (ImGui::GetIO().WantCaptureMouse) return;

    //change zoomFactor
    zoomFactor += y * 0.1f;
    if (zoomFactor < 0.1f) {
        zoomFactor = 0.1f;
    }
    else if (zoomFactor > 3.9f) {
        zoomFactor = 3.9f;
    }

}

void resize_callback(GLFWwindow *window, int width, int height)
{
    // Update window size and viewport rectangle
    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    ctx->width = width;
    ctx->height = height;
    glViewport(0, 0, width, height);
}

int main(int argc, char *argv[])
{
    Context ctx = Context();
    if (argc > 1) { ctx.gltfFilename = std::string(argv[1]); }

    // Create a GLFW window
    glfwSetErrorCallback(error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ctx.window = glfwCreateWindow(ctx.width, ctx.height, "Model viewer", nullptr, nullptr);
    glfwMakeContextCurrent(ctx.window);
    glfwSetWindowUserPointer(ctx.window, &ctx);
    glfwSetKeyCallback(ctx.window, key_callback);
    glfwSetCharCallback(ctx.window, char_callback);
    glfwSetMouseButtonCallback(ctx.window, mouse_button_callback);
    glfwSetCursorPosCallback(ctx.window, cursor_pos_callback);
    glfwSetScrollCallback(ctx.window, scroll_callback);
    glfwSetFramebufferSizeCallback(ctx.window, resize_callback);

    // Load OpenGL functions
    if (gl3wInit() || !gl3wIsSupported(3, 3) /*check OpenGL version*/) {
        std::cerr << "Error: failed to initialize OpenGL" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // Initialize ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(ctx.window, false /*do not install callbacks*/);
    ImGui_ImplOpenGL3_Init("#version 330" /*GLSL version*/);

    // Initialize rendering
    glGenVertexArrays(1, &ctx.emptyVAO);
    glBindVertexArray(ctx.emptyVAO);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    do_initialization(ctx);

    // Start rendering loop
    while (!glfwWindowShouldClose(ctx.window)) {
        glfwPollEvents();
        ctx.elapsedTime = glfwGetTime();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // ImGui::ShowDemoWindow();
        // ImGui window
        ImGui::Begin("Editor", NULL, 0);
        //Shading
        ImGui::ColorEdit3("Diffuse color", &ctx.diffuseColor[0]);
        ImGui::ColorEdit3("Ambient color", &ctx.ambientColor[0]);
        ImGui::ColorEdit3("Specular color", &ctx.specularColor[0]);
        ImGui::InputFloat("Specular Power", &ctx.specularPower);
        ImGui::InputFloat3("Light position", &ctx.lightPosition[0]);
        //Background
        ImGui::ColorEdit3("Background color", &ctx.background_color[0]);
        //Projection
        ImGui::Checkbox("Orthographic projection", &ctx.ortho);
        //Toggle shader
        ImGui::Checkbox("Toggle Shader", &ctx.shaderToggle);
        ImGui::Checkbox("Toggle Gamma Correction", &ctx.gammaToggle);
        ImGui::Checkbox("Toggle reflection", &ctx.cubemapToggle);
        ImGui::DragInt("Roughness", &ctx.textureIndex, 1.0f, 0, 8);
        ImGui::End();
        //
        do_rendering(ctx);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(ctx.window);
    }

    // Shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
    std::exit(EXIT_SUCCESS);
}
