#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdint>
#include <cmath>

float vertex_buffer[4][6] = {
    {-1.0, -1.0, 0.0, 1.0,-1.0,-1.0},
    {-1.0,  1.0, 0.0, 1.0,-1.0, 2.0},
    { 1.0, -1.0, 0.0, 1.0, 2.0,-1.0},
    { 1.0,  1.0, 0.0, 1.0, 2.0, 2.0}
};

uint32_t index_buffer[6] = {
    0,2,1,
    1,2,3,
};

uint32_t vertex_buffer_object;
uint32_t index_buffer_object;

uint32_t shader_program_object;

uint32_t texture_object;

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

    glCreateBuffers(1, &index_buffer_object);
    glNamedBufferData(index_buffer_object, sizeof(index_buffer), nullptr, GL_STATIC_DRAW);
    glNamedBufferSubData(index_buffer_object, 0, sizeof(index_buffer), index_buffer);


    uint32_t vertex_shader_object = CompileGLSLShaderFromFile("vertex_shader.glsl", GL_VERTEX_SHADER);
    uint32_t fragment_shader_object = CompileGLSLShaderFromFile("fragment_shader.glsl", GL_FRAGMENT_SHADER);

    shader_program_object = LinkProgram(vertex_shader_object, fragment_shader_object);
    glDeleteShader(vertex_shader_object);
    glDeleteShader(fragment_shader_object);

    uint8_t pixel_data[256][256][3];
    for (int i = 0; i < 256; i++)
        for (int j = 0; j < 256; j++)
        {
            pixel_data[i][j][0] = i ^ j;
            pixel_data[i][j][1] = i ^ j;
            pixel_data[i][j][2] = i ^ j;
        }

    glCreateTextures(GL_TEXTURE_2D, 1, &texture_object);
    glBindTexture(GL_TEXTURE_2D, texture_object);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int main(void)
{
    GLFWwindow* window;

    /* 初始化 GLFW 库 */
    if (!glfwInit())
        return -1;

    /* 创建窗口 */
    window = glfwCreateWindow(768, 768, "Texture", NULL, NULL);
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
        glVertexAttribPointer(0, 4, GL_FLOAT, false, 24, (void*)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 24, (void*)16);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
        glBindTexture(GL_TEXTURE_2D, texture_object);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /* 交换缓冲 */
        glfwSwapBuffers(window);

        /* 处理窗口消息 */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}