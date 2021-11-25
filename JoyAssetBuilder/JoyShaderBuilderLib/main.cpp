#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <shaderc/shaderc.hpp>
#include <JoyAssetHeaders.h>

const char* s = "Hello from c++";

extern "C" __declspec(dllexport) void __cdecl GetString(char** string)
{
	*string = const_cast<char*>(s);
}

shaderc_compiler_t compiler;
shaderc_compilation_result_t result;

enum ShaderType
{
	Vertex = 1 << 0,
	Fragment = 1 << 1
};

extern "C" __declspec(dllexport) void __cdecl InitializeCompiler()
{
	compiler = shaderc_compiler_initialize();
}

extern "C" __declspec(dllexport) int __cdecl CompileGLSL(
	char* shaderText,
	int shaderTextSize,
	ShaderType type,
	const char** dataPtr,
	unsigned long long* dataSize,
	const char** errorMessage)
{
	shaderc_shader_kind kind;
	switch (type)
	{
	case Vertex:
		kind = shaderc_vertex_shader;
		break;
	case Fragment:
		kind = shaderc_fragment_shader;
		break;
	default:
		return shaderc_compilation_status_configuration_error;
	}
	result = shaderc_compile_into_spv(
		compiler,
		shaderText,
		shaderTextSize,
		kind,
		"",
		"main",
		nullptr);
	shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
	if (status != shaderc_compilation_status_success)
	{
		//std::cout << "error: " << shaderc_result_get_error_message(result) << std::endl;
		*errorMessage = shaderc_result_get_error_message(result);
	}
	else
	{
		//std::cout << "EVERYTHING IS ALRIGHT" << std::endl;
		*dataPtr = shaderc_result_get_bytes(result);
		*dataSize = shaderc_result_get_length(result);
	}
	return status;
}

extern "C" __declspec(dllexport) void __cdecl ReleaseInternalData()
{
	if (result != nullptr)
	{
		shaderc_result_release(result);
	}
}


extern "C" __declspec(dllexport) void __cdecl ReleaseCompiler()
{
	if (compiler != nullptr)
	{
		shaderc_compiler_release(compiler);
	}
}
