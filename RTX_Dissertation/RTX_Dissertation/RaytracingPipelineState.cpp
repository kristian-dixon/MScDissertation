#include "RaytracingPipelineState.h"


RaytracingPipelineState::RaytracingPipelineState(string& shaderFileName) : 
	mShaderFileName(shaderFileName), 
	mShaderConfig(sizeof(float) * 2, sizeof(float) * 3),
	mPipelineConfig(2)
{
}

void RaytracingPipelineState::AddHitProgram(HitProgram& hitProgram)
{
	if(hitProgram.localRootSignature == nullptr)
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

void RaytracingPipelineState::AddMissProgram(MissProgram& missProgram)
{
	if (missProgram.localRootSignature == nullptr)
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

void RaytracingPipelineState::BuildPipeline(HWND winHandle, ID3D12Device5Ptr device)
{
	//TODO:: Figure out how many spaces we need.

	//1 for loading shader
	//1 per hit program
	//1 for ray gen
	//2 per object with root signature
	//1 for all empties
	//1 for shader config
	//1 for pipeline config
	//1 for global root signature

	// 6 + (hit groups total) + (hitPrograms with root signatures * 2) + (miss programs with root signatures * 2)

	const int subobjectCount = 6 + (mHitPrograms.size() + mEmptyHitPrograms.size()) + (mHitPrograms.size() * 2) + (mMissPrograms.size() * 2);


	std::vector<D3D12_STATE_SUBOBJECT> subobjects(subobjectCount);
	uint32_t index = 0;

	const WCHAR* kRayGenShader = L"rayGen";
	const WCHAR* kMissShader = L"miss";
	const WCHAR* kClosestHitShader = L"chs";
	const WCHAR* kShadowChs = L"shadowChs";
	const WCHAR* kShadowMiss = L"shadowMiss";

	const WCHAR* kHitGroup = L"HitGroup";
	const WCHAR* kShadowHitGroup = L"ShadowHitGroup";


	const WCHAR* entryPoints[] = { kRayGenShader, kMissShader, kClosestHitShader, kShadowChs, kShadowMiss };

	// Create the DXIL library
	DxilLibrary dxilLib;// = RendererUtil::CreateDxilLibrary(winHandle, RendererUtil::string_2_wstring("Data/Shaders.hlsl"), entryPoints);
	subobjects[index++] = dxilLib.stateSubobject; // 0 Library

	HitProgram hitProgram(nullptr, kClosestHitShader, kHitGroup);
	subobjects[index++] = hitProgram.subObject; // 1 Hit Group

	HitProgram shadowHitProgram(nullptr, kShadowChs, kShadowHitGroup);
	subobjects[index++] = shadowHitProgram.subObject; // 2 Shadow Hit Group

	// Create the ray-gen root-signature and association
	LocalRootSignature rgsRootSignature(winHandle, device, RendererUtil::CreateRayGenRootDesc().desc);
	subobjects[index] = rgsRootSignature.subobject; // 3 RayGen Root Sig

	uint32_t rgsRootIndex = index++; // 3
	ExportAssociation rgsRootAssociation(&kRayGenShader, 1, &(subobjects[rgsRootIndex]));
	subobjects[index++] = rgsRootAssociation.subobject; // 4 Associate Root Sig to RGS

	///

	// Create the tri hit root-signature and association
	LocalRootSignature hitRootSignature(winHandle, device, RendererUtil::CreateHitRootDesc().desc);
	subobjects[index] = hitRootSignature.subobject; // 5 Triangle Hit Root Sig

	uint32_t hitRootIndex = index++; // 5
	ExportAssociation hitRootAssociation(&kClosestHitShader, 1, &(subobjects[hitRootIndex]));
	subobjects[index++] = hitRootAssociation.subobject; // 6 Associate Triangle Root Sig to Triangle Hit Group

	///

	// Create the miss- and hit-programs root-signature and association
	D3D12_ROOT_SIGNATURE_DESC emptyDesc = {};
	emptyDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	LocalRootSignature emptyRootSignature(winHandle, device, emptyDesc);
	subobjects[index] = emptyRootSignature.subobject; // 7 Root Sig to be shared between Miss and CHS

	uint32_t hitMissRootIndex = index++; // 8
	const WCHAR* missHitExportName[] = { kMissShader, kShadowChs, kShadowMiss };
	ExportAssociation missHitRootAssociation(missHitExportName, arraysize(missHitExportName), &(emptyRootSignature.subobject));
	subobjects[index++] = missHitRootAssociation.subobject; // 9 Associate Root Sig to Miss and CHS

	// Bind the payload size to the programs
	ShaderConfig shaderConfig(sizeof(float) * 2, sizeof(float) * 3);
	subobjects[index] = shaderConfig.subobject; // 10 Shader Config

	uint32_t shaderConfigIndex = index++; // 10
	const WCHAR* shaderExports[] = { kMissShader, kClosestHitShader, kRayGenShader, kShadowChs, kShadowMiss };
	ExportAssociation configAssociation(shaderExports, arraysize(shaderExports), &(subobjects[shaderConfigIndex]));
	subobjects[index++] = configAssociation.subobject; //11 Associate Shader Config to Miss, CHS, RGS

	// Create the pipeline config
	PipelineConfig config(2);
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
