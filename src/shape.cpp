#include "shape.h"

#include <cassert>


float Vec3D::length() const
{
	return sqrt(x*x + y*y + z*z);
}

void Vec3D::normalize()
{
	*this = *this / length();
}

Vec3D operator+(const Vec3D& lhs, const Vec3D& rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Vec3D operator/(const Vec3D& lhs, const Vec3D& rhs)
{
	return { lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z };
}

Vec3D& Vec3D::operator/(float d)
{
	assert(d != 0.0f);

	this->x /= d;
	this->y /= d;
	this->z /= d;

	return *this;
}

Vec3D& Vec3D::operator*=(float m)
{
	this->x *= m;
	this->y *= m;
	this->z *= m;

	return *this;
}

Vec3D& Vec3D::operator*(float m)
{
	return *this *= m;
}


uint_fast32_t Shape::add_normalized_vertex(const Vec3D&& vtx)
{
	const auto idx = static_cast<uint_fast32_t>(positions.size());
	positions.emplace_back(vtx);
	positions.back().normalize();
	return idx;
}

void Shape::scale(float factor)
{
	for (auto& vtx : positions)
	{
		vtx *= factor;
	}
}

void Shape::sub_divide(uint_fast8_t level)
{
	assert(level > 0);

	for (int i = 0; i < level; ++i)
	{
		std::vector<std::array<uint_fast32_t, 3>> new_faces;

		for (const auto& face : indices)
		{
			const auto v1 = positions[face[0]];
			const auto v2 = positions[face[1]];
			const auto v3 = positions[face[2]];

			const auto v1_v2_idx = add_normalized_vertex(std::move((v1 + v2) / 2.0f));
			const auto v2_v3_idx = add_normalized_vertex(std::move((v2 + v3) / 2.0f));
			const auto v3_v1_idx = add_normalized_vertex(std::move((v3 + v1) / 2.0f));

			new_faces.push_back({face[0], v1_v2_idx, v3_v1_idx});
			new_faces.push_back({v1_v2_idx, face[1], v2_v3_idx});
			new_faces.push_back({v3_v1_idx, v2_v3_idx, face[2]});
			new_faces.push_back({v1_v2_idx, v2_v3_idx, v3_v1_idx});
		}

		indices = new_faces;
	}
}
