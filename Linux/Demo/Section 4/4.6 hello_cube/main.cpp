#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <cmath>

float vertex_buffer[8][8] = {
    {-1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
    {-1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
    {-1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
    { 1.0f,-1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
    { 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
    { 1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
    { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
};

uint32_t index_buffer[36] = {
    0,2,1,
    1,2,3,
    4,5,6,
    5,7,6,
    0,6,2,
    0,4,6,
    1,3,5,
    3,7,5,
    2,6,3,
    3,6,7,
    0,1,4,
    1,5,4
};

uint32_t vertex_buffer_object;
uint32_t index_buffer_object;

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

/* ????????????????????????????????????????????????????????? */
void InitAssets()
{
    for (int i = 0; i < 8; i++)
        vertex_buffer[i][2] += 5.0;
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
}

void ProjectionMatrix(float Near, float Far, float* mat_data)
{
    float data[] = {
        1.0f, 0.0f, 0.0f,                        0.0f,
        0.0f, 1.0f, 0.0f,                        0.0f,
        0.0f, 0.0f, Far / (Far - Near),          1.0f,
        0.0f, 0.0f, (Far * Near) / (Near - Far), 0.0f
    };
    memcpy(mat_data, data, sizeof(data));
}

int main(void)
{
    GLFWwindow* window;

    /* ????????? GLFW ??? */
    if (!glfwInit())
        return -1;

    /* ???????????? */
    window = glfwCreateWindow(640, 480, "Hello Cube", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* ????????????????????????????????? */
    glfwMakeContextCurrent(window);

    /* ????????? GLAD ???????????????????????????????????? glXXX ?????? */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }

    /* ???????????????????????? GL ?????? */
    std::cout << "GPU:         " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GPU Vendor:  " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL Version:  " << glGetString(GL_VERSION) << std::endl;

    /* ????????????????????? */

    InitAssets();

    glEnable(GL_DEPTH_TEST);

    int32_t mat_proj_location
        = glGetUniformLocation(shader_program_object, "mat_proj");

    /* ???????????? */
    while (!glfwWindowShouldClose(window))
    {
        /* ??????????????????????????? */
        glClearColor(0.6, 0.7, 0.8, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glClear(GL_DEPTH_BUFFER_BIT);

        int32_t width, height;
        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glUseProgram(shader_program_object);

        float mat_proj_data[16];
        ProjectionMatrix(1.0, 10.0, mat_proj_data);
        glUniformMatrix4fv(mat_proj_location, 1, false, mat_proj_data);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
        glVertexAttribPointer(0, 4, GL_FLOAT, false, 32, (void*)0);
        glVertexAttribPointer(1, 4, GL_FLOAT, false, 32, (void*)16);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

        /* ???????????? */
        glfwSwapBuffers(window);

        /* ?????????????????? */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}