#pragma once

#include "PARS/Core/Core.h"

namespace PARS
{
	enum class ShaderType
	{
		Color
	};

	class RenderComponentFactory
	{
	private:
		static RenderComponentFactory* s_Instance;

	public:
		RenderComponentFactory(const SPtr<class DirectX12>& directX);
		~RenderComponentFactory() = default;

		bool Initialize();
		void Shutdown(); 
		void BeginDraw();
		void Draw(ShaderType type);
		void PrepareToNextDraw();

	public:
		inline static RenderComponentFactory* GetRenderComponentFactory() { return s_Instance; }

		void AddRenderComponent(ShaderType type, const SPtr<class RenderComponent>& component);
		void RemoveRenderComponent(ShaderType type, const SPtr<class RenderComponent>& component);
		void MoveToPrepareComponent(ShaderType type, const SPtr<class RenderComponent>& component);

		SPtr<class Mesh> GetMesh(std::string&& fileName) const;
		void SaveMesh(std::string&& fileName, const SPtr<class Mesh>& mesh);

	private:
		SPtr<class DirectX12> m_DirectX12;
		std::unordered_map<ShaderType, std::vector<SPtr<class RenderComponent>>> m_PrepareComponents;
		std::unordered_map<ShaderType, std::vector<SPtr<class RenderComponent>>> m_RenderComponents;
		std::unordered_map<std::string, SPtr<class Mesh>> m_MeshCache;
	};
}


