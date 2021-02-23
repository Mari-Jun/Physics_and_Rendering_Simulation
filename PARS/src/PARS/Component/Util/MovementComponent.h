#pragma once
#include "PARS/Component/Component.h"

namespace PARS
{
	class MovementComponent : public Component
	{
	public:
		MovementComponent(int updateOrder = 10);
		virtual ~MovementComponent() = default;

		virtual void Update(float deltaTime) override;

	public:
		void SetForwardSpeed(float speed) { m_ForwardSpeed = speed; }
		void SetRightSpeed(float speed) { m_RightSpeed = speed; }
		void SetUpSpeed(float speed) { m_UpSpeed = speed; }

	protected:
		float m_ForwardSpeed;
		float m_RightSpeed;
		float m_UpSpeed;
	};

}

