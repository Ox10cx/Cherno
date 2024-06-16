#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "GLHead.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "stb_image.h"

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

/**
 * @brief 生成VAO 绑定VAO 
 * 采用singleBuffer进行VA0的构建的策略
 * 
 */
static void prepareSingleBuffer() {
    // 1、准备position数据 和 color数据
    float positions[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
    };

    float colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    // 2、使用数据生成两个vbo posVbo colorVbo
    unsigned int posVbo, colorVbo;
    GLCall(glGenBuffers(1, &posVbo));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, posVbo));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW));

    GLCall(glGenBuffers(1, &colorVbo));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, colorVbo));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW));
    
    // 3、生成vao并且绑定
    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    // 4、分别将位置/颜色属性描述信息加入vao中
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, posVbo));
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0));

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, colorVbo));
    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0));

    // // 5、将vao进行解绑
    // GLCall(glBindVertexArray(0));

}

/**
 * @brief 生成VAO 绑定VAO 
 * 采用interleavedBuffer进行VA0的构建的策略
 * 
 */
static void perpareInterleavedBuffer() {
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
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0));

    // 顶点颜色描述信息
    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float))));

    // // 5、将vao进行解绑
    // GLCall(glBindVertexArray(0));

}

/**
 * @brief 用于存储顶点绘制顺序索引序号的GPU显存区域
 * 
 * 
 */
static void prepareEBO() {
    float positions[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f, 
    };

    float uvs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,
    };


    // 生成VBO 并绑定VBO  GPU顶点数据
    unsigned int posVBO;
    GLCall(glGenBuffers(1, &posVBO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, posVBO));
    GLCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

    unsigned int uvVBO;
    GLCall(glGenBuffers(1, &uvVBO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, uvVBO));
    GLCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), uvs, GL_STATIC_DRAW));
    

    // 生成VAO并且绑定   VAO 网格顶点的描述信息
    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    // 将顶点描述信息加入VAO中
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, posVBO)); // 只有绑定VBO 下面才会次属于与VBO相关
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void *)0));

    // 加入uv属性描述数据
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, uvVBO));
    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void *)0));
   

    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    // 同样也会将EBO的信息添加都VAO中，注意与glVertexAttribPointer 的调用先后关系
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));
}

static unsigned int prepareShader() {
    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    std::cout << source.VertexSource << std::endl;
    std::cout << source.FragmentSource << std::endl;

    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    GLCall(glUseProgram(shader));

    return shader;
}


static unsigned int prepareTextureShader() {
    ShaderProgramSource source = ParseShader("res/shaders/Basic_texture.shader");
    std::cout << source.VertexSource << std::endl;
    std::cout << source.FragmentSource << std::endl;

    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);

    GLCall(glUseProgram(shader));
    return shader;
}

static void transDataToShader(unsigned int shader, const std::string& name, int value) {
    GLCall(int location = glGetUniformLocation(shader, name.c_str()));
    GLCall(glUniform1i(location, value));
}


/**
 * @brief 
 * 创建纹理对象
 *   stbImage 读取图片
 *   生成纹理单元并且激活单元绑定
 *   传输纹理数据
 * 
 * 设置纹理过滤
 *   当需要像素 大于 图片像素 Linear
 *   当需要像素 小于 图片像素 Nearest
 * 
 * 设置纹理包裹方式（Wrapping)
 *   -Repeat Mirror, ClampToEdge ClampToBorder
 * 
 * 采样器与采样
 *     -加入uv属性数据，并且单独开辟一个vbo
 *     -vs中加入uv属性的获取以及向下一个阶段的传输，fs中加入获取uv数值
 *     -fs中加入采样器，并且通过采样器采样颜色
 *     -将采样器与某一个纹理单元绑定
 * 
 * 
 */
static void prepareTexture() {
    // 1、stbImage 读取图片
    int width, height, channels;
    // 反转y轴
    stbi_set_flip_vertically_on_load(true);
    unsigned char * data = stbi_load("res/textures/ChernoLogo.png", &width, &height, &channels, STBI_rgb_alpha);

    // 2、生成纹理单元并且激活单元绑定
    unsigned int texture;
    glGenTextures(1, &texture);
    //-- 激活纹理单元
    glActiveTexture(GL_TEXTURE0);
    //-- 绑定纹理对象
    glBindTexture(GL_TEXTURE_2D, texture);

    // 3、传输纹理数据, 开辟显存
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // 4、释放数据
    stbi_image_free(data);

    // 5 设置纹理过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // 6 设置纹理的包裹方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


static void deleteShader(unsigned int shader) {
     GLCall(glDeleteShader(shader));
}

static void render() {
    /* Render here */
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

static void renderEBO() {
    /* Render here */
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
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

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // prepareSingleBuffer();
    // perpareInterleavedBuffer();
    prepareEBO();

    //unsigned int shader = prepareShader();

    prepareTexture();
    unsigned int shader = prepareTextureShader();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        //  render();
        
        transDataToShader(shader, "sampler", 0);
        renderEBO();
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // 移除shader
    deleteShader(shader);

    glfwTerminate();
    return 0;
}
