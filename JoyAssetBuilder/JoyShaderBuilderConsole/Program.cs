﻿using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace ConsoleApplication1
{
    enum ShaderType
    {
        Vertex = 0,
        Fragment = 1
    };

    class Program
    {
        private const StringSplitOptions trimgAndNoEmpty =
            StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries;

        const string dllPath = @"D:\CppProjects\JoyEngine\JoyAssetBuilder\x64\Release\JoyShaderBuilderLib.dll";

        static void ReadInputOrOutput(string body, ref List<string> vars)
        {
            foreach (string varStr in body.Split(';', trimgAndNoEmpty))
            {
                if (string.IsNullOrWhiteSpace(varStr)) continue;
                vars.Add(varStr.Trim(' ', '\r', '\n', '\t'));
            }
        }

        #region Compiler

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern void InitializeCompiler();

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern unsafe int CompileGLSL(string shader, int shdaerSize, ShaderType type, IntPtr* dataPtr,
            UInt64* dataSize, IntPtr* errorMessage);

        static unsafe int CompileGLSL(string shader, ShaderType type, out byte[]? buffer, out string? errorMessage)
        {
            IntPtr outData = IntPtr.Zero;
            UInt64 len;
            IntPtr errorMessagePtr = IntPtr.Zero;
            int result = CompileGLSL(shader, shader.Length, type, &outData, &len, &errorMessagePtr);
            if (result == 0)
            {
                buffer = new byte[len];
                Marshal.Copy(outData, buffer, 0, (int)len);
                errorMessage = null;
            }
            else
            {
                buffer = null;
                errorMessage = Marshal.PtrToStringAnsi(errorMessagePtr);
            }
            return result;
        }

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern void ReleaseInternalData();

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern void ReleaseCompiler();

        #endregion

        #region Test

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern unsafe void GetString(IntPtr* stringPtr);

        static unsafe void GetString()
        {
            IntPtr outString = IntPtr.Zero;
            GetString(&outString);
            System.Console.WriteLine(Marshal.PtrToStringAnsi(outString));
        }

        #endregion

        static void Main(string[] args)
        {
            const string shaderPath = @"D:\CppProjects\JoyEngine\JoyData\shaders\shader.shader";

            const string vertInputAttr = "vert_input";
            const string vertToFragInputAttr = "vert_to_frag";
            const string fragOutputAttr = "frag_output";

            const string vertShaderAttr = "vertex_shader";
            const string fragShaderAttr = "fragment_shader";

            string shader = File.ReadAllText(shaderPath);

            List<string> directives = new List<string>();
            List<string> verInputs = new List<string>();
            List<string> vertToFragInputs = new List<string>();
            List<string> fragOutputs = new List<string>();
            string vertexShader = "";
            string fragmentShader = "";


            for (int i = 0; i < shader.Length; i++)
            {
                if (shader[i] == '#')
                {
                    int posSharp = shader.IndexOf('#', i + 1);
                    int posBracket = shader.IndexOf('[', i + 1);
                    int posDirectiveEnd = -1;
                    if (posSharp == -1 && posBracket == -1)
                    {
                        posDirectiveEnd = shader.Length - 1;
                    }
                    else if (posSharp != -1 && posBracket != -1)
                    {
                        posDirectiveEnd = Math.Min(posSharp, posBracket);
                    }
                    else if (posSharp == -1 && posBracket != -1)
                    {
                        posDirectiveEnd = posBracket;
                    }
                    else if (posSharp != -1 && posBracket == -1)
                    {
                        posDirectiveEnd = posSharp;
                    }

                    directives.Add(shader.Substring(i, posDirectiveEnd - i - 1).Trim('\t', ' ', '\r', '\n'));
                    i = posDirectiveEnd - 1;
                }

                if (shader[i] == '[')
                {
                    int posBracket = shader.IndexOf(']', i + 1);
                    string header = shader.Substring(i + 1, posBracket - i - 1).Trim('\t', ' ', '\r', '\n');
                    int openBrace = shader.IndexOf('{', posBracket);
                    int closeBrace = -1;
                    int count = 1;
                    for (int j = openBrace + 1; j < shader.Length; j++)
                    {
                        if (shader[j] == '}') count--;
                        if (shader[j] == '{') count++;
                        if (count == 0)
                        {
                            closeBrace = j;
                            break;
                        }
                    }

                    string body = shader.Substring(openBrace + 1, closeBrace - openBrace - 1)
                        .Trim('\t', ' ', '\r', '\n');
                    i = posBracket - 1;


                    if (header == vertInputAttr)
                    {
                        ReadInputOrOutput(body, ref verInputs);
                    }

                    if (header == vertToFragInputAttr)
                    {
                        ReadInputOrOutput(body, ref vertToFragInputs);
                    }

                    if (header == fragOutputAttr)
                    {
                        ReadInputOrOutput(body, ref fragOutputs);
                    }

                    if (header == vertShaderAttr)
                    {
                        vertexShader = body;
                    }

                    if (header == fragShaderAttr)
                    {
                        fragmentShader = body;
                    }
                }
            }

            //Console.Write(directives);
            //Console.Write(pushConstant);
            //foreach (var bindingSet in bindingSets)
            //{
            //    Console.WriteLine(bindingSet);
            //}
            //foreach (var s in verInputs)
            //{
            //    Console.WriteLine(s);
            //}
            //foreach (var s in vertToFragInputs)
            //{
            //    Console.WriteLine(s);
            //}
            //foreach (var s in fragOutputs)
            //{
            //    Console.WriteLine(s);
            //}

            //Console.WriteLine(vertexShader);
            //Console.WriteLine(fragmentShader);


            StringBuilder vertexShaderStr = new StringBuilder();
            foreach (var d in directives)
            {
                vertexShaderStr.AppendFormat("{0}\n", d);
            }

            for (int i = 0; i < verInputs.Count; i++)
            {
                vertexShaderStr.AppendFormat("layout(location = {0}) in {1};\n", i, verInputs[i]);
            }

            vertexShaderStr.Append('\n');
            for (int i = 0; i < vertToFragInputs.Count; i++)
            {
                vertexShaderStr.AppendFormat("layout(location = {0}) out {1};\n", i, vertToFragInputs[i]);
            }

            vertexShaderStr.Append('\n');
            vertexShaderStr.Append(vertexShader);

            Console.WriteLine(vertexShaderStr);


            StringBuilder fragmentShaderStr = new StringBuilder();
            foreach (var d in directives)
            {
                fragmentShaderStr.AppendFormat("{0}\n", d);
            }

            for (int i = 0; i < vertToFragInputs.Count; i++)
            {
                fragmentShaderStr.AppendFormat("layout(location = {0}) in {1};\n", i, vertToFragInputs[i]);
            }

            fragmentShaderStr.Append('\n');

            for (int i = 0; i < fragOutputs.Count; i++)
            {
                fragmentShaderStr.AppendFormat("layout(location = {0}) out {1};\n", i, fragOutputs[i]);
            }

            fragmentShaderStr.Append('\n');
            fragmentShaderStr.Append(fragmentShader);

            Console.WriteLine(fragmentShaderStr);

            InitializeCompiler();

            int vResult = CompileGLSL(vertexShaderStr.ToString(), ShaderType.Vertex, out var vertexData,
                out var vErrorMessage);
            ReleaseInternalData();

            int fResult = CompileGLSL(fragmentShaderStr.ToString(), ShaderType.Fragment, out var fragmentData,
                out var fErrorMessage);
            ReleaseInternalData();

            ReleaseCompiler();

            if (vResult != 0)
            {
                Console.WriteLine(vErrorMessage);
            }
            else if (fResult != 0)
            {
                Console.WriteLine(fErrorMessage);
            }
            else
            {
                FileStream fileStream = new FileStream(shaderPath + ".data", FileMode.Create);
                fileStream.Write(BitConverter.GetBytes(vertexData.Length));
                fileStream.Write(BitConverter.GetBytes(fragmentData.Length));
                fileStream.Write(vertexData);
                fileStream.Write(fragmentData);
                fileStream.Close();
            }
        }
    }
}