#ifndef _SHADER_HELPER_H
#define _SHADER_HELPER_H

#include "GL/glew.h"
#include "FL/gl.h"
#include <vector>

bool createShaderCompiled(const char* file, GLenum shaderType, GLuint& shaderID);
bool createProgramLinked(const std::vector<GLuint>& shaders, GLuint& programID);
#endif // _SHADER_HELPER_H