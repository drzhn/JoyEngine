#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <shaderc/shaderc.hpp>
#include <JoyAssetHeaders.h>

//// Returns GLSL shader source text after preprocessing.
//std::string preprocess_shader(const std::string& source_name, shaderc_shader_kind kind, const std::string& source)
//{
//	shaderc::Compiler compiler;
//	shaderc::CompileOptions options;
//
//	// Like -DMY_DEFINE=1
//	options.AddMacroDefinition("MY_DEFINE", "1");
//
//	shaderc::PreprocessedSourceCompilationResult result =
//		compiler.PreprocessGlsl(source, kind, source_name.c_str(), options);
//
//	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
//	{
//		std::cerr << result.GetErrorMessage();
//		return "";
//	}
//
//	return {result.cbegin(), result.cend()};
//}
//
//// Compiles a shader to SPIR-V assembly. Returns the assembly text  as a string.
//std::string compile_file_to_assembly(const std::string& source_name, shaderc_shader_kind kind,
//                                     const std::string& source, bool optimize = false)
//{
//	shaderc::Compiler compiler;
//	shaderc::CompileOptions options;
//
//	// Like -DMY_DEFINE=1
//	options.AddMacroDefinition("MY_DEFINE", "1");
//	if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);
//
//	shaderc::AssemblyCompilationResult result = compiler.CompileGlslToSpvAssembly(
//		source, kind, source_name.c_str(), options);
//
//	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
//	{
//		std::cerr << result.GetErrorMessage();
//		return "";
//	}
//
//	return {result.cbegin(), result.cend()};
//}
//
//// Compiles a shader to a SPIR-V binary. Returns the binary as a vector of 32-bit words.
//std::vector<uint32_t> compile_file(const std::string& source_name,
//                                   shaderc_shader_kind kind,
//                                   const std::string& source,
//                                   bool optimize = false)
//{
//	shaderc::Compiler compiler;
//	shaderc::CompileOptions options;
//
//	// Like -DMY_DEFINE=1
//	options.AddMacroDefinition("MY_DEFINE", "1");
//	if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);
//
//	shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);
//
//	if (module.GetCompilationStatus() != shaderc_compilation_status_success)
//	{
//		std::cerr << module.GetErrorMessage();
//		return std::vector<uint32_t>();
//	}
//
//	return {module.cbegin(), module.cend()};
//}
//
//int main()
//{
//	const char kShaderSource[] =
//		"#version 310 es\n"
//		"void main() { int x = MY_DEFINE; }\n";
//
//	{
//		// Preprocessing
//		auto preprocessed = preprocess_shader(
//			"shader_src", shaderc_glsl_vertex_shader, kShaderSource);
//		std::cout << "Compiled a vertex shader resulting in preprocessed text:"
//			<< std::endl
//			<< preprocessed << std::endl;
//	}
//
//	{
//		// Compiling
//		auto assembly = compile_file_to_assembly(
//			"shader_src", shaderc_glsl_vertex_shader, kShaderSource);
//		std::cout << "SPIR-V assembly:" << std::endl << assembly << std::endl;
//
//		auto spirv =
//			compile_file("shader_src", shaderc_glsl_vertex_shader, kShaderSource);
//		std::cout << "Compiled to a binary module with " << spirv.size()
//			<< " words." << std::endl;
//	}
//
//	{
//		// Compiling with optimizing
//		auto assembly =
//			compile_file_to_assembly("shader_src", shaderc_glsl_vertex_shader,
//			                         kShaderSource, /* optimize = */ true);
//		std::cout << "Optimized SPIR-V assembly:" << std::endl
//			<< assembly << std::endl;
//
//		auto spirv = compile_file("shader_src", shaderc_glsl_vertex_shader,
//		                          kShaderSource, /* optimize = */ true);
//		std::cout << "Compiled to an optimized binary module with " << spirv.size()
//			<< " words." << std::endl;
//	}
//
//	{
//		// Error case
//		const char kBadShaderSource[] =
//			"#version 310 es\nint main() { int main_should_be_void; }\n";
//
//		std::cout << std::endl << "Compiling a bad shader:" << std::endl;
//		compile_file("bad_src", shaderc_glsl_vertex_shader, kBadShaderSource);
//	}
//
//	{
//		// Compile using the C API.
//		std::cout << "\n\nCompiling with the C API" << std::endl;
//
//		// The first example has a compilation problem.  The second does not.
//		const char source[2][80] = {"void main() {}", "#version 450\nvoid main() {}"};
//
//		shaderc_compiler_t compiler = shaderc_compiler_initialize();
//		for (int i = 0; i < 2; ++i)
//		{
//			std::cout << "  Source is:\n---\n" << source[i] << "\n---\n";
//			shaderc_compilation_result_t result = shaderc_compile_into_spv(
//				compiler, source[i], std::strlen(source[i]), shaderc_glsl_vertex_shader,
//				"main.vert", "main", nullptr);
//			auto status = shaderc_result_get_compilation_status(result);
//			std::cout << "  Result code " << int(status) << std::endl;
//			if (status != shaderc_compilation_status_success)
//			{
//				std::cout << "error: " << shaderc_result_get_error_message(result)
//					<< std::endl;
//			}
//			shaderc_result_release(result);
//		}
//		shaderc_compiler_release(compiler);
//	}
//
//	return 0;
//}
//#include <string>
//
//class A
//{
//private:
//	int a = 5;
//};
//
//A* a = nullptr;
//
//extern "C" __declspec(dllexport) void __cdecl Initialize()
//{
//	a = new A();
//}
//


const char* s = "Hello from c++";

extern "C" __declspec(dllexport) void __cdecl GetString(char** string)
{
	*string = const_cast<char*>(s);
}

struct Binding
{
	char* attribute;
	char* qualifier;
	char* type;
	char* name;
};

struct BindingSet
{
	int isStatic;
	int index;
	int bindingsCount;
	Binding* bindings;
};

extern "C" __declspec(dllexport) void __cdecl PrintBindingList(BindingSet * bindingSets, int len)
{
	for (int i =0; i < len; i++)
	{
		std::cout << bindingSets[i].index << " " << bindingSets[i].isStatic << std::endl;
		for (int j = 0; j < bindingSets[i].bindingsCount; j++)
		{
			std::cout << "    " << bindingSets[i].bindings[j].type << " " << bindingSets[i].bindings[j].name << std::endl;
		}
	}
}

shaderc_compiler_t compiler;
shaderc_compilation_result_t result;

enum ShaderType
{
	Vertex=0,
	Fragment=1
};

extern "C" __declspec(dllexport) void __cdecl InitializeCompiler()
{
	compiler = shaderc_compiler_initialize();
}


extern "C" __declspec(dllexport) void __cdecl CompileGLSL(
	char* s,
	int len,
	ShaderType type,
	const char** dataPtr,
	unsigned long long* dataSize)
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
		return;
	}
	result = shaderc_compile_into_spv(
		compiler,
		s,
		len,
		kind,
		"",
		"main",
		nullptr);
	shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
	if (status != shaderc_compilation_status_success)
	{
		std::cout << "error: " << shaderc_result_get_error_message(result) << std::endl;
	}
	else
	{
		std::cout << "EVERYTHING IS ALRIGHT" << std::endl;
	}
	*dataPtr = shaderc_result_get_bytes(result);
	*dataSize = shaderc_result_get_length(result);
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
