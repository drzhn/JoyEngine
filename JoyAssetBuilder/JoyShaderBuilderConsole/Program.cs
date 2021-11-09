using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace ConsoleApplication1
{

    public enum Qualifier
    {
        Uniform = 0
    }

    public enum VariableType
    {
        Sampler2D = 0,
        Float = 1,
        Vec2 = 2,
        Vec3 = 3,
        Mat4 = 4,
    }
    public struct Binding
    {
        public string attribute;
        public string qualifier;
        public string type;
        public string name;

        public override string ToString()
        {
            return attribute + "\n" + qualifier + " " + type + " " + name;
        }
    }

    public struct BindingSet
    {
        public int isStatic;
        public int index;
        public int bindingsCount;
        public Binding[] bindings;

        public override string ToString()
        {
            string ret = "";
            foreach (var binding in bindings)
            {
                ret += binding;
            }

            return ret;
        }
    }

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

        static void ReadBindingSet(string header, string body, ref HashSet<BindingSet> bindings)
        {
            BindingSet set = new BindingSet();

            foreach (string arg in header.Split(',', trimgAndNoEmpty))
            {
                string[] prms = arg.Split('=', trimgAndNoEmpty);
                if (prms[0] == "set")
                {
                    int.TryParse(prms[1], out set.index);
                }

                if (prms[0] == "static")
                {
                    if (prms[1] == "true")
                    {
                        set.isStatic = 1;
                    }
                    else if (prms[1] == "false")
                    {
                        set.isStatic = 0;
                    }
                    else
                    {
                        throw new Exception();
                    }
                }
            }

            string[] bindingStrArr = body.Split(';', trimgAndNoEmpty);
            set.bindings = new Binding[bindingStrArr.Length];
            set.bindingsCount = bindingStrArr.Length;

            for (var i = 0; i < bindingStrArr.Length; i++)
            {
                string[] bindingDeclaration = bindingStrArr[i].Split(' ', trimgAndNoEmpty);
                // TODO add additional binding attributes such as input_attachment_index 
                set.bindings[i] = new Binding()
                {
                    attribute = "",
                    name = bindingDeclaration[^1],
                    type = bindingDeclaration[^2],
                    qualifier = bindingDeclaration.Length == 3 ? bindingDeclaration[^3] : ""
                };

            }

            bindings.Add(set);
        }

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
        static extern unsafe void CompileGLSL(string s, int len, ShaderType type, IntPtr* dataPtr, UInt64* dataSize);

        static unsafe void CompileGLSL(string shader, ShaderType type, out byte[] buffer)
        {
            IntPtr outData = IntPtr.Zero;
            UInt64 len;
            CompileGLSL(shader, shader.Length, type, &outData, &len);
            buffer = new byte[len];
            Marshal.Copy(outData, buffer, 0, (int)len);
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

            const string pushConstantAttr = "push_constant";
            const string setAttr = "set";

            const string vertInputAttr = "vert_input";
            const string vertToFragInputAttr = "vert_to_frag";
            const string fragOutputAttr = "frag_output";

            const string vertShaderAttr = "vertex_shader";
            const string fragShaderAttr = "fragment_shader";

            string shader = File.ReadAllText(shaderPath);

            List<string> directives = new List<string>();
            string pushConstant = "";
            HashSet<BindingSet> bindingSets = new HashSet<BindingSet>();
            List<string> verInputs = new List<string>();
            List<string> vertToFragInputs = new List<string>();
            List<string> fragOutputs = new List<string>();
            //Dictionary<string, string> functions = new Dictionary<string, string>(); // I'm too lazy to parse all functions and build function tree sorry
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


                    if (header == pushConstantAttr)
                    {
                        pushConstant = body;
                    }

                    if (header.StartsWith(setAttr))
                    {
                        ReadBindingSet(header, body, ref bindingSets);
                    }

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

            if (!string.IsNullOrWhiteSpace(pushConstant))
            {
                vertexShaderStr.AppendFormat("layout(push_constant) {0}\n", pushConstant);
            }

            foreach (BindingSet bindingSet in bindingSets)
            {

                for (int j = 0; j < bindingSet.bindings.Length; j++)
                {
                    vertexShaderStr.AppendFormat("layout(set = {0}, binding = {1}) {2};\n",
                        bindingSet.index,
                        j,
                        bindingSet.bindings[j]);
                }
            }

            BindingSet[] b = bindingSets.ToArray();

            for (int i = 0; i < verInputs.Count; i++)
            {
                vertexShaderStr.AppendFormat("layout(location = {0}) in {1};\n", i, verInputs[i]);
            }

            vertexShader.Append('\n');
            for (int i = 0; i < vertToFragInputs.Count; i++)
            {
                vertexShaderStr.AppendFormat("layout(location = {0}) out {1};\n", i, vertToFragInputs[i]);
            }

            vertexShaderStr.Append('\n');
            vertexShaderStr.Append(vertexShader);

            //Console.WriteLine(vertexShaderStr);

            //InitializeCompiler();
            //CompileGLSL(vertexShaderStr.ToString(), ShaderType.Vertex, out var data);
            //ReleaseInternalData();
            //ReleaseCompiler();
            //File.WriteAllBytes(@"D:\CppProjects\JoyEngine\JoyData\shaders\shader.vert.spv", data);
        }
    }
}