#pragma once

#include <string>
#include <vector>
#include "RendererUtil.h"

using namespace std;
#pragma once
class RaytracingPipelineState
{
public:
	RaytracingPipelineState(string& shaderFileName);

	void AddHitProgram(HitProgram& hitProgram);
	void AddMissProgram(MissProgram& missProgram);

	void BuildPipeline(HWND winHandle, ID3D12Device5Ptr device);

private:

	string mShaderFileName;

	//Names of the shaders that'll be run (don't include the hit group names)
	vector<string> mEntryPoints;

	vector<HitProgram> mHitPrograms;
	vector<HitProgram> mEmptyHitPrograms;
	vector<MissProgram> mMissPrograms;
	vector<MissProgram> mEmptyMissPrograms;

	ShaderConfig mShaderConfig;
	PipelineConfig mPipelineConfig;

	ID3D12RootSignaturePtr mEmptyRootSignature;

	ID3D12StateObjectPtr mPipelineStateObject;
};

