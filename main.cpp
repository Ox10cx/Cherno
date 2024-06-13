#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "GLHead.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

struct ShaderProgramSource
{
   std::string VertexSource;
   std::string FragmentSource;
};


ShaderProgramSource ParseShader(const std::string& filepath)
{
   std::ifstream stream(filepath);
   enum class ShaderType {
       NONE = -1, VERTEX = 0, FRAGMENT = 1
   };

   std::string line;
   std::stringstream ss[2];
   ShaderType type = ShaderType::NONE;
   while (getline(stream, line)) {
       if (line.find("#shader") != std::string::npos) {
           if (line.find("vertex") != std::string::npos) {
               type = ShaderType::VERTEX;
           }
           else if (line.find("fragment") != std::string::npos) {
               type = ShaderType::FRAGMENT;
           }
       }
       else {
           ss[(int)type] << line << '\n';
       }
   }
   return { ss[0].str(),ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
   GLCall(unsigned int id = glCreateShader(type));
   const char* src = source.c_str();
   GLCall(glShaderSource(id, 1, &src, nullptr));
   GLCall(glCompileShader(id));

   int result;
   GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
   if (result == GL_FALSE)
   {
       int length;
       GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
       char* message = (char *)alloca(length * sizeof (char));
       GLCall(glGetShaderInfoLog(id, length, &length, message));
       std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? " vertexShader" : "fragmentShader" )  << " shader !"<< std::endl;
       std::cout << message << std::endl;
       GLCall(glDeleteShader(id));
       return 0;
   }
   return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
   GLCall(unsigned int program = glCreateProgram());
   unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
   unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

   GLCall(glAttachShader(program, vs));
   GLCall(glAttachShader(program, fs));
   GLCall(glLinkProgram(program));
   GLCall(glValidateProgram(program));

   GLCall(glDeleteShader(vs));
   GLCall(glDeleteShader(fs));
   return program;
}

static void interleavedBuffer() {
    float vertex[] = {
        -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,   0.0f, 0.0f, 1.0f,
    };

    // 创建vbo
    unsigned int vbo;
    GLCall(glGenBuffers(1, &vbo));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW));

    // 创建vao
    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    //为 vao加入顶点描述信息
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    // 顶点坐标描述信息
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2 * sizeof(float), GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0));

    // 顶点颜色描述信息
    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(0, 3 * sizeof(float), GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float))));
  
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // // 核心配置 没有默认的vao，需要我们显示配置
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    // 在 Mac 上要加这一句配置，否则不显示
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
           std::cout<<"glewinit error" << std::endl;

    std::cout<< glGetString(GL_VERSION) << std::endl;

    float positions[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f, 
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    //  You're seeing this error on OS X because it only supports the OpenGL Core Profile 
    //  if you're using OpenGL 3.x or higher. Your code is not Core Profile compliant. 
    // You were most likely using the Compatibility Profile on Windows.
    // Specifically, the Core Profile requires a Vertex Array Object (VAO) to be bound for all vertex related calls. So before calling glEnableVertexAttribArray(), or other similar functions, you will need to create and bind a VAO:

    // 生成VBO 并绑定VBO  GPU顶点数据
    unsigned int posVBO;
    GLCall(glGenBuffers(1, &posVBO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, posVBO));
    GLCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW));
    

    // 生成VAO并且绑定   VAO 网格顶点的描述信息
    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    // 将顶点描述信息加入VAO中
    // GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer)); 只有绑定VBO 下面才会次属于与VBO相关
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));
   

    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    // 同样也会将EBO的信息添加都VAO中，注意与glVertexAttribPointer 的调用先后关系
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));


    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    std::cout << source.VertexSource << std::endl;
    std::cout << source.FragmentSource << std::endl;

    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    GLCall(glUseProgram(shader));

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    GLCall(glDeleteShader(shader));
    glfwTerminate();
    return 0;
}
