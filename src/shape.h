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

struct Shape
{
	std::vector<std::array<float, 3>>         positions; ///< vbo vertex positions
	std::vector<std::array<float, 3>>         normals;   ///< vertex normals
	std::vector<std::array<uint_fast32_t, 3>> indices;   ///< vbo indices

	GLShape vbos;
};
