#include "RaytracingPipelineState.h"


RaytracingPipelineState::RaytracingPipelineState(wstring& shaderFileName) : 
	mShaderFileName(shaderFileName), 
	mShaderConfig(sizeof(float) * 2, sizeof(float) * 3),
	mPipelineConfig(2)
{
}

void RaytracingPipelineState::AddHitProgram(shared_ptr<HitProgram> hitProgram)
{
	if(hitProgram->localRootSignature == nullptr)
	{
		//Add to empty list
		mEmptyHitPrograms.push_back(hitProgram);
	}
	else
	{
		//Add to list that'll mount the root signature later
		mHitPrograms.push_back(hitProgram);
	}
}

void RaytracingPipelineState::AddMissProgram(shared_ptr<MissProgram> missProgram)
{
	if (missProgram->localRootSignature == nullptr)
	{
		//Add to empty list
		mEmptyMissPrograms.push_back(missProgram);
	}
	else
	{
		//Add to list that'll mount the root signature later
		mMissPrograms.push_back(missProgram);
	}
}

void RaytracingPipelineState::GetEntryPoints(vector<const WCHAR*>& entryPoints)
{
	entryPoints.push_back(L"rayGen");

	for(int i = 0; i < mMissPrograms.size(); ++i)
	{
		entryPoints.push_back(mMissPrograms[i]->missShader);
	}

	for (int i = 0; i < mEmptyMissPrograms.size(); ++i)
	{
		entryPoints.push_back(mEmptyMissPrograms[i]->missShader);
	}

	for (int i = 0; i < mHitPrograms.size(); ++i)
	{
		if(mHitPrograms[i]->desc.AnyHitShaderImport != nullptr)
		{
			entryPoints.push_back(mHitPrograms[i]->exportStr);
		}

		if (mHitPrograms[i]->desc.ClosestHitShaderImport != nullptr)
		{
			entryPoints.push_back(mHitPrograms[i]->exportStr);
		}

		if (mHitPrograms[i]->desc.IntersectionShaderImport != nullptr)
		{
			entryPoints.push_back(mHitPrograms[i]->exportStr);
		}
	}

	for (int i = 0; i < mEmptyHitPrograms.size(); ++i)
	{
		if (mEmptyHitPrograms[i]->desc.AnyHitShaderImport != nullptr)
		{
			entryPoints.push_back(mEmptyHitPrograms[i]->exportStr);
		}

		if (mEmptyHitPrograms[i]->desc.ClosestHitShaderImport != nullptr)
		{
			entryPoints.push_back(mEmptyHitPrograms[i]->exportStr);
		}

		if (mEmptyHitPrograms[i]->desc.IntersectionShaderImport != nullptr)
		{
			entryPoints.push_back(mEmptyHitPrograms[i]->exportStr);
		}
	}
}	

