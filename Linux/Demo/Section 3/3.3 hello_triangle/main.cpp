#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <cstdint>

float vertex_buffer[3][8] = {
    { 0.0f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
    { 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
};

float vertex_buffer_copy[3][8];

uint32_t vertex_buffer_object;

uint32_t shader_program_object;

uint32_t CompileGLSLShaderFromFile(
    const char* shader_file_path,
    uint32_t shader_type
)
{
    FILE* file = nullptr;
#ifdef _WIN32
    fopen_s(&file, shader_file_path, "rb");
#else
    file = fopen(shader_file_path, "rb");
#endif
    if (!file) return 0;
    fseek(file, 0, SEEK_END);
    int64_t length = ftell(file);
    char* source_code = new char[length + 1];
    memset(source_code, 0, length + 1);
    fseek(file, 0, SEEK_SET);
    fread(source_code, 1, length, file);
    fclose(file);

    uint32_t shader_object = glCreateShader(shader_type);
    glShaderSource(shader_object, 1, &source_code, nullptr);
    delete[] source_code;
    glCompileShader(shader_object);

    int32_t compilation_success;
    glGetShaderiv(shader_object, GL_COMPILE_STATUS, &compilation_success);
    if (!compilation_success)
    {
        int32_t log_length;
        glGetShaderiv(shader_object, GL_INFO_LOG_LENGTH, &log_length);
        char* error_info = new char[log_length + 1];
        glGetShaderInfoLog(shader_object, log_length + 1, &log_length, error_info);
        std::cout << error_info << std::endl;
        glDeleteShader(shader_object);
        return 0;
    }
    return shader_object;
}

uint32_t LinkProgram(uint32_t vs_object, uint32_t fs_object)
{
    uint32_t program_object = glCreateProgram();
    glAttachShader(program_object, vs_object);
    glAttachShader(program_object, fs_object);
    glLinkProgram(program_object);
    int32_t link_success;
    glGetProgramiv(program_object, GL_LINK_STATUS, &link_success);
    if (!link_success)
    {
        int32_t log_length;
        glGetProgramiv(program_object, GL_INFO_LOG_LENGTH, &log_length);
        char * error_info = new char [log_length + 1];
        glGetProgramInfoLog(program_object, log_length + 1, &log_length, error_info);
        std::cout << error_info << std::endl;
        glDeleteProgram(program_object);
        delete[] error_info;
        return 0;
    }
    return program_object;
}

/* 这个函数用于初始化渲染过程中用到的资源 */
void InitAssets()
{
    glCreateBuffers(1, &vertex_buffer_object);
    glNamedBufferData(vertex_buffer_object, sizeof(vertex_buffer), nullptr, GL_STATIC_DRAW);
    glNamedBufferSubData(vertex_buffer_object, 0, sizeof(vertex_buffer), vertex_buffer);


    uint32_t vertex_shader_object = CompileGLSLShaderFromFile("vertex_shader.glsl", GL_VERTEX_SHADER);
    uint32_t fragment_shader_object = CompileGLSLShaderFromFile("fragment_shader.glsl", GL_FRAGMENT_SHADER);

    shader_program_object = LinkProgram(vertex_shader_object, fragment_shader_object);
    glDeleteShader(vertex_shader_object);
    glDeleteShader(fragment_shader_object);
}

int main(void)
{
    GLFWwindow* window;

    /* 初始化 GLFW 库 */
    if (!glfwInit())
        return -1;

    /* 创建窗口 */
    window = glfwCreateWindow(640, 480, "Hello Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* 将窗口设置为当前上下文 */
    glfwMakeContextCurrent(window);

    /* 初始化 GLAD 库，在这一步之后才能调用 glXXX 函数 */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }

    /* 显示设备信息以及 GL 版本 */
    std::cout << "GPU:         " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GPU Vendor:  " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL Version:  " << glGetString(GL_VERSION) << std::endl;

    /* 调用初始化函数 */

    InitAssets();

    /* 消息循环 */
    while (!glfwWindowShouldClose(window))
    {
        /* 在这里实现渲染代码 */
        glClearColor(0.6, 0.7, 0.8, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        int32_t width, height;
        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glUseProgram(shader_program_object);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
        glVertexAttribPointer(0, 4, GL_FLOAT, false, 32, (void*)0);
        glVertexAttribPointer(1, 4, GL_FLOAT, false, 32, (void*)16);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        /* 交换缓冲 */
        glfwSwapBuffers(window);

        /* 处理窗口消息 */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}