using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;


namespace Knife
{
	abstract public class IKnifeMain : MarshalByRefObject
	{
		public abstract int Main(string arg);

		public abstract int Shutdown();
	}

	abstract class IReflectiveLoader : MarshalByRefObject
	{
		public abstract void Load(AppDomain instance, string dllPath);
		public abstract void Shutdown();
	}
}