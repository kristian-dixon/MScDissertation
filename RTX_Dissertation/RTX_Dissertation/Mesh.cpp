#include "Mesh.h"
Mesh::Mesh(ID3D12ResourcePtr vbo) : mVBO(vbo)
{
	//Load BLAS?
}

int Mesh::AddInstance(mat4& transform)
{
	mInstances.push_back(transform);
	return mInstances.size() - 1;
}

void Mesh::RemoveInstance(int id)
{
	if(id < mInstances.size())
	{
		mInstances.erase(mInstances.begin() + id);
	}
}
