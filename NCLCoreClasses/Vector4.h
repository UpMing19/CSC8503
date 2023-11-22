/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

namespace NCL::Maths {
	class Vector3;
	class Vector2;

	class Vector4 {

	public:
		float x;
		float y;
		float z;
		float w;
	public:
		constexpr Vector4(void) : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

		constexpr Vector4(float xVal, float yVal, float zVal, float wVal) : x(xVal), y(yVal), z(zVal), w(wVal) {}

		Vector4(const Vector3& v3, float w = 0.0f);
		Vector4(const Vector2& v2, float z = 0.0f, float w = 0.0f);

		~Vector4(void) = default;

		Vector4 Normalised() const {
			Vector4 temp(*this);
			temp.Normalise();
			return temp;
		}

		void			Normalise() {
			float length = Length();

			if (length != 0.0f) {
				length = 1.0f / length;
				x = x * length;
				y = y * length;
				z = z * length;
				w = w * length;
			}
		}

		float	Length() const {
			return std::sqrt(LengthSquared());
		}

		constexpr float	LengthSquared() const {
			return ((x*x) + (y*y) + (z*z) + (w * w));
		}

		constexpr float		GetMinElement() const {
			float v = x;
			v = std::min(v, y);
			v = std::min(v, z);
			v = std::min(v, w);
			return v;
		}

		constexpr float		GetMaxElement() const {
			float v = x;
			v = std::max(v, y);
			v = std::max(v, z);
			v = std::max(v, w);
			return v;
		}

		float		GetAbsMaxElement() const {
			float v = std::abs(x);
			v = std::max(v, std::abs(y));
			v = std::max(v, std::abs(z));
			v = std::max(v, std::abs(w));
			return v;
		}

		static constexpr Vector4 Clamp(const Vector4& input, const Vector4& mins, const Vector4& maxs);

		static constexpr float	Dot(const Vector4 &a, const Vector4 &b) {
			return (a.x*b.x) + (a.y*b.y) + (a.z*b.z) + (a.w*b.w);
		}

		inline Vector4  operator+(const Vector4  &a) const {
			return Vector4(x + a.x, y + a.y, z + a.z, w + a.w);
		}

		inline Vector4  operator-(const Vector4  &a) const {
			return Vector4(x - a.x, y - a.y, z - a.z, w - a.w);
		}

		inline Vector4  operator-() const {
			return Vector4(-x, -y, -z, -w);
		}

		inline Vector4  operator*(float a)	const {
			return Vector4(x * a, y * a, z * a, w * a);
		}

		inline Vector4  operator*(const Vector4  &a) const {
			return Vector4(x * a.x, y * a.y, z * a.z, w * a.w);
		}

		inline Vector4  operator/(const Vector4  &a) const {
			return Vector4(x / a.x, y / a.y, z / a.z, w / a.w);
		};

		inline Vector4  operator/(float v) const {
			return Vector4(x / v, y / v, z / v, w / v);
		};

		inline constexpr void operator+=(const Vector4  &a) {
			x += a.x;
			y += a.y;
			z += a.z;
			w += a.w;
		}

		inline void operator-=(const Vector4  &a) {
			x -= a.x;
			y -= a.y;
			z -= a.z;
			w -= a.w;
		}

		inline void operator*=(const Vector4  &a) {
			x *= a.x;
			y *= a.y;
			z *= a.z;
			w *= a.w;
		}

		inline void operator/=(const Vector4  &a) {
			x /= a.x;
			y /= a.y;
			z /= a.z;
			w /= a.w;
		}

		inline void operator*=(float f) {
			x *= f;
			y *= f;
			z *= f;
			w *= f;
		}

		inline void operator/=(float f) {
			x /= f;
			y /= f;
			z /= f;
			w /= f;
		}

		inline float operator[](int i) const {
			return ((float*)this)[i];
		}

		inline float& operator[](int i) {
			return ((float*)this)[i];
		}

		inline bool	operator==(const Vector4 &A)const { return (A.x == x && A.y == y && A.z == z && A.w == w) ? true : false; };
		inline bool	operator!=(const Vector4 &A)const { return (A.x == x && A.y == y && A.z == z && A.w == w) ? false : true; };

		inline friend std::ostream& operator<<(std::ostream& o, const Vector4& v) {
			o << "Vector4(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")\n";
			return o;
		}
	};
}