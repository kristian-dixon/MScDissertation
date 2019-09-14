#pragma once

#include <string>
#include <vector>
#include "RendererUtil.h"

using namespace std;
#pragma once
class RaytracingPipelineState
{
public:
	RaytracingPipelineState(wstring& shaderFileName);

	void AddHitProgram(shared_ptr<HitProgram> hitProgram);
	void AddMissProgram(shared_ptr<MissProgram> missProgram);

	void GetEntryPoints(vector<const WCHAR*>& entryPoints);
	void BuildPipeline(HWND winHandle, ID3D12Device5Ptr device);

	ID3D12StateObjectPtr GetPipelineObject() const { return mPipelineStateObject; };
private:

	wstring mShaderFileName;

	vector<shared_ptr<HitProgram>> mHitPrograms;
	vector<shared_ptr<HitProgram>> mEmptyHitPrograms;
	vector<shared_ptr<MissProgram>> mMissPrograms;
	vector<shared_ptr<MissProgram>> mEmptyMissPrograms;

	ShaderConfig mShaderConfig;
	PipelineConfig mPipelineConfig;

	ID3D12RootSignaturePtr mEmptyRootSignature;

	ID3D12StateObjectPtr mPipelineStateObject;
};

