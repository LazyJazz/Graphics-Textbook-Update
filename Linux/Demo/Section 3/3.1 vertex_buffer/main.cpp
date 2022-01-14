#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdint>

float vertex_buffer[3][8] = {
    { 0.0f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
    { 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
};

float vertex_buffer_copy[3][8];

uint32_t vertex_buffer_object;

/* 这个函数用于初始化渲染过程中用到的资源 */
void InitAssets()
{
    glCreateBuffers(1, &vertex_buffer_object);
    glNamedBufferData(vertex_buffer_object, sizeof(vertex_buffer), nullptr, GL_STATIC_DRAW);
    glNamedBufferSubData(vertex_buffer_object, 0, sizeof(vertex_buffer), vertex_buffer);

    glGetNamedBufferSubData(vertex_buffer_object, 0, sizeof(vertex_buffer_copy), vertex_buffer_copy);

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 8; j++)
            printf("%.3lf%c", vertex_buffer_copy[i][j], j == 7 ? '\n' : '\t');
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

        /* 交换缓冲 */
        glfwSwapBuffers(window);

        /* 处理窗口消息 */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}