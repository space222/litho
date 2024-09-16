#pragma once

struct vec3
{
	float x,y,z;
	
	vec3 operator*(float scalar) { return vec3{x*scalar,y*scalar,z*scalar}; }
	vec3 operator/(float scalar) { return vec3{x/scalar,y/scalar,z/scalar}; }
	
	vec3 operator*(const vec3& v) { return vec3{ x*v.x, y*v.y, z*v.z }; }
	vec3 operator-(const vec3& v) { return vec3{ x-v.x, y-v.y, z-v.z }; }
	vec3 operator+(const vec3& v) { return vec3{ x+v.x, y+v.y, z+v.z }; }
	float dot(const vec3& v) { return x*v.x + y*v.y + z*v.z; }
} __attribute__((packed));




