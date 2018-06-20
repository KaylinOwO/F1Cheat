using System.Numerics;
using System;

namespace Knife {
    class MathExtension {

		public static double Deg2Rad(double degrees)
		{
			return Math.PI * degrees / 180;
		}

		public static Vector3 ToVector(Vector3 v)
		{
			float sp, sy, cp, cy;

			sy = (float)Math.Sin(Deg2Rad(v.Y));
			cy = (float)Math.Cos(Deg2Rad(v.Y));

			sp = (float)Math.Sin(Deg2Rad(v.X));
			cp = (float)Math.Cos(Deg2Rad(v.X));

			return new Vector3(cp * cy, cp * sy, -sp);
		}
    }
}
