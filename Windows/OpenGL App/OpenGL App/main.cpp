#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdint>
#include <cmath>

struct Matrix
{
    float m[4][4];
    Matrix(float scale = 0.0) : Matrix(
        scale, 0.0f , 0.0f , 0.0f,
        0.0f , scale, 0.0f , 0.0f,
        0.0f , 0.0f , scale, 0.0f,
        0.0f , 0.0f , 0.0f , scale
    )
    {}
    Matrix(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33
    )
    {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }
    Matrix operator * (const Matrix& mat) const
    {
        Matrix res;
        for (int j = 0; j < 4; j++)
            for (int i = 0; i < 4; i++)
                for (int k = 0; k < 4; k++)
                    res.m[j][i] += m[k][i] * mat.m[j][k];
        return res;
    }
};

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
}

Matrix ProjectionMatrix(float Near, float Far, float aspect)
{
    return Matrix(
        1.0f / aspect, 0.0f, 0.0f,                        0.0f,
        0.0f,          1.0f, 0.0f,                        0.0f,
        0.0f,          0.0f, Far / (Far - Near),          1.0f,
        0.0f,          0.0f, (Far * Near) / (Near - Far), 0.0f
    );
}

Matrix TranslateMatrix(float x, float y, float z)
{
    return Matrix(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
          x,   y,   z, 1.0
        );
}


Matrix RotationMatrix(float pitch, float yaw, float roll)
{
    return
        Matrix(
             cos(yaw), 0.0, sin(yaw), 0.0,
            0.0,       1.0, 0.0,      0.0,
            -sin(yaw), 0.0, cos(yaw), 0.0,
            0.0,       0.0, 0.0,      1.0
            ) *
        Matrix(
            1.0, 0.0,        0.0,        0.0,
            0.0, cos(pitch), sin(pitch), 0.0,
            0.0,-sin(pitch), cos(pitch), 0.0,
            0.0, 0.0,        0.0,        1.0
            ) *
        Matrix(
            cos(roll),-sin(roll), 0.0, 0.0,
            sin(roll), cos(roll), 0.0, 0.0,
            0.0,       0.0,       1.0, 0.0,
            0.0,       0.0,       0.0, 1.0
            );
}

int main(void)
{
    GLFWwindow* window;

    /* 初始化 GLFW 库 */
    if (!glfwInit())
        return -1;

    /* 创建窗口 */
    window = glfwCreateWindow(640, 480, "Rotating Cube", NULL, NULL);
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

    glEnable(GL_DEPTH_TEST);

    int32_t mat_proj_location
        = glGetUniformLocation(shader_program_object, "mat_proj");
    int32_t mat_trans_location
        = glGetUniformLocation(shader_program_object, "mat_trans");

    float pitch = 0.0, yaw = 0.0, roll = 0.0;

    glfwSwapInterval(0);

    /* 消息循环 */
    while (!glfwWindowShouldClose(window))
    {
        /* 在这里实现渲染代码 */
        glClearColor(0.6, 0.7, 0.8, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glClear(GL_DEPTH_BUFFER_BIT);

        int32_t width, height;
        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glUseProgram(shader_program_object);

        Matrix mat_proj = 
            ProjectionMatrix(1.0, 10.0, (float)width / (float)height);
        glUniformMatrix4fv(mat_proj_location, 1, false, (float*)&mat_proj);

        pitch += 0.01;
        yaw   += 0.02;
        roll  += 0.03;

        Matrix mat_trans = TranslateMatrix(0.0, 0.0, 5.0) * RotationMatrix(pitch, yaw, roll);
        glUniformMatrix4fv(mat_trans_location, 1, false, (float*)&mat_trans);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
        glVertexAttribPointer(0, 4, GL_FLOAT, false, 32, (void*)0);
        glVertexAttribPointer(1, 4, GL_FLOAT, false, 32, (void*)16);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

        /* 交换缓冲 */
        glfwSwapBuffers(window);

        /* 处理窗口消息 */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}