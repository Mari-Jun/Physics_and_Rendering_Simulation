#include "stdafx.h"
#include "PARS/Actor/Pawn.h"
#include "PARS/Actor/Controller/Controller.h"

namespace PARS
{
	Pawn::Pawn()
	{
	}

	void Pawn::Initialize()
	{
		m_MovementComp = CreateSPtr<MovementComponent>();
		AddComponent(m_MovementComp);
	}

	void Pawn::Shutdown()
	{
	}
}