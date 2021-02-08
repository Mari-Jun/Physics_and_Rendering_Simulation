#pragma once

#include "PARS/Level/Level.h"

namespace PARS
{
	class LevelManager final
	{
	public:
		LevelManager();
		~LevelManager() = default;

		void ProcessInput();
		void Update(float deltaTime);
		void Shutdown();

		void AddLevel(const SPtr<Level>& level);
		void RemoveLevel(const WPtr<Level>& level);

		inline static LevelManager* GetLevelManager() { return s_Instance; }

	private:
		bool m_IsUpdateLevels = false;
		SPtr<Level> m_ReadyLevels = nullptr;
		std::stack<SPtr<Level>> m_Levels;
		static LevelManager* s_Instance;
	};
}


