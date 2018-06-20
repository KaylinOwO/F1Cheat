using System;
using System.CodeDom.Compiler;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Knife;
using Microsoft.CSharp;

namespace KnifeLoader
{

	class KnifeCompiler : IReflectiveLoader
	{
		public override void Load(AppDomain instance, string dllPath)
		{
			throw new NotImplementedException();
		}

		public override void Shutdown()
		{
			throw new NotImplementedException();
		}
	}


	unsafe class KnifeProvider
	{
		static CSharpCodeProvider codeProvider;
		static ICodeCompiler codeCompiler;
		static string compileinfo;


		static int Compile(string s)
		{
			codeCompiler = codeProvider.CreateCompiler();

			CompilerParameters cp = new CompilerParameters();

			// Generate an executable instead of 
			// a class library.
			cp.GenerateExecutable = false;

			// Specify the assembly file name to generate.
			cp.OutputAssembly = "";

			// Save the assembly as a physical file.
			cp.GenerateInMemory = true;

			// Set whether to treat all warnings as errors.
			cp.TreatWarningsAsErrors = false;

			var output = codeCompiler.CompileAssemblyFromSource(cp, s);
			compileinfo = string.Join("\n", output.Output);

			return -1;
		}

		static int FailureReason(string s)
		{

			return (int)Marshal.StringToCoTaskMemUni(compileinfo);
		}
	}
}
