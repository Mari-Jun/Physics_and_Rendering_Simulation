#pragma once
#include "PARS/Actor/Controller/Controller.h"

namespace PARS
{
	class PlayerController : public Controller
	{
	public:
		PlayerController();
		PlayerController(const SPtr<class Pawn>& pawn);
		virtual ~PlayerController() = default;

		virtual void Initialize() override;
		virtual void Shutdown() override {}
		virtual void Update(float deltaTime) override;

	private:
		bool b_UseDefaultKeyEvent = true;
		bool b_UseDefaultMouseEvent = true;
		bool b_IsSameRotationWithPawn = true;

	public:
		void SetUseDefaultKeyEvent(bool use) { b_UseDefaultKeyEvent = use; }
		void SetUseDefaultMouseEvent(bool use) { b_UseDefaultMouseEvent = use; }
		void SetSameRoationWithPawn(bool boolean) { b_IsSameRotationWithPawn = boolean; }

	private:
		void MoveForward(float axis);
		void MoveRightward(float axis);
	};
}


