using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Knife.Classes;
using Knife.Hack.Hooking_Classes;
using Knife.Hack.Hooking_Classes.ClientModeCreateMove;
using Knife.Hack.Hooking_Classes.PaintTraverse;
using Knife.Hooking;
using Knife.Structs;

namespace Knife.Hack
{
	class NativeAttribute : Attribute
	{

	}

	unsafe class Hook
	{

		public enum EventCallbackType
		{
			init,
			paint,
			beforePred,
			afterPred,
			keyevent,
			entity,
		}

		[DllImport("F12017-premake.dll")]
		public extern static void Knife_SubscribeToEvent(EventCallbackType t, IntPtr fn);

		public static void SubscribeToEvent(EventCallbackType t, Delegate fn)
		{
			Knife_SubscribeToEvent(t ,Ioc.Get<MarshalCache>().GetPtrForDel(fn));
		}

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		public delegate void ProcessCommandFn(UserCmd* cmd);
		
		List<IClientModeCreateMove> m_CreateMoveHooks = new List<IClientModeCreateMove> {
			new SniperTriggerbot(),
            //new JumpAttack()
        };

		List<IPaintTraverse> m_PaintTraverseHooks = new List<IPaintTraverse> {
			new PlayerGlow(),
			new SpyUndisguise(),
		};

		void OnProcessCommand(UserCmd* cmd)
		{
			Ioc.Get<Hack>().MsgLine("OnProcessCommand");
			Ioc.Get<Hack>().MsgLine(Ioc.Get<EngineClient>().ViewAngles.ToString());
			foreach (var hook in m_CreateMoveHooks)
				hook.With(_ =>
				{
					bool @out = false;
					if (_.Condition())
						_.OnCreateMove(0, cmd, ref @out);
				});
		}

		public void Init()
		{
			SubscribeToEvent(EventCallbackType.afterPred, (ProcessCommandFn)OnProcessCommand);
		}
	}
}
