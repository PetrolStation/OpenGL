#include <PCH.h>

#include "Core/Aliases.h"
#include "Core/DebugTools.h"
#include "Core/Renderer/Shader.h"
#include "OpenGLShader.h"

#include <Core/Files.h>

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>
#include <shaderc/shaderc.hpp>

#include <glad/glad.h>

//
// INFO
// 1. I use here properties of glShaderXXX(shader) which is stated at khronos site "A value of 0 for shader will be silently ignored."
//

void errorCheck()
{
    GLenum err;
    while ( ( err = glGetError() ) != GL_NO_ERROR) {
        std::cerr << err;
    }
}

namespace PetrolEngine {

    void OpenGLShader::bindUniformBuffer(const String& name, UniformBuffer* uniformBuffer) {
        uint32 bind = this->metadata.uniforms[name];
        glUniformBlockBinding(this->ID, bind, uniformBuffer->getBinding());
    }

    Vector<uint32>* OpenGLShader::fromSpvToGlslSpv(Vector<uint32>* spv, ShaderType type) { LOG_FUNCTION();
        String cacheName = "glsl_" + name + shaderTypeToShadercExtensionS(type) + ".cache";
        if(std::filesystem::exists(cacheName) && !Shader::alwaysCompile){ 
            std::ifstream file(cacheName, std::ios::in | std::ios::binary);

            file.seekg (0, file.end);
            int length = file.tellg();
            file.seekg (0, file.beg);
            
            Vector<uint32>* res = new Vector<uint32>;
            res->resize(length/sizeof(uint32));
            file.read((char*)res->data(), length);
            return res;
        }
        
        String source = spirv_cross::CompilerGLSL(*spv).compile();

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        options.SetTargetEnvironment(shaderc_target_env_opengl, 450);

        auto result = compiler.CompileGlslToSpv(
            source,
            shaderTypeToShadercShaderKind(type),
            this->name.c_str(),
            options
        );

        if(!result.GetCompilationStatus()){
            std::ofstream file(cacheName, std::ios::out | std::ios::binary);
            Vector<uint32>* res = new Vector<uint32>(result.cbegin(), result.cend());

            file.write((char*)res->data(), res->size()*sizeof(uint32));
            file.close();
            return res;
        }

        LOG("Shader compilation failed: " + result.GetErrorMessage(), 3);
        return nullptr;
    }

    void OpenGLShader::compileFromSpv(Vector<uint32>*   vertexByteCode,
                                      Vector<uint32>* fragmentByteCode,
                                      Vector<uint32>* geometryByteCode ){ LOG_FUNCTION();
        uint   vertexShaderID = 0;
        uint fragmentShaderID = 0;
        uint geometryShaderID = 0;
        uint programID        = glCreateProgram();

        if (vertexByteCode) {
            Vector<uint32>* vertexByteCodeGlsl = fromSpvToGlslSpv(vertexByteCode, ShaderType::Vertex);

            vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
            glShaderBinary(1, &vertexShaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, vertexByteCodeGlsl->data(), vertexByteCodeGlsl->size() * sizeof(uint32));
            glSpecializeShader(vertexShaderID, "main", 0, nullptr, nullptr);
            glAttachShader(programID, vertexShaderID);

            delete vertexByteCodeGlsl;
        }

        if (fragmentByteCode) {
            Vector<uint32>* fragmentByteCodeGlsl = fromSpvToGlslSpv(fragmentByteCode, ShaderType::Fragment);

            fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderBinary(1, &fragmentShaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, fragmentByteCodeGlsl->data(), fragmentByteCodeGlsl->size() * sizeof(uint32));
            glSpecializeShader(fragmentShaderID, "main", 0, nullptr, nullptr);
            glAttachShader(programID, fragmentShaderID);

            delete fragmentByteCodeGlsl;
        }

        if (geometryByteCode) {
            Vector<uint32>* geometryByteCodeGlsl = fromSpvToGlslSpv(geometryByteCode, ShaderType::Geometry);

            geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderBinary(1, &geometryShaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, geometryByteCodeGlsl->data(), geometryByteCodeGlsl->size() * sizeof(uint32));
            glSpecializeShader(geometryShaderID, "main", 0, nullptr, nullptr);
            glAttachShader(programID, geometryShaderID);

            delete geometryByteCodeGlsl;
        }

        glLinkProgram(programID);

        int error = checkProgramCompileErrors(programID);

        // if error occurred return without changing anything
        if(error) {
            glDeleteShader (  vertexShaderID);
            glDeleteShader (fragmentShaderID);
            glDeleteShader (geometryShaderID);
            glDeleteProgram(       programID);

            return;
        }

        // delete everything that exists
        if (this->  vertexShaderID) glDeleteShader (this->  vertexShaderID);
        if (this->fragmentShaderID) glDeleteShader (this->fragmentShaderID);
        if (this->geometryShaderID) glDeleteShader (this->geometryShaderID);
        if (this->              ID) glDeleteProgram(this->              ID);

        // replace with new
        this->  vertexShaderID =   vertexShaderID;
        this->fragmentShaderID = fragmentShaderID;
        this->geometryShaderID = geometryShaderID;
        this->              ID =        programID;
    }

