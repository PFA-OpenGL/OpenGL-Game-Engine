#pragma once

#include <glm/glm.hpp>

namespace AGE
{
	class Plane
	{
	public:
		Plane(const glm::vec3 &normal = glm::vec3(0,0,0), float distance = 0.0f)
		{}

		void normalize()
		{
			float divider = 1 / glm::length(_normal);
			_normal = _normal * divider;
			_distance = _distance * divider;
		}

		const glm::vec3 &getNormal() const { return _normal; }
		float getDistance() const { return _distance; }
		void setNormal(const glm::vec3 normal) { _normal = normal; }
		void setDistance(float distance) { _distance = distance; }
		void set(float x, float y, float z, float w) { _normal = glm::vec3(x, y, z); _distance = w; }
		float dot(const glm::vec3 &point) const
		{
			return glm::dot(_normal, point) + _distance;
		}
	private:
		glm::vec3 _normal;
		float _distance;
	};
}