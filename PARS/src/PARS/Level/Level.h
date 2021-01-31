#pragma once

#include "PARS/Core/Core.h"

namespace PARS
{
	class ActorManager;

	class Level
	{
	public:
		enum class LevelState
		{
			Active, Paused, Dead
		};

		Level(const std::string& name = "Defualt_Level");
		virtual ~Level();

		virtual void Initialize();			//override this function, please enter "Level::Initailze()" at first
		virtual void Shutdown();			//override this function, please enter "Level::Shutdown()" at last
		virtual void LevelInput() {}
		void Update(float deltaTime);
		void UpdateActorManager(float deltaTime);
		virtual void UpdateLevel(float deltaTime) {};

		void AddActor(const SPtr<class Actor>& level);
		void AddLayer(const SPtr<class Layer>& layer);

	protected:
		std::string m_LevelName;
		LevelState m_LevelState;

	private:
		UPtr<ActorManager> m_ActorManager;

	public:
		const std::string& GetLevelName() const { return m_LevelName; }
		LevelState GetLevelState() const { return m_LevelState; }
		void SetLevelState(LevelState state) { m_LevelState = state; }
		void SetStateDead() { m_LevelState = LevelState::Dead; }
	};
}


