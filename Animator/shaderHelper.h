#ifndef _SHADER_HELPER_H
#define _SHADER_HELPER_H

#include "GL/glew.h"
#include "FL/gl.h"
#include <vector>

bool createShaderCompiled(const char* file, GLenum shaderType, GLhandleARB& shaderID);
bool createProgramLinked(const std::vector<GLhandleARB>& shaders, GLhandleARB& programID);
bool createProgramWithTwoShaders(const char* vertShader, const char* fragShader, GLhandleARB& programID);
#endif // _SHADER_HELPER_H