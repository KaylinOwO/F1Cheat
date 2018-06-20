using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using Knife.Hack;

namespace Knife
{

	class KnifeMain : IKnifeMain
	{
		public override int Main(string arg)
		{
			// this function is called when KnifeMain() is called from C++ code
			// we should probably launch a thread and run init code from here
			new Thread(() => {
#if DEBUG
                Kernel32.AllocConsole();
#endif
				Ioc.Register<SignaturePool>();
				Ioc.Get<Hack.Hack>().Init();
				Ioc.Get<Hook>().Init();
			}).Start();
			return 0;
		}

		public override int Shutdown()
		{
			return 0;
		}
	}
}
