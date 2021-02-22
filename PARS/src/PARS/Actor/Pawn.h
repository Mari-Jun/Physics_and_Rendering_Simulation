#pragma once
#include "PARS/Actor/Actor.h"

namespace PARS
{
	class Pawn : public Actor
	{
	public:
		Pawn(const WPtr<class Controller>& controller);
		virtual ~Pawn() = default;

	protected:
		WPtr<class Controller> m_Controller;
		//Movement Ŭ���� <�־����>

	public:
		const SPtr<class Controller>& GetController();
	};
}


