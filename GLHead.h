#ifndef GLHEAD_H
#define GLHEAD_H

#include <GL/glew.h>

#define ASSERT(x) if(!(x)) __builtin_debugtrap();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))



void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);


#endif // GLHEAD_H
