// Copyright (c) 2023. Johan Lind

#include "jleScene.h"
#include "jleGameEngine.h"
#include "jleObject.h"
#include "jleProfiler.h"

#include <filesystem>
#include <iostream>

JLE_EXTERN_TEMPLATE_CEREAL_CPP(jleScene)

int jleScene::_scenesCreatedCount{0};

jleScene::jleScene() {
    sceneName = "Scene_" + std::to_string(_scenesCreatedCount);
    _scenesCreatedCount++;
}

jleScene::jleScene(const std::string &sceneName) {
    this->sceneName = sceneName;
    _scenesCreatedCount++;
}

void jleScene::updateSceneObjects(float dt) {
    JLE_SCOPE_PROFILE_CPU(jleScene_updateSceneObjects)
    for (int32_t i = _sceneObjects.size() - 1; i >= 0; i--) {
        if (_sceneObjects[i]->_pendingKill) {
            _sceneObjects[i]->propagateDestroy();
            _sceneObjects.erase(_sceneObjects.begin() + i);
            continue;
        }

        _sceneObjects[i]->update(dt);
        _sceneObjects[i]->updateComponents(dt);
        _sceneObjects[i]->updateChildren(dt);
    }
}

void
jleScene::updateSceneObejctsEditor(float dt)
{

    JLE_SCOPE_PROFILE_CPU(jleScene_updateSceneObejctsEditor)
    for (int32_t i = _sceneObjects.size() - 1; i >= 0; i--) {
        if (_sceneObjects[i]->_pendingKill) {
            _sceneObjects.erase(_sceneObjects.begin() + i);
            continue;
        }

        _sceneObjects[i]->editorUpdate(dt);
        _sceneObjects[i]->updateComponentsEditor(dt);
        _sceneObjects[i]->updateChildrenEditor(dt);
    }

}

void jleScene::processNewSceneObjects() {
    JLE_SCOPE_PROFILE_CPU(jleScene_processNewSceneObjects)
    if (!_newSceneObjects.empty()) {
        for (const auto &newObject : _newSceneObjects) {
            if (!newObject->_isStarted) {
                if(!gEngine->isGameKilled())
                {
                    newObject->start();
                    newObject->startComponents();
                }
                newObject->_isStarted = true;
            }

            // Only push back objects existing directly in the scene into scene
            // objects The object can be placed as a child object in another
            // object, and thus no longer existing directly in the scene
            if (newObject->_parentObject == nullptr) {
                _sceneObjects.push_back(newObject);
            }
        }

        _newSceneObjects.clear();
    }
}

void jleScene::destroyScene() {
    bPendingSceneDestruction = true;
    onSceneDestruction();
}

void jleScene::configurateSpawnedObject(const std::shared_ptr<jleObject> &obj) {
    obj->_containedInScene = this;
    obj->_instanceName = std::string{obj->objectNameVirtual()} + "_" +
                         std::to_string(obj->_instanceID);

    obj->replaceChildrenWithTemplate();
    _newSceneObjects.push_back(obj);
}

void
jleScene::startObjects()
{
    for(auto&& o : _sceneObjects)
    {
        startObject(&*o);
    }
}

void
jleScene::startObject(jleObject *o)
{
    if (!o->_isStarted) {
        o->start();
        o->startComponents();
        o->_isStarted = true;
        for(auto&& c : o->__childObjects)
        {
            startObject(&*c);
        }
    }
}

void
jleScene::saveScene()
{
    saveToFile();
}

void
jleScene::spawnObject(std::shared_ptr<jleObject> object)
{
    configurateSpawnedObject(object);
}

std::shared_ptr<jleObject>
jleScene::spawnObjectWithName(const std::string &name)
{
    auto obj = spawnObject<jleObject>();
    obj->_instanceName = name;
    return obj;
}

