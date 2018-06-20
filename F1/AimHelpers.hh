#pragma once

#include "../SDK/SDK.hh"

inline void clamp_angle(QAngle &qaAng)
{
	float x = qaAng.x;
	float y = qaAng.y;

	qaAng.x = Clamp(fmodf(x + 90.0f, 180.0f) - 90.0f, -89.0f, 89.0f);

	qaAng.y = Clamp(fmodf(y + 180.f, 360.f) - 180.f, -180.0f, 180.0f);

	qaAng.z = 0.0f;
}

// returns the corrected movement based on the new angles
inline Vector silent_movement_fix(Vector movement_vector, QAngle angles)
{
	Vector out;
	VectorRotate(movement_vector, angles, out);

	return out;

	//float flSpeed = movement_vector.Length2D();
	//QAngle angMove;
	//VectorAngles(movement_vector, angMove);
	//float yaw = (angles.y - movement_vector.y + angMove.y);

	//float c, s;
	//SinCos(yaw, s, c);

	//return {c * flSpeed, s * flSpeed, 0};
}

namespace suvat {
inline Vector calculate_position(float time, Vector startPosition, Vector velocity, Vector accel = {0, 0, 0})
{
	return startPosition + (velocity * time) + (accel * 0.5 * (time * time));
}

inline Vector calculate_addition(float time, Vector velocity, Vector accel = {0, 0, 0})
{
	return (velocity * time) + (accel * 0.5 * (time * time));
}

inline float asymetric_interception_time(Vector position1, Vector position2, float velocity1, float velocity2)
{
	Vector dX = position2 - position1;
	float  dV = velocity1 - velocity2;
	return (dX / dV).Length();
}

inline float asymetric_single_accelerated_interception_time(Vector l, Vector p, float s, float v, Vector a)
{
	/*
        sqrt( (2 * a2 * x1) + (2 * a2 * x2) + v1^2 - (2 * v1 * v2) + v2^2 ) - v1 + v2
    t = ------------------------------------------------------------------------------
        a2
    */
	if (a.x == 0 && a.y == 0 && a.z == 0) {
		return asymetric_interception_time(l, p, s, v);
	}

	return (fsqrtf((a * l * 2).Length() + (a * p * 2).Length() + (s * s) - (s * v * 2) + (v * v)) - s + v) / (a).Length();
}

inline float asymetric_accelerated_interception_time(Vector position1, Vector position2, float velocity1, float velocity2, Vector acceleration1, Vector acceleration2)
{
	/*
        sqrt( (-2 * a1 * x1) + (2 * a1 * x2) + (2 * a2 * x1) - (2 * a2 * x2) + v1^2 - (2 * v1 * v2) + v2^2 ) - v1 + v2
    t = ----------------------------------------------------------------------------------------------------------------
        a1 - a2
    */
	Vector nullvec(0, 0, 0);

	if (acceleration2 == nullvec && acceleration1 != nullvec) {
		return asymetric_single_accelerated_interception_time(position1, position2, velocity1, velocity2, acceleration1);
	} else if (acceleration2 != nullvec && acceleration1 == nullvec) {
		return asymetric_single_accelerated_interception_time(position1, position2, velocity1, velocity2, acceleration2);
	}

	return (sqrt(-(acceleration1 * position1 * 2).Length() + (acceleration1 * position2 * 2).Length() + (acceleration2 * position1 * 2).Length() - (acceleration2 * position2 * 2).Length() + (velocity1 * velocity1) - (velocity1 * velocity2 * 2) + (velocity2 * velocity2)) - velocity1 + velocity2) /
	       (acceleration1 - acceleration2).Length();
}
} // namespace suvat
