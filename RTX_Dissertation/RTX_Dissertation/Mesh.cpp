#include "Mesh.h"
Mesh::Mesh(std::vector<ID3D12ResourcePtr>& vbo, std::vector<uint32_t>& vertCounts) : mVBOs(vbo), mVertexCounts(vertCounts)
{
	//Load BLAS?
}

size_t Mesh::AddInstance(mat4& transform)
{
	mInstances.push_back(transform);
	mInstanceCount++;
	return mInstances.size() - 1;
}

void Mesh::RemoveInstance(size_t id)
{
	if(id < mInstances.size())
	{
		mInstances.erase(mInstances.begin() + id);
		mInstanceCount--;
	}
}