    OpenGLShader::OpenGLShader( String         name,
                                String   vertexCode,
                                String fragmentCode,
                                String geometryCode ) {
        this->vertexShaderSourceCode   =   vertexCode;
        this->fragmentShaderSourceCode = fragmentCode;
        this->geometryShaderSourceCode = geometryCode;

        this->name = name;
        this->compile();
    }

    OpenGLShader::~OpenGLShader() {
        glDeleteShader(vertexShaderID  );
        glDeleteShader(fragmentShaderID);
        glDeleteShader(geometryShaderID);

        glDeleteProgram(this->ID);
    }

    void OpenGLShader::compileNative( const String& vertexShaderSourceCode  ,
                                      const String& fragmentShaderSourceCode,
                                      const String& geometryShaderSourceCode ) { LOG_FUNCTION();
        this->  vertexShaderID = 0;
        this->fragmentShaderID = 0;
        this->geometryShaderID = 0;

        this->ID = glCreateProgram();


        // creating and attaching vertex shader
        {
            this->vertexShaderID = glCreateShader(GL_VERTEX_SHADER);

            const char* source = vertexShaderSourceCode.c_str();
            glShaderSource (vertexShaderID, 1, &source, nullptr);
            glCompileShader(vertexShaderID);

            checkShaderCompileErrors(vertexShaderID, "VERTEX");

            glAttachShader(ID, vertexShaderID);
        }

        // creating and attaching fragment shader
        {
            this->fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

            const char* source = fragmentShaderSourceCode.c_str();
            glShaderSource(fragmentShaderID, 1, &source, nullptr);
            glCompileShader(fragmentShaderID);

            checkShaderCompileErrors(fragmentShaderID, "FRAGMENT");

            glAttachShader(ID, fragmentShaderID);
        }

        // creating and attaching geometry shader if given
        if (!geometryShaderSourceCode.empty())
        {
            this->geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

            const char* source = geometryShaderSourceCode.c_str();
            glShaderSource (geometryShaderID, 1, &source, nullptr);
            glCompileShader(geometryShaderID);

            checkShaderCompileErrors(geometryShaderID, "GEOMETRY");

            glAttachShader(ID, geometryShaderID);
        }

        glLinkProgram(ID);

        checkProgramCompileErrors(ID);
    }

    void OpenGLShader::setVec4(const String& uniform, float x, float y, float z, float w) {
        glUniform4f(glGetUniformLocation(ID, uniform.c_str()), x, y, z, w);
    }
    void OpenGLShader::setBool(const String& uniform, bool     x) {
        glUniform1i(glGetUniformLocation(ID, uniform.c_str()), (int)x);
    }
    void OpenGLShader::setInt(const String& uniform, int      x) {
        glUniform1i(glGetUniformLocation(ID, uniform.c_str()), x);
    }
    void OpenGLShader::setUint(const String& uniform, uint      x) {
        glUniform1ui(glGetUniformLocation(ID, uniform.c_str()), x);
    }
    void OpenGLShader::setFloat(const String& uniform, float     x) {
        glUniform1f(glGetUniformLocation(ID, uniform.c_str()), x);
    }
    void OpenGLShader::setVec2(const String& uniform, const glm::vec2& x) {
        glUniform2fv(glGetUniformLocation(ID, uniform.c_str()), 1, &x[0]);
    }
    void OpenGLShader::setVec2(const String& uniform, float x, float    y) {
        glUniform2f(glGetUniformLocation(ID, uniform.c_str()), x, y);
    }
    void OpenGLShader::setVec3(const String& uniform, const glm::vec3& x) {
        glUniform3fv(glGetUniformLocation(ID, uniform.c_str()), 1, &x[0]);
    }
    void OpenGLShader::setVec3(const String& uniform, float x, float y, float z) {
        glUniform3f(glGetUniformLocation(ID, uniform.c_str()), x, y, z);
    }
    void OpenGLShader::setVec4(const String& uniform, const glm::vec4& x) {
        glUniform4fv(glGetUniformLocation(ID, uniform.c_str()), 1, &x[0]);
    }
    void OpenGLShader::setMat2(const String& uniform, const glm::mat2& mat) {
        glUniformMatrix2fv(glGetUniformLocation(ID, uniform.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void OpenGLShader::setMat3(const String& uniform, const glm::mat3& mat) {
        glUniformMatrix3fv(glGetUniformLocation(ID, uniform.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void OpenGLShader::setMat4(const String& uniform, const glm::mat4& mat) {
        glUniformMatrix4fv(glGetUniformLocation(ID, uniform.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    int OpenGLShader::checkProgramCompileErrors(GLuint id) { LOG_FUNCTION();
        GLint success;
        GLchar infoLog[1024];

        glGetProgramiv(id, GL_LINK_STATUS, &success);

        if (success == GL_TRUE)
            return 0;

        glGetProgramInfoLog(id, 1024, nullptr, infoLog);

        LOG( std::string("PROGRAM_LINKING_ERROR: ") + std::string(infoLog), 2);

        return 1;
    }

    int OpenGLShader::checkShaderCompileErrors(GLuint shader, const String& type) { LOG_FUNCTION();
        GLint success;
        GLchar infoLog[1024];

        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (success)
            return 0;

        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);

        LOG( "ERROR::SHADER_COMPILATION_ERROR(" + type + "): " + infoLog, 2);

        return 1;
    }
}
