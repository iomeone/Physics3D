﻿#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "../../util/Log.h"

#include <fstream>
#include <sstream>

void Shader::createUniform(std::string uniform) {
	int location = glGetUniformLocation(id, uniform.c_str());
	if (location < 0)
		Log::error("Could not find uniform (%s)", uniform.c_str());
	uniforms.insert(std::make_pair(uniform, location));
}

void Shader::setUniform(std::string uniform, int value) {
	glUniform1i(uniforms[uniform], value);
}

void Shader::setUniform(std::string uniform, float value) {
	glUniform1f(uniforms[uniform], value);
}

void Shader::setUniform(std::string uniform, double value) {
	glUniform1d(uniforms[uniform], value);
}

void Shader::setUniform(std::string uniform, Vec3 value) {
	glUniform3d(uniforms[uniform], value.x, value.y, value.z);
}

void Shader::setUniform(std::string uniform, Mat4 value) {
	glUniformMatrix4dv(uniforms[uniform], 1, GL_FALSE, reinterpret_cast<double*>(&value));
}

unsigned int compileShader(const std::string& source, unsigned int type) {
	unsigned int id = glCreateShader(type);
	
	const char* src = source.c_str();
	
	glShaderSource(id, 1, &src, nullptr);
	
	glCompileShader(id);

	// Error handling
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*) alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);

		Log::error(message);
		
		glDeleteShader(id);
		return 0;
	}
	return id;
}

unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader) {
	unsigned int program = glCreateProgram();

	Log::info("Compiling vertex shader");
	unsigned int vs = compileShader(vertexShader, GL_VERTEX_SHADER);
	Log::info("Done compiling vertex shader");

	Log::info("Compiling fragment shader");
	unsigned int fs = compileShader(fragmentShader, GL_FRAGMENT_SHADER);
	Log::info("Done compiling fragment shader");

	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

ShaderSource parseShader(const std::string& vertexPath, const std::string& fragmentPath) {
	Log::info("Started parsing vertex shader: (%s), and fragment shader: %s)", vertexPath.c_str(), fragmentPath.c_str());
	std::string line;
	std::ifstream vertexFileStream(vertexPath);
	std::ifstream fragmentFileStream(fragmentPath);
	std::stringstream vertexStringStream;
	std::stringstream fragmentStringStream;

	if (vertexFileStream.fail()) {
		Log::fatal("File could not be opened: %s", vertexPath.c_str());
	}

	if (fragmentFileStream.fail()) {
		Log::fatal("File could not be opened: %s", fragmentPath.c_str());
	}

	while (getline(vertexFileStream, line)) {
		vertexStringStream << line << "\n";
	}

	while (getline(fragmentFileStream, line)) {
		fragmentStringStream << line << "\n";
	}

	vertexFileStream.close();
	fragmentFileStream.close();
	Log::info("Parsed vertex shader: (%s), and fragment shader: %s)", vertexPath.c_str(), fragmentPath.c_str());
	return { vertexStringStream.str(), fragmentStringStream.str() };
}

ShaderSource parseShader(const std::string& path) {
	Log::info("Started parsing fragment and vertex shader: (%s)", path.c_str());
	enum class ShaderType {
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1
	};

	ShaderType type = ShaderType::NONE;

	std::ifstream fileStream(path);
	std::string line;
	std::stringstream stringStream[2];
	
	if (fileStream.fail()) {
		Log::fatal("File could not be opened: %s", path.c_str());
	}

	int lineNumber = 0;
	while (getline(fileStream, line)) {
		lineNumber++;
		if (line.find("#shader vertex") != std::string::npos) {
			type = ShaderType::VERTEX;
		} else if (line.find("#shader fragment") != std::string::npos) {
			type = ShaderType::FRAGMENT;
		} else {
			if (type == ShaderType::NONE) {
				Log::warn("(line %d): Code in (%s) before the first #shader instruction will be ignored", lineNumber, path.c_str());
				continue;
			}
			stringStream[(int) type] << line << "\n";
		}
	}

	fileStream.close();
	Log::info("Parsed fragment and vertex shader: (%s)", path.c_str());

	return { stringStream[(int)ShaderType::VERTEX].str(), stringStream[(int)ShaderType::FRAGMENT].str() };
}

unsigned int Shader::getId() {
	return id;
}

Shader::Shader(const std::string& vertexShader, const std::string& fragmentShader) {
	id = createShader(vertexShader, fragmentShader);
}

void Shader::bind() {
	glUseProgram(id);
}

void Shader::unbind() {
	glUseProgram(0);
}

void Shader::close() {
	unbind();
	if (id != 0) {
		glDeleteProgram(id);
	}
}

