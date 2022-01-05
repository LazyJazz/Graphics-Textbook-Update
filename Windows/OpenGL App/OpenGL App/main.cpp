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

/* ����������ڳ�ʼ����Ⱦ�������õ�����Դ */
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

    /* ��ʼ�� GLFW �� */
    if (!glfwInit())
        return -1;

    /* �������� */
    window = glfwCreateWindow(640, 480, "Hello Window", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* ����������Ϊ��ǰ������ */
    glfwMakeContextCurrent(window);

    /* ��ʼ�� GLAD �⣬����һ��֮����ܵ��� glXXX ���� */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }

    /* ��ʾ�豸��Ϣ�Լ� GL �汾 */
    std::cout << "GPU:         " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GPU Vendor:  " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL Version:  " << glGetString(GL_VERSION) << std::endl;

    /* ���ó�ʼ������ */

    InitAssets();

    /* ��Ϣѭ�� */
    while (!glfwWindowShouldClose(window))
    {
        /* ������ʵ����Ⱦ���� */
        glClearColor(0.6, 0.7, 0.8, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        /* �������� */
        glfwSwapBuffers(window);

        /* ��������Ϣ */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}