using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace ConsoleApplication1
{
    public struct Binding
    {
        public string attribute;
        public string binding;

        public override string ToString()
        {
            return attribute + "\n" + binding + "\n";
        }
    }

    public struct BindingSet
    {
        public bool isStatic;
        public List<Binding> bindings;

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

    public enum ReadingState
    {
        None,
        Header,
        PushConstant,
        Bindings,
        VertInput,
        VertToFrag,
        FragOutput,
        Function
    };

    class Program
    {
        const string dllPath =
            @"D:\CppProjects\JoyEngine\JoyAssetBuilder\JoyAssetBuilder\x64\Release\JoyShaderBuilderLib.dll";

        static readonly HashSet<string> namesToReplace = new HashSet<string>() { "vert", "frag" };

        static void ReadHeader(int currentPos, string shader, out string header, out int headerEndPos)
        {
            headerEndPos = shader.IndexOf('\n', currentPos);
            header = shader.Substring(currentPos, headerEndPos - currentPos).Trim('	', ' ', '\r', '\n');
        }

        static void ReadPushConstant(int currentPos, string shader, out string pushConstant, out int headerEndPos)
        {
            int openBracesPos = shader.IndexOf('{', currentPos);
            int closeBracesPos = shader.IndexOf('}', openBracesPos);
            int semicolonPos = shader.IndexOf(';', closeBracesPos);

            headerEndPos = semicolonPos;
            pushConstant = shader
                .Substring(currentPos, semicolonPos - currentPos + 1)
                .Trim('	', ' ', '\r', '\n');
        }

        static void ReadBindingSet(int currentPos, string shader, ref List<BindingSet> bindings, out int bindingsEndPos)
        {
            BindingSet set = new BindingSet();
            set.bindings = new List<Binding>();

            int equalPos = shader.IndexOf('=', currentPos);
            int commaPos = shader.IndexOf(',', equalPos);
            string setIndexStr = shader
                .Substring(equalPos + 1, commaPos - equalPos - 1)
                .ReplaceLineEndings()
                .Trim(' ');
            int setIndex = -1;
            int.TryParse(setIndexStr, out setIndex);

            equalPos = shader.IndexOf('=', commaPos);
            int closedBracketPos = shader.IndexOf(']', equalPos);
            string isStaticStr = shader
                .Substring(equalPos + 1, closedBracketPos - equalPos - 1)
                .ReplaceLineEndings()
                .Trim(' ');
            if (isStaticStr == "true")
            {
                set.isStatic = true;
            }
            else if (isStaticStr == "false")
            {
                set.isStatic = false;
            }
            else
            {
                throw new Exception();
            }

            int openBrackedPos = shader.IndexOf('{', closedBracketPos);
            closedBracketPos = shader.IndexOf('}', openBrackedPos);
            foreach (string bindingStr in shader
                .Substring(openBrackedPos + 1, closedBracketPos - openBrackedPos - 1)
                .Split(';'))
            {
                if (string.IsNullOrWhiteSpace(bindingStr)) continue;
                set.bindings.Add(new Binding() { attribute = "", binding = bindingStr.Trim('	', ' ', '\r', '\n') });
            }

            bindings.Add(set);

            bindingsEndPos = closedBracketPos;
        }

        static void ReadInputOrOutput(int currentPos, string shader, ref List<string> vars, out int endPos)
        {
            int openBrackedPos = shader.IndexOf('{', currentPos);
            int closedBracketPos = shader.IndexOf('}', openBrackedPos);
            foreach (string varStr in shader
                .Substring(openBrackedPos + 1, closedBracketPos - openBrackedPos - 1)
                .Split(';'))
            {
                if (string.IsNullOrWhiteSpace(varStr)) continue;
                vars.Add(varStr.Trim('	', ' ', '\r', '\n'));
            }

            endPos = closedBracketPos;
        }

        static void ReadFunction(int currentPos, string shader, ref Dictionary<string, string> functions,
            out int endPos)
        {
            int closedBracketPos = shader.IndexOf('}', currentPos);
            string name = shader
                .Substring(currentPos, shader.IndexOf('(', currentPos) - currentPos)
                .Split(' ', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries)[1];
            string function = shader.Substring(currentPos, closedBracketPos - currentPos + 1);
            functions[name] = function.Trim('	', ' ', '\r', '\n');

            if (namesToReplace.Contains(name)) // glsl only allows entry point to be called "main"
            {
                int place = functions[name].IndexOf(name);
                functions[name] = functions[name].Remove(place, name.Length).Insert(place, "main");
            }

            endPos = closedBracketPos;
        }

        static bool CheckAttribute(string attr, ref string shader, ref int pos)
        {
            return pos + attr.Length < shader.Length && shader.Substring(pos, attr.Length) == attr;
        }

        static bool CheckSymbolName(string s)
        {
            if (!char.IsLetter(s[0])) return false;
            if (s.Length == 1) return true;
            for (int i = 1; i < s.Length; i++)
            {
                if (!char.IsLetterOrDigit(s[0])) return false;
            }

            return true;
        }

        static bool CheckVarDeclaration(string s)
        {
            var entities = s.Split(' ', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
            if (entities.Length != 2) return false;
            if (!CheckSymbolName(entities[0]) || !CheckSymbolName(entities[1])) return false;
            return true;
        }

        static bool CheckFunction(ref string shader, ref int pos)
        {
            int openedBrackedPos = shader.IndexOf('{', pos + 5); // T n(){ 
            if (openedBrackedPos < 0) return false;
            string functionHeader = shader.Substring(pos, openedBrackedPos - pos - 1);
            if (functionHeader.IndexOf(')') < 0 || functionHeader.IndexOf('(') < 0) return false;

            string[] nameAndArgs = functionHeader.Split(new char[] { '(', ')' },
                StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
            if (nameAndArgs.Length > 2 || nameAndArgs.Length < 1) return false;
            if (!CheckVarDeclaration(nameAndArgs[0])) return false;
            if (nameAndArgs.Length == 2)
            {
                string[] args = nameAndArgs[1].Split(',',
                    StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
                foreach (var arg in args)
                {
                    if (!CheckVarDeclaration(arg)) return false;
                }
            }

            return true;
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

        static void Main(string[] args)
        {
            const string shaderPath = @"D:\CppProjects\JoyEngine\JoyData\shaders\shader.shader";
            const string pushConstantAttr = "[push_constant]";
            const string setAttr = "[set";
            const string vertInputAttr = "[vert_input]";
            const string vertToFragInputAttr = "[vert_to_frag]";
            const string fragOutputAttr = "[frag_output]";

            string shader = File.ReadAllText(shaderPath);

            StringBuilder headers = new StringBuilder();
            string pushConstant = "";
            List<BindingSet> bindingSets = new List<BindingSet>();
            List<string> verInputs = new List<string>();
            List<string> vertToFragInputs = new List<string>();
            List<string> fragOutputs = new List<string>();
            Dictionary<string, string> functions = new Dictionary<string, string>();

            ReadingState state = ReadingState.None;

            for (int i = 0; i < shader.Length; i++)
            {
                if (state == ReadingState.None)
                {
                    if (shader[i] == '#')
                    {
                        state = ReadingState.Header;
                    }
                    else if (CheckAttribute(pushConstantAttr, ref shader, ref i))
                    {
                        state = ReadingState.PushConstant;
                    }
                    else if (CheckAttribute(setAttr, ref shader, ref i))
                    {
                        state = ReadingState.Bindings;
                    }
                    else if (CheckAttribute(vertInputAttr, ref shader, ref i))
                    {
                        state = ReadingState.VertInput;
                    }
                    else if (CheckAttribute(vertToFragInputAttr, ref shader, ref i))
                    {
                        state = ReadingState.VertToFrag;
                    }
                    else if (CheckAttribute(fragOutputAttr, ref shader, ref i))
                    {
                        state = ReadingState.FragOutput;
                    }
                    else if (CheckFunction(ref shader, ref i))
                    {
                        state = ReadingState.Function;
                    }
                }

                switch (state)
                {
                    case ReadingState.Header:
                    {
                        ReadHeader(i, shader, out var header, out i);
                        headers.Append(header + "\n");
                        state = ReadingState.None;
                        break;
                    }
                    case ReadingState.PushConstant:
                    {
                        ReadPushConstant(i + pushConstantAttr.Length, shader, out pushConstant, out i);
                        state = ReadingState.None;
                        break;
                    }
                    case ReadingState.Bindings:
                    {
                        ReadBindingSet(i, shader, ref bindingSets, out i);
                        state = ReadingState.None;
                        break;
                    }
                    case ReadingState.VertInput:
                    {
                        ReadInputOrOutput(i, shader, ref verInputs, out i);
                        state = ReadingState.None;
                        break;
                    }
                    case ReadingState.VertToFrag:
                    {
                        ReadInputOrOutput(i, shader, ref vertToFragInputs, out i);
                        state = ReadingState.None;
                        break;
                    }
                    case ReadingState.FragOutput:
                    {
                        ReadInputOrOutput(i, shader, ref fragOutputs, out i);
                        state = ReadingState.None;
                        break;
                    }
                    case ReadingState.Function:
                    {
                        ReadFunction(i, shader, ref functions, out i);
                        state = ReadingState.None;
                        break;
                    }
                }
            }

            //Console.Write(headers);
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

            //foreach (KeyValuePair<string, string> pair in functions)
            //{
            //    Console.WriteLine(pair.Key);
            //    Console.WriteLine(pair.Value);
            //}

            //Console.WriteLine(functions["vert"]);
            //Console.WriteLine(functions["frag"]);


            StringBuilder vertexShader = headers;
            vertexShader.AppendFormat("layout(push_constant) {0}\n", pushConstant);
            for (int i = 0; i < bindingSets.Count; i++)
            {
                for (int j = 0; j < bindingSets[i].bindings.Count; j++)
                {
                    vertexShader.AppendFormat("layout(set = {0}, binding = {1}) {2};\n",
                        i,
                        j,
                        bindingSets[i].bindings[j]);
                }
            }

            for (int i = 0; i < verInputs.Count; i++)
            {
                vertexShader.AppendFormat("layout(location = {0}) in {1};\n", i, verInputs[i]);
            }

            vertexShader.Append('\n');
            for (int i = 0; i < vertToFragInputs.Count; i++)
            {
                vertexShader.AppendFormat("layout(location = {0}) out {1};\n", i, vertToFragInputs[i]);
            }

            vertexShader.Append('\n');
            vertexShader.Append(functions["vert"]);

            Console.WriteLine(vertexShader);

            InitializeCompiler();
            CompileGLSL(vertexShader.ToString(), ShaderType.Vertex, out var data);
            ReleaseInternalData();
            ReleaseCompiler();
            File.WriteAllBytes(@"D:\CppProjects\JoyEngine\JoyData\shaders\shader.vert.spv", data);
        }
    }
}