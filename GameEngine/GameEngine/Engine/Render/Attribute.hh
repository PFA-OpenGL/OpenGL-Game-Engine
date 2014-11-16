#pragma once

# include <Utils/OpenGL.hh>
# include <stdint.h>
# include <string>
# include <tuple>

enum class Attribute
{
	Position = 0,
	Normal,
	TexCoord,
	Tangent,
	Color,
	Indices,
	END
};

typedef std::tuple<std::string, size_t, size_t, GLenum> AttributeData;

static const AttributeData AttributeProperty[(size_t)Attribute::END] =
{
	std::make_tuple("position", 4, sizeof(float), GL_FLOAT),
	std::make_tuple("normal", 4, sizeof(float), GL_FLOAT),
	std::make_tuple("texCoord", 2, sizeof(float), GL_FLOAT),
	std::make_tuple("tangent", 4, sizeof(float), GL_FLOAT),
	std::make_tuple("color", 4, sizeof(float), GL_FLOAT),
	std::make_tuple("Indices", 1, sizeof(int), GL_UNSIGNED_INT)
};