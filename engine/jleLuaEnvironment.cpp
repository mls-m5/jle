// Copyright (c) 2023. Johan Lind

#include "jleLuaEnvironment.h"
#include "jleLuaScript.h"
#include "jleObject.h"
#include "jlePath.h"
#include "jleResourceRef.h"
#include <glm/ext/matrix_transform.hpp>

#ifdef BUILD_EDITOR
#include "ImGui/sol_ImGui.h"
#endif

jleLuaEnvironment::jleLuaEnvironment()
{
    _luaState = sol::state{};
    setupLua(_luaState);
}

void
jleLuaEnvironment::setupLua(sol::state &lua)
{

    lua.open_libraries(sol::lib::base,
                       sol::lib::math,
                       sol::lib::string,
                       sol::lib::coroutine,
                       sol::lib::package,
                       sol::lib::debug,
                       sol::lib::io,
                       sol::lib::table,
                       sol::lib::os);

    setupLuaGLM(lua);

#ifdef BUILD_EDITOR
    sol_ImGui::Init(lua);
#endif

    lua.set_function("loadScript", [&](const std::string path) {
        loadScript(path.c_str());
    });

    lua.new_usertype<jlePath>(
        "jlePath",
        sol::constructors<jlePath(), jlePath(const std::string &), jlePath(const std::string &, bool)>(),
        "prefix",
        &jlePath::getPathPrefix,
        "virtual",
        &jlePath::getVirtualPathConst,
        "real",
        &jlePath::getRealPathConst,
        "isEmpty",
        &jlePath::isEmpty,
        "fileEnding",
        &jlePath::getFileEnding,
        "fileName",
        &jlePath::getFileNameNoEnding,
        sol::meta_function::to_string,
        &jlePath::getVirtualPathConst);

    lua.new_usertype<jleTransform>("jleTransform",
                                   "localPos",
                                   &jleTransform::getLocalPosition,
                                   "worldPos",
                                   &jleTransform::getWorldPosition,
                                   "setLocalPos",
                                   &jleTransform::setLocalPosition,
                                   "setWorldPos",
                                   &jleTransform::setWorldPosition,
                                   "worldMatrix",
                                   &jleTransform::getWorldMatrix,
                                   "localMatrix",
                                   &jleTransform::getLocalMatrix);

    lua.new_usertype<jleObject>(
        "jleObject",
        "name",
        &jleObject::_instanceName,
        "transform",
        &jleObject::getTransform,
        "addComponent",
        &jleObject::addComponentByName,
        "duplicate",
        [](jleObject &object) { return object.duplicate().get(); },
        "destroy",
        &jleObject::destroyObject,
        "pendingKill",
        sol::readonly(&jleObject::_pendingKill),
        "isStarted",
        sol::readonly(&jleObject::_isStarted),
        "instanceID",
        sol::readonly(&jleObject::_instanceID),
        "scene",
        sol::readonly(&jleObject::_containedInScene));

    lua.new_usertype<jleScene>("jleScene",
                               "name",
                               &jleScene::sceneName,
                               "spawnObject",
                               &jleScene::spawnObjectWithName,
                               "destroy",
                               &jleScene::destroyScene,
                               "objects",
                               &jleScene::sceneObjects);

    lua.set_function("LOGE", [](const std::string &s) {
        if (!plog::get<0>() || !plog::get<0>()->checkSeverity(plog::error)) {
            ;
        } else
            (*plog::get<0>()) += plog::Record(plog::error, "(Lua)", 0, "(Lua)", reinterpret_cast<void *>(0), 0).ref()
                                 << s;
    });

    lua.set_function("LOGF", [](const std::string &s) {
        if (!plog::get<0>() || !plog::get<0>()->checkSeverity(plog::fatal)) {
            ;
        } else
            (*plog::get<0>()) += plog::Record(plog::fatal, "(Lua)", 0, "(Lua)", reinterpret_cast<void *>(0), 0).ref()
                                 << s;
    });

    lua.set_function("LOGI", [](const std::string &s) {
        if (!plog::get<0>() || !plog::get<0>()->checkSeverity(plog::info)) {
            ;
        } else
            (*plog::get<0>()) += plog::Record(plog::info, "(Lua)", 0, "(Lua)", reinterpret_cast<void *>(0), 0).ref()
                                 << s;
    });
    lua.set_function("LOGW", [](const std::string &s) {
        if (!plog::get<0>() || !plog::get<0>()->checkSeverity(plog::warning)) {
            ;
        } else
            (*plog::get<0>()) += plog::Record(plog::warning, "(Lua)", 0, "(Lua)", reinterpret_cast<void *>(0), 0).ref()
                                 << s;
    });
    lua.set_function("LOGV", [](const std::string &s) {
        if (!plog::get<0>() || !plog::get<0>()->checkSeverity(plog::verbose)) {
            ;
        } else
            (*plog::get<0>()) += plog::Record(plog::verbose, "(Lua)", 0, "(Lua)", reinterpret_cast<void *>(0), 0).ref()
                                 << s;
    });

    for (auto &c : jleTypeReflectionUtils::registeredComponentsRef()) {
        auto instance = c.second();
        auto table = lua.create_table(c.first);
        instance->registerLua(lua, table);
    }
}

