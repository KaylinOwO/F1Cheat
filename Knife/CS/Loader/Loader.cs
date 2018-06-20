using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using Knife;


namespace KnifeLoader
{
	class Native
	{
		[DllImport("user32.dll", EntryPoint = "MessageBoxW", SetLastError = true, CharSet = CharSet.Unicode)]
		public static extern int MessageBox(IntPtr hWnd, String text, String caption, int options);

		public static int Msg(string text, string caption)
		{
			return MessageBox(IntPtr.Zero, text, caption, 0);
		}
	}

	class ReflectiveLoader : IReflectiveLoader
	{
		static dynamic knifeProxy;
		public override void Load(AppDomain instance, string dllPath)
		{
			object o = instance.CreateInstanceFromAndUnwrap(dllPath + "Knife.dll", "Knife.KnifeMain");
			knifeProxy = o;
			knifeProxy.Main(dllPath);
		}

		public override void Shutdown()
		{
			knifeProxy.Shutdown();
		}
	}

	class AppLoader
	{
		static string dllPath = "";
		// this stays loaded in the target process and allows for us to hotload the Knife dll as we please
		static AppDomain instance;
		static dynamic reflectiveLoader;
		static bool inited = false;

		static int Load(string arg)
		{
			if(!inited)
			{
				dllPath = arg;

				string lastAction = "";

				try
				{
					lastAction = "CreateDomain";
					instance = AppDomain.CreateDomain("KnifeDomain", null, new AppDomainSetup { ApplicationBase = arg });

					AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;

					object o = instance.CreateInstanceFromAndUnwrap(arg + "Loader.dll", "KnifeLoader.ReflectiveLoader");
					reflectiveLoader = o;
					reflectiveLoader.Load(instance, arg);
					inited = true;
				}
				catch (Exception e)
				{
					Native.Msg(e.Message, lastAction);
				}
			}

			return 0;
		}

		static int Unload(string arg)
		{
			reflectiveLoader.Shutdown();
			AppDomain.Unload(instance);

			inited = false;

			return 0;
		}

		private static Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
		{
			string projectDir = dllPath;
			string shortAssemblyName = args.Name.Substring(0, args.Name.IndexOf(','));
			if (shortAssemblyName == "F12017-Knife-Loader")
			{
				shortAssemblyName = "Loader";
			}
			string fileName = Path.Combine(projectDir, shortAssemblyName + ".dll");
			if (File.Exists(fileName))
			{
				Assembly result = Assembly.LoadFrom(fileName);
				return result;
			}
			else
				return Assembly.GetExecutingAssembly().FullName == args.Name ? Assembly.GetExecutingAssembly() : null;

		}
	}
}
