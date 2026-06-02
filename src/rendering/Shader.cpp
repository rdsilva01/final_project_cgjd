#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#ifdef USE_GLAD
#include <glad/glad.h>
#else
#include <GL/glew.h>
#endif

static std::string readFile(const char* path) {
    std::ifstream f; f.exceptions(std::ifstream::failbit|std::ifstream::badbit);
    try { f.open(path); std::stringstream ss; ss << f.rdbuf(); return ss.str(); }
    catch(...) { std::cerr << "ERROR::SHADER::FILE_NOT_FOUND: " << path << "\n"; return ""; }
}

Shader::Shader(const char* vertPath, const char* fragPath, const char* geomPath) {
    auto vc = readFile(vertPath), fc = readFile(fragPath);
    const char *vcs=vc.c_str(), *fcs=fc.c_str();

    unsigned int v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v,1,&vcs,NULL); glCompileShader(v); checkErrors(v,"VERTEX");

    unsigned int f2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f2,1,&fcs,NULL); glCompileShader(f2); checkErrors(f2,"FRAGMENT");

    unsigned int g = 0;
    if (geomPath) {
        auto gc = readFile(geomPath); const char* gcs = gc.c_str();
        g = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(g,1,&gcs,NULL); glCompileShader(g); checkErrors(g,"GEOMETRY");
    }

    ID = glCreateProgram();
    glAttachShader(ID,v); glAttachShader(ID,f2);
    if (g) glAttachShader(ID,g);
    glLinkProgram(ID); checkErrors(ID,"PROGRAM");

    glDeleteShader(v); glDeleteShader(f2); if (g) glDeleteShader(g);
}

void Shader::use()  const { glUseProgram(ID); }
void Shader::setBool (const std::string& n, bool   v) const { glUniform1i(glGetUniformLocation(ID,n.c_str()),(int)v); }
void Shader::setInt  (const std::string& n, int    v) const { glUniform1i(glGetUniformLocation(ID,n.c_str()),v); }
void Shader::setFloat(const std::string& n, float  v) const { glUniform1f(glGetUniformLocation(ID,n.c_str()),v); }
void Shader::setMat4 (const std::string& n, const glm::mat4& m) const { glUniformMatrix4fv(glGetUniformLocation(ID,n.c_str()),1,GL_FALSE,&m[0][0]); }
void Shader::setVec3 (const std::string& n, const glm::vec3& v) const { glUniform3fv(glGetUniformLocation(ID,n.c_str()),1,&v[0]); }

void Shader::checkErrors(unsigned int shader, const std::string& type) {
    int ok; char log[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);
        if (!ok) { glGetShaderInfoLog(shader,1024,NULL,log); std::cerr<<"SHADER ERROR ["<<type<<"]:\n"<<log<<"\n"; }
    } else {
        glGetProgramiv(shader,GL_LINK_STATUS,&ok);
        if (!ok) { glGetProgramInfoLog(shader,1024,NULL,log); std::cerr<<"PROGRAM LINK ERROR:\n"<<log<<"\n"; }
    }
}
