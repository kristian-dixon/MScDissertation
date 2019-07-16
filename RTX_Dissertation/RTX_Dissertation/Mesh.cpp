#include "Mesh.h"
Mesh::Mesh(ID3D12ResourcePtr vbo) : mVBO(vbo)
{
	//Load BLAS?
}

size_t Mesh::AddInstance(mat4& transform)
{
	mInstances.push_back(transform);
	return mInstances.size() - 1;
}

void Mesh::RemoveInstance(size_t id)
{
	if(id < mInstances.size())
	{
		mInstances.erase(mInstances.begin() + id);
	}
}
