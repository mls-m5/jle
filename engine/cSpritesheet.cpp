// Copyright (c) 2023. Johan Lind

#include "cSpritesheet.h"
#include "jleCore.h"
#include "jleGameEngine.h"
#include "jleQuadRendering.h"
#include "jleResource.h"
#include "json.hpp"
#include <plog/Log.h>

void cSpritesheet::start() {
}

cSpritesheet::cSpritesheet(jleObject *owner, jleScene *scene)
    : jleComponent(owner, scene) {}

void cSpritesheet::update(float dt) {
    if (!_spritesheet) {
        return;
    }

    auto &texture = _spritesheet->_imageTexture;
    if (texture != nullptr && _hasEntity) {
        texturedQuad quad{texture};
        quad.x = getTransform().getWorldPosition().x + _spritesheetEntityCache.sourceSize.x - _offset.x;
        quad.y = getTransform().getWorldPosition().y + _spritesheetEntityCache.sourceSize.y - _offset.y;

        quad.height = _spritesheetEntityCache.frame.height;
        quad.width = _spritesheetEntityCache.frame.width;
        quad.textureX = _spritesheetEntityCache.frame.x;
        quad.textureY = _spritesheetEntityCache.frame.y;
        quad.depth = getTransform().getWorldPosition().z;

        if (quad.texture.get()) {
            gCore->quadRendering().sendTexturedQuad(quad);
        }
    }
}

void cSpritesheet::toJson(nlohmann::json &j_out) {
    j_out["_spritesheetPath"] = _spritesheetPath;
    j_out["_spriteName"] = _spriteName;
    j_out["_offsetX"] = _offset.x;
    j_out["_offsetY"] = _offset.y;
}

void cSpritesheet::fromJson(const nlohmann::json &j_in) {
    _spritesheetPath = j_in["_spritesheetPath"];
    _spriteName = j_in["_spriteName"];
    _offset.x = j_in.value("_offsetX", 0);
    _offset.y = j_in.value("_offsetY", 0);
    _spritesheet = gCore->resources().loadResourceFromFile<jleSpritesheet>(
        jleRelativePath{_spritesheetPath});

    entity(_spriteName);
}

void cSpritesheet::entity(const std::string &entityName) {
    if (!_spritesheet) {
        return;
    }

    _spriteName = entityName;
    const auto entity = _spritesheet->_spritesheetEntities.find(_spriteName);
    if (entity != _spritesheet->_spritesheetEntities.end()) {
        _spritesheetEntityCache = entity->second;
        _hasEntity = true;
    }
    else {
        LOGW << "Could not find sprite entity on the spritesheet: "
             << _spriteName;
    }
}
