// Copyright (c) 2023. Johan Lind

#include "jleGame.h"
#include <glm/ext/matrix_transform.hpp>

void
jleGame::updateActiveScenes(float dt)
{
    JLE_SCOPE_PROFILE_CPU(jleGame_updateActiveScenes)
    for (int i = _activeScenes.size() - 1; i >= 0; i--) {
        if (_activeScenes[i]->bPendingSceneDestruction) {
            _activeScenes.erase(_activeScenes.begin() + i);
            continue;
        }

        _activeScenes[i]->updateScene();
        _activeScenes[i]->processNewSceneObjects();
        _activeScenes[i]->updateSceneObjects(dt);
    }
}

std::vector<std::shared_ptr<jleScene>> &
jleGame::activeScenesRef()
{
    return _activeScenes;
}

jleGame::jleGame()
{
}