void RaytracingPipelineState::BuildPipeline(HWND winHandle, ID3D12Device5Ptr device)
{
	const WCHAR* kRayGenShader = L"rayGen";

	//1 for loading shader
	//1 per hit program
	//2 for ray gen
	//2 per object with root signature
	//2 for all empties
	//2 for shader config
	//1 for pipeline config
	//1 for global root signature

	// 6 + (hit groups total) + (hitPrograms with root signatures * 2) + (miss programs with root signatures * 2)

	const int subobjectCount = 9 + (mHitPrograms.size() + mEmptyHitPrograms.size()) + (mHitPrograms.size() * 2) + (mMissPrograms.size() * 2);


	std::vector<D3D12_STATE_SUBOBJECT> subobjects(subobjectCount);
	uint32_t index = 0;

	vector<const WCHAR*> shaderEntryPoints;
	GetEntryPoints(shaderEntryPoints);



	// Create the DXIL library
	DxilLibrary dxilLib = RendererUtil::CreateDxilLibrary(winHandle, mShaderFileName, shaderEntryPoints);
	subobjects[index++] = dxilLib.stateSubobject; // 0 Library


	//Bind hit groups
	for(auto& hitProgram : mHitPrograms)
	{
		subobjects[index++] = hitProgram->subObject;
	}

	for (auto& hitProgram : mEmptyHitPrograms)
	{
		subobjects[index++] = hitProgram->subObject;
	}


	//Create Ray-Gen Root-Signature and Association
	

	// Create the ray-gen root-signature and association
	LocalRootSignature rgsRootSignature(winHandle, device, RendererUtil::CreateRayGenRootDesc().desc);
	subobjects[index] = rgsRootSignature.subobject; // RayGen Root Sig

	uint32_t rgsRootIndex = index++; // 3
	ExportAssociation rgsRootAssociation(&kRayGenShader, &(subobjects[rgsRootIndex]));
	subobjects[index++] = rgsRootAssociation.subobject; //Associate Root Sig to RGS


	vector<LocalRootSignature> hitRootSignatures(mHitPrograms.size());
	vector<ExportAssociation> exportAssociations(mHitPrograms.size());
	int counter = 0;


	//Create hit programs and their associations // TODO:: Look up what we need to do about AHS 
	for(auto& hitProgram : mHitPrograms)
	{
		hitRootSignatures[counter] = *hitProgram->localRootSignature;
		subobjects[index] = hitRootSignatures[counter].subobject; // 5 Triangle Hit Root Sig

		uint32_t hitRootIndex = index++; // 5
		
		ExportAssociation association(&hitProgram->exportStr, &(subobjects[hitRootIndex]));

		exportAssociations[counter].association = association.association;


		subobjects[index++] = exportAssociations[counter].subobject; // 6 Associate Triangle Root Sig to Triangle Hit Group

		counter++;
	}


	

	//Create things that'll use the empty-root signature and the association object

	D3D12_ROOT_SIGNATURE_DESC emptyDesc = {};
	emptyDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	LocalRootSignature emptyRootSignature(winHandle, device, emptyDesc);
	subobjects[index] = emptyRootSignature.subobject; // 7 Root Sig to be shared between Miss and CHS

	uint32_t hitMissRootIndex = index++; // 8
	
	vector<const WCHAR*> missHitExportName;
	for(auto& t : mEmptyHitPrograms)
	{
		missHitExportName.push_back(t->exportStr);
	}

	for (auto& t : mEmptyMissPrograms)
	{
		missHitExportName.push_back(t->missShader);
	}
	
	ExportAssociation missHitRootAssociation(missHitExportName.data(), static_cast<int>(missHitExportName.size()), &(emptyRootSignature.subobject));
	subobjects[index++] = missHitRootAssociation.subobject; // 9 Associate Root Sig to Miss and CHS

	
	
	
	// Bind the payload size to the programs
	ShaderConfig shaderConfig(sizeof(float) * 2, sizeof(float) * 3);
	subobjects[index] = shaderConfig.subobject; // 10 Shader Config



	


	uint32_t shaderConfigIndex = index++; // 10
	//const WCHAR* shaderExports[] = { kMissShader, kClosestHitShader, kRayGenShader, kShadowChs, kShadowMiss };
	ExportAssociation configAssociation(shaderEntryPoints.data(), static_cast<int>(shaderEntryPoints.size()), &(subobjects[shaderConfigIndex]));
	subobjects[index++] = configAssociation.subobject; //11 Associate Shader Config to Miss, CHS, RGS

	// Create the pipeline config
	PipelineConfig config(30);
	subobjects[index++] = config.subobject; //12

	// Create the global root signature and store the empty signature
	GlobalRootSignature root(winHandle, device, {});
	mEmptyRootSignature = root.pRootSig;
	subobjects[index++] = root.subobject; //13

	// Create the state
	D3D12_STATE_OBJECT_DESC desc;
	desc.NumSubobjects = index; // 13
	desc.pSubobjects = subobjects.data();
	desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

	RendererUtil::D3DCall(winHandle, device->CreateStateObject(&desc, IID_PPV_ARGS(&mPipelineStateObject)));
}
