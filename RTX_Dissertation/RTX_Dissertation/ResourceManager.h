#pragma once
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

	//Used to add new meshes to the mesh DB since hardcoding them into RequestMesh is dumb. 
	static shared_ptr<Mesh> AddNewMesh(const string& key, const vector<glm::vec3> verts);

	//Used to add new meshes to the mesh DB since hardcoding them into RequestMesh is dumb. 
	static shared_ptr<Mesh> AddNewMesh(const string& key, ID3D12ResourcePtr vbo);

	static const std::map<string, shared_ptr<Mesh>>& GetMeshDB() {return mMeshDB; };

	static void ClearResources() { mMeshDB.clear(); }

	static shared_ptr<HitProgram> RequestHitProgram(const string& key);
		
	static void AddHitProgram(const string& key, shared_ptr<HitProgram> hitProgram);
	static void AddMissProgram(const string& key, shared_ptr<MissProgram> missProgram);

	static std::map<string, shared_ptr<HitProgram>> GetHitProgramDB() { return mHitProgramDB; };
	static std::map<string, shared_ptr<MissProgram>> GetMissProgramDB() { return mMissProgramDB; };


private:
	static std::map<string, shared_ptr<Mesh>> mMeshDB;
	static std::map<string, shared_ptr<HitProgram>> mHitProgramDB;
	static std::map<string, shared_ptr<MissProgram>> mMissProgramDB;

};

