#include "Mesh.h"
Mesh::Mesh(std::vector<ID3D12ResourcePtr>& vbo, std::vector<uint32_t>& vertCounts, std::vector<ID3D12ResourcePtr>& indexPtrs, std::vector<uint32_t>& indexCounts) 
: mVBOs(vbo), mVertexCounts(vertCounts), mIndices(indexPtrs), mIndicesCounts(indexCounts)
{
}

size_t Mesh::AddInstance(Instance& val)
{
	mInstances.push_back(val);
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

