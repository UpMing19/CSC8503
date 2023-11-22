/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

using namespace NCL;
using namespace Maths;

Vector2::Vector2(const Vector3& v3) : x(v3.x), y(v3.y){
}

Vector2::Vector2(const Vector4& v4) : x(v4.x), y(v4.y) {
}

constexpr Vector2 Vector2::Clamp(const Vector2& input, const Vector2& mins, const Vector2& maxs) {
	return Vector2(
		std::clamp(input.x, mins.x, maxs.x),
		std::clamp(input.y, mins.y, maxs.y)
	);
}