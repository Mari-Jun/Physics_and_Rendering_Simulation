#include "stdafx.h"
#include "PARS/Component/Render/RenderComponent.h"

namespace PARS
{
	RenderComponent::RenderComponent(int updateOrder)
		: Component(updateOrder)
		, m_RenderShaderType(ShaderType::Color)
	{
	}

	void RenderComponent::Initialize()
	{
		auto factory = RenderComponentFactory::GetRenderComponentFactory();
		factory->AddRenderComponent(m_RenderShaderType, std::reinterpret_pointer_cast<RenderComponent>(shared_from_this()));
	}

	void RenderComponent::Shutdown()
	{
		auto factory = RenderComponentFactory::GetRenderComponentFactory();
		factory->RemoveRenderComponent(m_RenderShaderType, std::reinterpret_pointer_cast<RenderComponent>(shared_from_this()));
	}

	void RenderComponent::ChangeComponentItem()
	{
		auto factory = RenderComponentFactory::GetRenderComponentFactory();
		factory->MoveToPrepareComponent(m_RenderShaderType, std::reinterpret_pointer_cast<RenderComponent>(shared_from_this()));
	}
}
