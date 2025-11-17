#pragma once

#include <vector>
#include "glm/glm.hpp"
#include <string>
#include "shader.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
};

struct Texture
{
	uint32_t id;
	std::string type;
};

class Mesh
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture> textures;
public:
	Mesh(std::vector<Vertex> _vertices, std::vector<uint32_t> _indices, std::vector<Texture> _textures);

	std::vector<Vertex>& Vertices() { return vertices; }
	std::vector<uint32_t>& Indices() { return indices; }
	std::vector<Texture>& Textures() { return textures; }
};