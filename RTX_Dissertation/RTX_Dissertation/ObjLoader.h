#include "Mesh.h"

#pragma once
class ObjLoader
{
public:
	static std::shared_ptr<Mesh> LoadOBJMesh(std::string fileName);
	static glm::vec3 GetVectorFromString(std::string& input, std::string::size_type offset);
};




