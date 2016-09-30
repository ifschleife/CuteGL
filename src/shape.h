#pragma once

#include <array>
#include <cinttypes>
#include <vector>

#include <QOpenGLWindow> // for GLuint


struct GLShape
{
	GLuint index;
	GLuint pos;
};

struct Vec3D
{
	float x;
	float y;
	float z;

	float length() const;
	void normalize();

	friend Vec3D operator+(const Vec3D& lhs, const Vec3D& rhs);
	friend Vec3D operator/(const Vec3D& lhs, const Vec3D& rhs);
	Vec3D& operator/(float d);
	Vec3D& operator*=(float m);
	Vec3D& operator*(float m);
};


struct Shape
{
	std::vector<Vec3D>                        positions; ///< vbo vertex positions
	std::vector<std::array<float, 3>>         normals;   ///< vertex normals
	std::vector<std::array<uint_fast32_t, 3>> indices;   ///< vbo indices

	GLShape vbos;

	uint_fast32_t add_normalized_vertex(const Vec3D&& vtx);
	void scale(float factor);
	void sub_divide(uint_fast8_t level);
};
