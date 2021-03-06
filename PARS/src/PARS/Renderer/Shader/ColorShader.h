#pragma once

#include "PARS/Renderer/Shader/Core/Shader.h"

namespace PARS
{
	struct CBWorldMat
	{
		Mat4 m_WorldMatrix;
	};

	struct CBPassFrame
	{
		Mat4 m_ViewProj;
	};

	class ColorShader : public Shader
	{
	public:
		ColorShader(const SPtr<DirectX12>& directX);
		virtual ~ColorShader() = default;

		virtual void Shutdown() override;
		virtual void RenderReady(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT numOfObject) override;
		void UpdateShaderVariables(ID3D12GraphicsCommandList* commandList, const std::vector<SPtr<RenderComponent>>& renderComps, const CBPassFrame& cbPass);
		virtual void DrawRenderComp(ID3D12GraphicsCommandList* commandList, const SPtr<RenderComponent>& renderComp, int index) override;
		virtual void DrawPassFrame(ID3D12GraphicsCommandList* commandList);
		 
	public:
		virtual void CreateShader() override;
		virtual void CreateInputLayout()  override;
		virtual bool CreatePSO(ID3D12RootSignature* rootSignature) override;

	protected:
		ID3D12Resource* m_WorldMatCB = nullptr;
		CBWorldMat* m_MappedWorldMat = nullptr;
		
		ID3D12Resource* m_PassFrameCB = nullptr;
		CBPassFrame* m_MappedPassFrame = nullptr;
	};

}

