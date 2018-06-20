using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Knife
{

	unsafe struct Interfaces
	{
		// Client
		public IntPtr Client;
		public IntPtr EntList;
		public IntPtr Prediction;
		public IntPtr GameMovement;
		public IntPtr ClientMode;
		//IMoveHelper *> MoveHelper;


		// EngineClient 			*Engine;
		public IntPtr Engine;
		public IntPtr ModelInfo;
		public IntPtr ModelRender;
		public IntPtr RenderView;
		public IntPtr EngineTrace;
		public IntPtr RandomStream;
		public IntPtr EventManager;
		public IntPtr DebugOverlay;
		public IntPtr SoundEngine;
		public IntPtr DemoPlayer;
		public IntPtr ClientState;

		public IntPtr Surface;

		public IntPtr Panels;

		public IntPtr Cvar;

		public IntPtr Input;

		public IntPtr Globals;

		public IntPtr PhysicsSurfaceProps;

		public IntPtr MatSystem;

		public IntPtr EngineVGUI;

		public IntPtr ScreenSpaceEffectManager;

		//CSteamInterfaces steam;

		//class ILagCompensationManager LagCompensation;

		public IntPtr thisWindow;
		public IntPtr oldWindowProc;
		public IntPtr thisDll;
	}

	unsafe class CInterfaces
	{
		[DllImport("F12017-premake.dll")]
		public extern static Interfaces* Knife_GetInterfaces();

		public Interfaces *Get()
		{
			return Knife_GetInterfaces();
		}
	}
}