void
jleLuaEnvironment::setupLuaGLM(sol::state &lua)
{
    auto multOverloadsVec2 =
        sol::overload([](const glm::vec2 &v1, const glm::vec2 &v2) -> glm::vec2 { return v1 * v2; },
                      [](const glm::vec2 &v1, float f) -> glm::vec2 { return v1 * f; },
                      [](float f, const glm::vec2 &v1) -> glm::vec2 { return f * v1; });

    auto divOverloadsVec2 = sol::overload([](const glm::vec2 &v1, const glm::vec2 &v2) -> glm::vec2 { return v1 / v2; },
                                          [](const glm::vec2 &v1, float f) -> glm::vec2 { return v1 / f; },
                                          [](float f, const glm::vec2 &v1) -> glm::vec2 { return f / v1; });

    auto addOverloadsVec2 =
        sol::overload([](const glm::vec2 &v1, const glm::vec2 &v2) -> glm::vec2 { return v1 + v2; });

    auto subtractOverloadsVec2 =
        sol::overload([](const glm::vec2 &v1, const glm::vec2 &v2) -> glm::vec2 { return v1 - v2; });

    auto toStrOverloadsVec2 = sol::overload(
        [](const glm::vec2 &v1) -> std::string { return std::to_string(v1.x) + ", " + std::to_string(v1.y); });

    auto multOverloadsVec3 =
        sol::overload([](const glm::vec3 &v1, const glm::vec3 &v2) -> glm::vec3 { return v1 * v2; },
                      [](const glm::vec3 &v1, float f) -> glm::vec3 { return v1 * f; },
                      [](float f, const glm::vec3 &v1) -> glm::vec3 { return f * v1; });

    auto divOverloadsVec3 = sol::overload([](const glm::vec3 &v1, const glm::vec3 &v2) -> glm::vec3 { return v1 / v2; },
                                          [](const glm::vec3 &v1, float f) -> glm::vec3 { return v1 / f; },
                                          [](float f, const glm::vec3 &v1) -> glm::vec3 { return f / v1; });

    auto addOverloadsVec3 =
        sol::overload([](const glm::vec3 &v1, const glm::vec3 &v2) -> glm::vec3 { return v1 + v2; });

    auto subtractOverloadsVec3 =
        sol::overload([](const glm::vec3 &v1, const glm::vec3 &v2) -> glm::vec3 { return v1 - v2; });

    auto toStrOverloadsVec3 = sol::overload([](const glm::vec3 &v1) -> std::string {
        return std::to_string(v1.x) + ", " + std::to_string(v1.y) + ", " + std::to_string(v1.z);
    });

    auto multOverloadsVec4 =
        sol::overload([](const glm::vec4 &v1, const glm::vec4 &v2) -> glm::vec4 { return v1 * v2; },
                      [](const glm::vec4 &v1, float f) -> glm::vec4 { return v1 * f; },
                      [](float f, const glm::vec4 &v1) -> glm::vec4 { return f * v1; });

    auto divOverloadsVec4 = sol::overload([](const glm::vec4 &v1, const glm::vec4 &v2) -> glm::vec4 { return v1 / v2; },
                                          [](const glm::vec4 &v1, float f) -> glm::vec4 { return v1 / f; },
                                          [](float f, const glm::vec4 &v1) -> glm::vec4 { return f / v1; });

    auto addOverloadsVec4 =
        sol::overload([](const glm::vec4 &v1, const glm::vec4 &v2) -> glm::vec4 { return v1 + v2; });

    auto subtractOverloadsVec4 =
        sol::overload([](const glm::vec4 &v1, const glm::vec4 &v2) -> glm::vec4 { return v1 - v2; });

    auto toStrOverloadsVec4 = sol::overload([](const glm::vec4 &v1) -> std::string {
        return std::to_string(v1.x) + ", " + std::to_string(v1.y) + ", " + std::to_string(v1.z) + ", " +
               std::to_string(v1.w);
    });

    auto vec2 = lua.new_usertype<glm::vec2>(
        "vec2",
        sol::constructors<glm::vec2(), glm::vec2(glm::vec2), glm::vec2(float), glm::vec2(float, float)>(),
        "x",
        &glm::vec2::x,
        "y",
        &glm::vec2::y,
        "length",
        [&](glm::vec2 &v) { return glm::length(v); },
        sol::meta_function::multiplication,
        multOverloadsVec2,
        sol::meta_function::division,
        divOverloadsVec2,
        sol::meta_function::addition,
        addOverloadsVec2,
        sol::meta_function::subtraction,
        subtractOverloadsVec2,
        sol::meta_function::to_string,
        toStrOverloadsVec2);

    auto vec3 = lua.new_usertype<glm::vec3>(
        "vec3",
        sol::constructors<glm::vec3(), glm::vec3(glm::vec3), glm::vec3(float), glm::vec3(float, float, float)>(),
        "x",
        &glm::vec3::x,
        "y",
        &glm::vec3::y,
        "z",
        &glm::vec3::z,
        "length",
        [&](glm::vec3 &v) { return glm::length(v); },
        sol::meta_function::multiplication,
        multOverloadsVec3,
        sol::meta_function::division,
        divOverloadsVec3,
        sol::meta_function::addition,
        addOverloadsVec3,
        sol::meta_function::subtraction,
        subtractOverloadsVec3,
        sol::meta_function::to_string,
        toStrOverloadsVec3);

    auto vec4 = lua.new_usertype<glm::vec4>(
        "vec4",
        sol::constructors<glm::vec4(), glm::vec4(glm::vec4), glm::vec4(float), glm::vec4(float, float, float, float)>(),
        "x",
        &glm::vec4::x,
        "y",
        &glm::vec4::y,
        "z",
        &glm::vec4::z,
        "w",
        &glm::vec4::w,
        "length",
        [&](glm::vec4 &v) { return glm::length(v); },
        sol::meta_function::multiplication,
        multOverloadsVec4,
        sol::meta_function::division,
        divOverloadsVec4,
        sol::meta_function::addition,
        addOverloadsVec4,
        sol::meta_function::subtraction,
        subtractOverloadsVec4,
        sol::meta_function::to_string,
        toStrOverloadsVec4);

    vec2.set_function("mix", [](glm::vec2 &v1, glm::vec2 &v2, float a) { return glm::mix(v1, v2, a); });
    vec3.set_function("mix", [](glm::vec3 &v1, glm::vec3 &v2, float a) { return glm::mix(v1, v2, a); });
    vec4.set_function("mix", [](glm::vec4 &v1, glm::vec4 &v2, float a) { return glm::mix(v1, v2, a); });

    auto toStrOverloadsMat2 = sol::overload([](const glm::mat2 &m) -> std::string {
        return "mat2=\n[" + std::to_string(m[0].x) + ", " + std::to_string(m[0].y) + +"]\n" + "[" +
               std::to_string(m[1].x) + ", " + std::to_string(m[1].y) + "]";
    });

    auto multOverloadsMat2 =
        sol::overload([](const glm::mat2 &m1, const glm::mat2 &m2) -> glm::mat2 { return m1 * m2; },
                      [](const glm::mat2 &m1, float f) -> glm::mat2 { return m1 * f; },
                      [](float f, const glm::mat2 &m1) -> glm::mat2 { return f * m1; });

    auto addOverloadsMat2 = sol::overload([](const glm::mat2 &m1, const glm::mat2 &m2) -> glm::mat2 { return m1 + m2; },
                                          [](const glm::mat2 &m1, float f) -> glm::mat2 { return m1 + f; },
                                          [](float f, const glm::mat2 &m1) -> glm::mat2 { return f + m1; });

    auto subtractOverloadsMat2 =
        sol::overload([](const glm::mat2 &m1, const glm::mat2 &m2) -> glm::mat2 { return m1 - m2; },
                      [](const glm::mat2 &m1, float f) -> glm::mat2 { return m1 - f; },
                      [](float f, const glm::mat2 &m1) -> glm::mat2 { return f - m1; });

    auto divOverloadsMat2 = sol::overload([](const glm::mat2 &m1, const glm::mat2 &m2) -> glm::mat2 { return m1 / m2; },
                                          [](const glm::mat2 &m1, float f) -> glm::mat2 { return m1 / f; },
                                          [](float f, const glm::mat2 &m1) -> glm::mat2 { return f / m1; });

    auto mat2 = lua.new_usertype<glm::mat2>(
        "mat2",
        sol::constructors<glm::mat2(), glm::mat2(float), glm::mat2(glm::mat2)>(),
        sol::meta_function::to_string,
        toStrOverloadsMat2,
        sol::meta_function::multiplication,
        multOverloadsMat2,
        sol::meta_function::addition,
        addOverloadsMat2,
        sol::meta_function::subtraction,
        subtractOverloadsMat2,
        sol::meta_function::division,
        divOverloadsMat2,
        "get",
        [&](glm::mat2 &m, int index) {
            if (index < 0 || index > 1) {
                LOGW << "(Lua) Index out of bounds trying to get vec4 from mat4";
                return glm::vec2{};
            }
            return glm::vec2{m[index]};
        },
        "set",
        [&](glm::mat2 &m, int index, const glm::vec2 &v) {
            if (index < 0 || index > 1) {
                LOGW << "(Lua) Index out of bounds trying to set vec4 on mat4";
                return;
            }
            m[index] = v;
        });

    auto toStrOverloadsMat3 = sol::overload([](const glm::mat3 &m) -> std::string {
        return "mat3=\n[" + std::to_string(m[0].x) + ", " + std::to_string(m[0].y) + ", " + std::to_string(m[0].z) +
               +"]\n" + "[" + std::to_string(m[1].x) + ", " + std::to_string(m[1].y) + ", " + std::to_string(m[1].z) +
               "]\n" + "[" + std::to_string(m[2].x) + ", " + std::to_string(m[2].y) + ", " + std::to_string(m[2].z) +
               "]";
    });

    auto multOverloadsMat3 =
        sol::overload([](const glm::mat3 &m1, const glm::mat3 &m2) -> glm::mat3 { return m1 * m2; },
                      [](const glm::mat3 &m1, float f) -> glm::mat3 { return m1 * f; },
                      [](float f, const glm::mat3 &m1) -> glm::mat3 { return f * m1; });

    auto addOverloadsMat3 = sol::overload([](const glm::mat3 &m1, const glm::mat3 &m2) -> glm::mat3 { return m1 + m2; },
                                          [](const glm::mat3 &m1, float f) -> glm::mat3 { return m1 + f; },
                                          [](float f, const glm::mat3 &m1) -> glm::mat3 { return f + m1; });

    auto subtractOverloadsMat3 =
        sol::overload([](const glm::mat3 &m1, const glm::mat3 &m2) -> glm::mat3 { return m1 - m2; },
                      [](const glm::mat3 &m1, float f) -> glm::mat3 { return m1 - f; },
                      [](float f, const glm::mat3 &m1) -> glm::mat3 { return f - m1; });

    auto divOverloadsMat3 = sol::overload([](const glm::mat3 &m1, const glm::mat3 &m2) -> glm::mat3 { return m1 / m2; },
                                          [](const glm::mat3 &m1, float f) -> glm::mat3 { return m1 / f; },
                                          [](float f, const glm::mat3 &m1) -> glm::mat3 { return f / m1; });

    auto mat3 = lua.new_usertype<glm::mat3>(
        "mat3",
        sol::constructors<glm::mat3(), glm::mat3(float), glm::mat3(glm::mat3), glm::mat3(glm::mat2)>(),
        sol::meta_function::to_string,
        toStrOverloadsMat3,
        sol::meta_function::multiplication,
        multOverloadsMat3,
        sol::meta_function::addition,
        addOverloadsMat3,
        sol::meta_function::subtraction,
        subtractOverloadsMat3,
        sol::meta_function::division,
        divOverloadsMat3,
        "get",
        [&](glm::mat3 &m, int index) {
            if (index < 0 || index > 2) {
                LOGW << "(Lua) Index out of bounds trying to get vec4 from mat4";
                return glm::vec3{};
            }
            return glm::vec3{m[index]};
        },
        "set",
        [&](glm::mat3 &m, int index, const glm::vec3 &v) {
            if (index < 0 || index > 2) {
                LOGW << "(Lua) Index out of bounds trying to set vec4 on mat4";
                return;
            }
            m[index] = v;
        });

    auto toStrOverloadsMat4 = sol::overload([](const glm::mat4 &m) -> std::string {
        return "mat4=\n[" + std::to_string(m[0].x) + ", " + std::to_string(m[0].y) + ", " + std::to_string(m[0].z) +
               ", " + std::to_string(m[0].w) + "]\n" + "[" + std::to_string(m[1].x) + ", " + std::to_string(m[1].y) +
               ", " + std::to_string(m[1].z) + ", " + std::to_string(m[1].w) + "]\n" + "[" + std::to_string(m[2].x) +
               ", " + std::to_string(m[2].y) + ", " + std::to_string(m[2].z) + ", " + std::to_string(m[2].w) + "]\n" +
               "[" + std::to_string(m[3].x) + ", " + std::to_string(m[3].y) + ", " + std::to_string(m[3].z) + ", " +
               std::to_string(m[3].w) + "]";
    });

    auto multOverloadsMat4 =
        sol::overload([](const glm::mat4 &m1, const glm::mat4 &m2) -> glm::mat4 { return m1 * m2; },
                      [](const glm::mat4 &m1, float f) -> glm::mat4 { return m1 * f; },
                      [](float f, const glm::mat4 &m1) -> glm::mat4 { return f * m1; });

    auto addOverloadsMat4 = sol::overload([](const glm::mat4 &m1, const glm::mat4 &m2) -> glm::mat4 { return m1 + m2; },
                                          [](const glm::mat4 &m1, float f) -> glm::mat4 { return m1 + f; },
                                          [](float f, const glm::mat4 &m1) -> glm::mat4 { return f + m1; });

    auto subtractOverloadsMat4 =
        sol::overload([](const glm::mat4 &m1, const glm::mat4 &m2) -> glm::mat4 { return m1 - m2; },
                      [](const glm::mat4 &m1, float f) -> glm::mat4 { return m1 - f; },
                      [](float f, const glm::mat4 &m1) -> glm::mat4 { return f - m1; });

    auto divOverloadsMat4 = sol::overload([](const glm::mat4 &m1, const glm::mat4 &m2) -> glm::mat4 { return m1 / m2; },
                                          [](const glm::mat4 &m1, float f) -> glm::mat4 { return m1 / f; },
                                          [](float f, const glm::mat4 &m1) -> glm::mat4 { return f / m1; });

    auto mat4 = lua.new_usertype<glm::mat4>(
        "mat4",
        sol::constructors<glm::mat4(), glm::mat4(float), glm::mat4(glm::mat4), glm::mat4(glm::mat3)>(),
        sol::meta_function::to_string,
        toStrOverloadsMat4,
        sol::meta_function::multiplication,
        multOverloadsMat4,
        sol::meta_function::addition,
        addOverloadsMat4,
        sol::meta_function::subtraction,
        subtractOverloadsMat4,
        sol::meta_function::division,
        divOverloadsMat4,
        "get",
        [&](glm::mat4 &m, int index) {
            if (index < 0 || index > 3) {
                LOGW << "(Lua) Index out of bounds trying to get vec4 from mat4";
                return glm::vec4{};
            }
            return glm::vec4{m[index]};
        },
        "set",
        [&](glm::mat4 &m, int index, const glm::vec4 &v) {
            if (index < 0 || index > 3) {
                LOGW << "(Lua) Index out of bounds trying to set vec4 on mat4";
                return;
            }
            m[index] = v;
        });

    mat2.set_function("inverse", [](const glm::mat2 &m) { return glm::inverse(m); });
    mat3.set_function("inverse", [](const glm::mat3 &m) { return glm::inverse(m); });
    mat4.set_function("inverse", [](const glm::mat4 &m) { return glm::inverse(m); });

    mat2.set_function("transpose", [](const glm::mat2 &m) { return glm::transpose(m); });
    mat3.set_function("transpose", [](const glm::mat3 &m) { return glm::transpose(m); });
    mat4.set_function("transpose", [](const glm::mat4 &m) { return glm::transpose(m); });

    mat4.set_function("translate", [](const glm::mat4 &m, glm::vec3 &v) { return glm::translate(m, v); });
    mat4.set_function("scale", [](const glm::mat4 &m, glm::vec3 &v) { return glm::scale(m, v); });
    mat4.set_function("rotate", [](const glm::mat4 &m, float a, glm::vec3 &v) { return glm::rotate(m, a, v); });
}

sol::state &
jleLuaEnvironment::getState()
{
    return _luaState;
}

void
jleLuaEnvironment::loadScript(const jlePath &path)
{
    // Loads script and it will be placed in resource holder
    auto script = jleResourceRef<jleLuaScript>(path);
    _loadedScripts.insert(std::make_pair(path, script.get()));
}

std::unordered_map<jlePath, std::shared_ptr<jleLuaScript>> &
jleLuaEnvironment::loadedScripts()
{
    return _loadedScripts;
}
