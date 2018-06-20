using System;
using System.Runtime.InteropServices;
using Knife.Classes;
using Knife.Hooking;
using Knife.Winapi;
using Knife.Structs;

namespace Knife.Hack
{
	class Hack
	{
		public delegate IntPtr InterfaceFactory(string interfaceName, IntPtr result);

		private IntPtr TryInterfaceName(InterfaceFactory factory, string name)
		{
			var last = name[name.Length - 1];
			if (last >= '1' && last <= '9')
			{
				return factory(name, IntPtr.Zero);
			}

			for (var i = 0; i < 64; i++)
			{
				var str = $"{name}0{i}";
				var result = factory(str, IntPtr.Zero);
				if (result != IntPtr.Zero)
				{
					return result;
				}

				str = $"{name}00{i}";
				result = factory(str, IntPtr.Zero);
				if (result != IntPtr.Zero)
				{
					return result;
				}
			}

			return IntPtr.Zero;
		}

		public InterfaceFactory GetInterfaceFactory(string module)
		{
			var modHandle = Native.GetModuleHandleSafe(module);

			if (modHandle == IntPtr.Zero)
			{
				return null;
			}

			var addr = Kernel32.GetProcAddress(modHandle, "CreateInterface");
			return addr != IntPtr.Zero ? Ioc.Get<MarshalCache>().GetDelForPtr<InterfaceFactory>(addr) : null;
		}

		delegate void MessageFn([MarshalAs(UnmanagedType.LPStr)] string msg);

		public void MsgTemplate(string msg, string fnEntryName)
		{
			var modHandle = Native.GetModuleHandleSafe("tier0.dll");

			if (modHandle == IntPtr.Zero)
			{
				return;
			}

			var addr = Kernel32.GetProcAddress(modHandle, fnEntryName);
			if (addr != IntPtr.Zero)
			{
				Ioc.Get<MarshalCache>().GetDelForPtr<MessageFn>(addr)(msg);	
			}
		}

		public void Msg(string msg) => MsgTemplate(msg, "Msg");
		public void Warning(string msg) => MsgTemplate(msg, "Warning");

		public void MsgLine(string msg) => Msg(msg + '\n');
		public void WarningLine(string msg) => Warning(msg + '\n');

		public Tuple<IntPtr, uint> EngineModuleBase => Native.GetModuleBaseByName("engine.dll");
		public Tuple<IntPtr, uint> ClientModuleBase => Native.GetModuleBaseByName("client.dll");

		public unsafe Lazy<InterfaceFactory> AppSystemFactory { get; } = new Lazy<InterfaceFactory>(() => {
			var ptr = **(IntPtr**)Ioc.Get<SignaturePool>()["engine.dll"]["AppSystem"];
			return Ioc.Get<MarshalCache>().GetDelForPtr<InterfaceFactory>(ptr);
		});

		public unsafe GlobalVarsBase* GlobalVariables
			=> (GlobalVarsBase *)Ioc.Get<CInterfaces>().Get()->Globals;

		public Lazy<InterfaceFactory> ClientFactory { get; } = new Lazy<InterfaceFactory>(
			() => Ioc.Get<Hack>().GetInterfaceFactory("client.dll"));

		public Lazy<InterfaceFactory> EngineFactory { get; } = new Lazy<InterfaceFactory>(
			() => Ioc.Get<Hack>().GetInterfaceFactory("engine.dll"));

		public Lazy<InterfaceFactory> VGUIFactory { get; } = new Lazy<InterfaceFactory>(
			() => Ioc.Get<Hack>().GetInterfaceFactory("vguimatsurface.dll"));

		public Lazy<InterfaceFactory> VGUI2Factory { get; } = new Lazy<InterfaceFactory>(
			() => Ioc.Get<Hack>().GetInterfaceFactory("vgui2.dll"));

		public void Init()
		{
			//ClientFactory.Value.With(_ => {
			//	Ioc.Register(new HLClient(TryInterfaceName(_, "VClient")));
			//	Ioc.Register(new EntityList(TryInterfaceName(_, "VClientEntityList")));
			//	Ioc.Register(new ClientMode(IntPtr.Zero));

			//	var @this = Ioc.Get<HLClient>().Self;
			//	var find = @this.GetVMTAndFnForThis(22).SignatureScan("8B0D", 0x100, 2);
			//	if (find != IntPtr.Zero)
			//		unsafe
			//		{
			//			Ioc.Register(new Input(**(IntPtr**)find));
			//		}

			//	find = @this.GetVMTAndFnForThis(10).SignatureScan("8B0D", 0x100, 2);
			//	if (find != IntPtr.Zero)
			//	{
			//		while (Ioc.Get<ClientMode>().Self == IntPtr.Zero)
			//		{
			//			unsafe
			//			{
			//				Ioc.Register(new ClientMode(**(IntPtr**)find));
			//			}
			//		}
			//	}
			//});

			//EngineFactory.Value.With(_ => {
			//	Ioc.Register(new EngineClient(TryInterfaceName(_, "VEngineClient")));
			//	Ioc.Register(new ModelInfo(TryInterfaceName(_, "VModelInfoClient")));
			//});

			//VGUIFactory.Value.With(_ => {

			//});

			//VGUI2Factory.Value.With(_ => {
			//	Ioc.Register(new Surface(TryInterfaceName(_, "VGUI_Surface")));
			//	Ioc.Register(new Panel(TryInterfaceName(_, "VGUI_Panel")));
			//});

			//AppSystemFactory.Value.With(_ => {
			//	Ioc.Register(new EngineTrace(TryInterfaceName(_, "EngineTraceClient")));
			//	Ioc.Register(new StudioRender(TryInterfaceName(_, "VStudioRender")));
			//});


			// register our interfaces
			Ioc.Register(new CInterfaces());

			unsafe
			{
				Ioc.Get<CInterfaces>().With(_ =>
				{
					Ioc.Register(new EngineClient(_.Get()->Engine));
					Ioc.Register(new EngineTrace(_.Get()->EngineTrace));
					Ioc.Register(new EntityList(_.Get()->EntList));
				});
			}

#if DEBUG
			Console.WriteLine(DumpPointers());
#endif
		}
#if DEBUG
		public string DumpPointers() =>
			$"HLClient: 0x{Ioc.Get<HLClient>().Self.ToInt32():X}\n" +
			$"EntityList: 0x{Ioc.Get<EntityList>().Self.ToInt32():X}\n" +
			$"Input: 0x{Ioc.Get<Input>().Self.ToInt32():X}\n" +
			$"ClientMode: 0x{Ioc.Get<ClientMode>().Self.ToInt32():X}\n" +
			$"EngineClient: 0x{Ioc.Get<EngineClient>().Self.ToInt32():X}\n" +
			$"ModelInfo: 0x{Ioc.Get<ModelInfo>().Self.ToInt32():X}\n" +
			$"Panel: 0x{Ioc.Get<Panel>().Self.ToInt32():X}\n" +
			$"Surface: 0x{Ioc.Get<Surface>().Self.ToInt32():X}\n";
#endif
	}
}
