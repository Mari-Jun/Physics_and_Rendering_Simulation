#include "stdafx.h"
#include "PARS/Actor/ActorManager.h"

namespace PARS
{
	void ActorManager::ProcessInput()
	{
		for (const auto& actor : m_Actors)
		{
			if (actor->GetActorState() == Actor::ActorState::Active)
			{
				actor->ProcessInput();
			}
		}
	}

	void ActorManager::Update(float deltaTime)
	{
		m_IsUpdateActors = true;
		for (const auto& actor : m_Actors)
		{
			if (actor->GetActorState() == Actor::ActorState::Active)
			{
				actor->UpdateActor(deltaTime);
			}
		}
		m_IsUpdateActors = false;

		for (auto actor : m_ReadyActors)
		{
			actor->UpdateWorldMatrix();
			m_Actors.emplace_back(actor);
		}
		m_ReadyActors.clear();

		std::vector<SPtr<Actor>> deadActors;
		for (auto actor : m_Actors)
		{
			if (actor->GetActorState() == Actor::ActorState::Dead)
			{
				deadActors.emplace_back(actor);
			}
		}

		for (auto actor : deadActors)
		{
			RemoveActor(actor);
		}
	}

	void ActorManager::Shutdown()
	{
		for (auto actor : m_Actors)
		{
			if (actor != nullptr)
			{
				actor->ShutdownActor();
			}
		}
	}

	void ActorManager::AddActor(const SPtr<Actor>& actor)
	{
		if (m_IsUpdateActors)
		{
			m_ReadyActors.emplace_back(actor);
		}
		else
		{
			m_Actors.emplace_back(actor);
		}
		actor->InitializeActor();
	}

	void ActorManager::RemoveActor(const WPtr<Actor>& actor)
	{
		auto iter = std::find_if(m_ReadyActors.begin(), m_ReadyActors.end(),
			[&actor](const WPtr<Actor>& ac)
			{return actor.lock() == ac.lock(); });
		if (iter != m_ReadyActors.end())
		{
			actor.lock()->ShutdownActor();
			m_ReadyActors.erase(iter);
		}

		iter = std::find_if(m_Actors.begin(), m_Actors.end(),
			[&actor](const WPtr<Actor>& ac)
			{return actor.lock() == ac.lock(); });
		if (iter != m_Actors.end())
		{
			actor.lock()->ShutdownActor();
			m_Actors.erase(iter);
		}
	}
}