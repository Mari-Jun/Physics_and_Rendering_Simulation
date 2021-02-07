#pragma once

#include "PARS/Core/Core.h"
#include "PARS/Math/Math.h"

namespace PARS
{
	class DirectX12;
	class ImGuiLayer;

	class Renderer
	{
	public:
		Renderer();
		virtual ~Renderer();
	
		bool Initialize();
		void ShutDown();
		void Draw();

	public:
		const SPtr<ImGuiLayer>& CreateImGui();

		static void SetClearColor(const Vec4& color) { s_ClearColor = color; }

	private:
		static Vec4 s_ClearColor;
		UPtr<DirectX12> m_DirectX12;
		SPtr<ImGuiLayer> m_ImGuiLayer;
	};
}



