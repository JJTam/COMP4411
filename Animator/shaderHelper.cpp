#include "shaderHelper.h"
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

unsigned long getFileLength(ifstream& file)
{
	if (!file.good()) return 0;

	unsigned long pos = file.tellg();
	file.seekg(0, ios::end);
	unsigned long len = file.tellg();
	file.seekg(ios::beg);

	return len;
}

bool createShaderCompiled(const char* filename, GLenum shaderType, GLhandleARB& shaderID)
{
	// read the file and set source
	ifstream ifs;
	ifs.open(filename, ios::in);
	if (!ifs.is_open())
	{
		printf("Can not open file %s.\n", filename);
		return false;
	}
	
	GLint* pLen = new GLint;
	auto len = getFileLength(ifs);
	*pLen = len;
	if (len < 0)
	{
		printf("File %s has length zero.\n", filename);
		ifs.close();
		return false;
	}

	GLchar** source = new GLchar*;
	source[0] = new GLchar[len + 1];
	source[0][len] = 0;

	unsigned int lastPos = 0;
	while (ifs.good())
	{
		source[0][lastPos] = ifs.get();
		if (!ifs.eof())
			++lastPos;
	}
	source[0][lastPos] = 0;
	ifs.close();

	// create the shader
	shaderID = glCreateShaderObjectARB(shaderType);
	glShaderSourceARB(shaderID, 1, (const GLcharARB**) source, NULL);
	glCompileShaderARB(shaderID);
	
	GLint isCompiled = 0;
	glGetObjectParameterivARB(shaderID, GL_OBJECT_COMPILE_STATUS_ARB, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		printf("Compilation failed! (%s) \n", filename);

		GLint maxLength = 0;
		glGetObjectParameterivARB(shaderID, GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);
		GLchar* errorLog = new GLchar[maxLength];
		glGetInfoLogARB(shaderID, maxLength, &maxLength, errorLog);
		printf("%s\n", errorLog);
		delete[] errorLog;

		glDeleteObjectARB(shaderID);
		return false;
	}

	return true;
}

bool createProgramLinked(const std::vector<GLhandleARB>& shaders, GLhandleARB& programID)
{
	programID = glCreateProgramObjectARB();
	for_each(shaders.begin(), shaders.end(), [programID](const GLhandleARB& id)->void {
		glAttachObjectARB(programID, id);
	});
	glLinkProgramARB(programID);

	GLint linked = GL_FALSE;
	glGetObjectParameterivARB(programID, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
	{
		printf("Linkage failed! \n");

		GLint maxLength = 0;
		glGetObjectParameterivARB(programID, GL_INFO_LOG_LENGTH, &maxLength);
		GLchar* errorLog = new GLchar[maxLength];
		glGetInfoLogARB(programID, maxLength, &maxLength, errorLog);
		printf("%s\n", errorLog);
		delete[] errorLog;

		glDeleteObjectARB(programID);
		return false;
	}

	return true;
}

bool createProgramWithTwoShaders(const char* vertShader, const char* fragShader, GLhandleARB& programID)
{
	GLhandleARB vertID;
	GLhandleARB fragID;

	if (!createShaderCompiled(vertShader, GL_VERTEX_SHADER, vertID) ||
		!createShaderCompiled(fragShader, GL_FRAGMENT_SHADER, fragID) ||
		!createProgramLinked(vector<GLhandleARB> { vertID, fragID }, programID))
	{
		return false;
	}
	
	return true;
}