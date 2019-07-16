#include <map>
#include <string>
#include <memory>
#include "Mesh.h"


using namespace std;
#pragma once
class ResourceManager
{
public:
	static shared_ptr<Mesh> RequestMesh(const string& key);

private:
	static std::map<string, shared_ptr<Mesh>> mMeshDB;

};

