#pragma once

#include "PARS/Actor/Pawn.h"
#include "PARS/Input/Input.h"

namespace PARS
{
	class Controller : public Actor
	{
	public:
		Controller();
		Controller(const SPtr<class Pawn>& pawn);
		virtual ~Controller() = default;

		virtual void Initialize() override {}
		virtual void Shutdown() override {}
		virtual void Update(float deltaTime) override {}

	public:
		const SPtr<Pawn>& GetControlledPawn() const { return m_ControlledPawn; }
		void SetControlledPawn(const SPtr<Pawn>& pawn) { m_ControlledPawn = pawn; }

	protected:
		SPtr<Pawn> m_ControlledPawn = nullptr;
	};
}


