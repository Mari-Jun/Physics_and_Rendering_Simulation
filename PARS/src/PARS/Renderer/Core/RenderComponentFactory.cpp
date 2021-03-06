#include "stdafx.h"
#include "PARS/Renderer/DirectX12/DirectX12.h"
#include "PARS/Component/Render/RenderComponent.h"
#include "PARS/Renderer/Core/RenderComponentFactory.h"

#include "PARS/Renderer/Shader/ColorShader.h"

namespace PARS
{
	RenderComponentFactory::RenderComponentFactory(const SPtr<DirectX12>& directX)
		: m_DirectX12(directX)
	{
	}

	bool RenderComponentFactory::Initialize()
	{
		s_Instance = this;
		return true;
	}

	void RenderComponentFactory::Shutdown()
	{
		for (auto iter = m_Shaders.begin(); iter != m_Shaders.end(); ++iter)
		{
			iter->second->Shutdown();
		}
	}

	void RenderComponentFactory::RenderReady()
	{
		if (!m_PrepareComponents.empty())
		{
			for (auto& comps : m_PrepareComponents)
			{
				auto iter = m_RenderComponents.emplace(comps);
				if (!iter.second)
				{
					for (auto& comp : comps.second)
					{
						comp->RenderReady(m_DirectX12->GetDevice(), m_DirectX12->GetCommandList());
						iter.first->second.emplace_back(comp);
					}
				}
				else
				{
					for (auto& comp : iter.first->second)
					{
						comp->RenderReady(m_DirectX12->GetDevice(), m_DirectX12->GetCommandList());
					}
				}

				m_IsShaderUpdate[comps.first] = true;
			}
		}

		for (auto& shader : m_IsShaderUpdate)
		{
			if (shader.second)
			{
				shader.second = false;
				m_Shaders[shader.first]->RenderReady(m_DirectX12->GetDevice(), m_DirectX12->GetCommandList(),
					static_cast<UINT>(m_RenderComponents[shader.first].size()));
			}
		}
	}

	void RenderComponentFactory::Draw(ShaderType type)
	{
		for (int index = 0; index < m_RenderComponents[type].size(); ++index)
		{
			m_Shaders[type]->DrawRenderComp(m_DirectX12->GetCommandList(), m_RenderComponents[type][index], index);
			m_Shaders[type]->DrawPassFrame(m_DirectX12->GetCommandList());
			m_RenderComponents[type][index]->Draw(m_DirectX12->GetCommandList());
		}
	}

	void RenderComponentFactory::PrepareToNextDraw()
	{
		if (!m_PrepareComponents.empty())
		{
			for (auto& comps : m_PrepareComponents)
			{
				for (auto& comp : comps.second)
				{
					comp->ReleaseUploadBuffers();
				}
			}
			m_PrepareComponents.clear();
		}
	}

	void RenderComponentFactory::AddShader(ShaderType type, SPtr<Shader>&& shader)
	{
		m_Shaders.insert({ type, std::move(shader) });
		m_IsShaderUpdate.insert({ type, false });
	}

	void RenderComponentFactory::AddRenderComponent(ShaderType type, const SPtr<RenderComponent>& component)
	{
		auto iter = m_PrepareComponents.emplace(type, std::vector<SPtr<RenderComponent>>{component});
		if (!iter.second)
		{
			iter.first->second.emplace_back(component);
		}		
	}

	void RenderComponentFactory::RemoveRenderComponent(ShaderType type, const SPtr<RenderComponent>& component)
	{
		auto rComp = m_RenderComponents.find(type);
		if (rComp != m_RenderComponents.end())
		{
			auto iter = std::find_if(rComp->second.begin(), rComp->second.end(),
				[&component](const SPtr<RenderComponent>& comp)
				{return component == comp; });
			if (iter != rComp->second.end())
			{
				rComp->second.erase(iter);
				if (rComp->second.empty())
				{
					m_RenderComponents.erase(type);
				}
				else
				{
					m_IsShaderUpdate[rComp->first] = true;
				}
			}
		}
	}

	void RenderComponentFactory::MoveToPrepareComponent(ShaderType type, const SPtr<class RenderComponent>& component)
	{
		RemoveRenderComponent(type, component);
		AddRenderComponent(type, component);
	}

	SPtr<Mesh> RenderComponentFactory::GetMesh(const std::string& fileName) const
	{
		SPtr<Mesh> mesh = nullptr;

		auto iter = m_MeshCache.find(fileName);
		if (iter != m_MeshCache.end())
		{
			mesh = iter->second;
		}

		return mesh;
	}

	void RenderComponentFactory::SaveMesh(const std::string& fileName, const SPtr<class Mesh>& mesh)
	{
		m_MeshCache.emplace(fileName, mesh);
	}

	RenderComponentFactory* RenderComponentFactory::s_Instance = nullptr;
}
