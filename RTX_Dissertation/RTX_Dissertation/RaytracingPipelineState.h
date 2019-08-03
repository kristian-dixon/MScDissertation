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

	void GetEntryPoints(vector<const WCHAR*>& entryPoints);
	void BuildPipeline(HWND winHandle, ID3D12Device5Ptr device);

	ID3D12StateObjectPtr GetPipelineObject() { return mPipelineStateObject; };
private:

	string mShaderFileName;

	vector<HitProgram*> mHitPrograms;
	vector<HitProgram*> mEmptyHitPrograms;
	vector<MissProgram*> mMissPrograms;
	vector<MissProgram*> mEmptyMissPrograms;

	ShaderConfig mShaderConfig;
	PipelineConfig mPipelineConfig;

	ID3D12RootSignaturePtr mEmptyRootSignature;

	ID3D12StateObjectPtr mPipelineStateObject;
};

