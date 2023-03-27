// Copyright (c) 2023. Johan Lind

#ifndef JLE_OBJECT
#define JLE_OBJECT

#include <memory>
#include <optional>
#include <vector>

#include "jleTypeReflectionUtils.h"

#include "jleJson.h"
#include "jlePath.h"
#include "jleTransform.h"
#include "json.hpp"

//#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>

class jleScene;

class jleObject : public jleJsonInterface<nlohmann::json>, public std::enable_shared_from_this<jleObject>
{
    JLE_REGISTER_OBJECT_TYPE(jleObject)
public:
    std::string _instanceName;

    virtual void
    start()
    {
    }

    virtual void
    update(float dt)
    {
    }


    std::shared_ptr<jleObject> duplicate(bool childChain = false);

    template <class Archive>
    void
    serialize(Archive &archive);

    jleObject();

    ~jleObject() override = default;

    template <typename T>
    std::shared_ptr<T> addComponent();

    std::shared_ptr<jleComponent> addComponent(const std::string &component_name);

    template <typename T>
    std::shared_ptr<T> getComponent();

    template <typename T>
    std::shared_ptr<T> addDependencyComponent(const jleComponent *component);

    template <typename T>
    std::shared_ptr<T> spawnChildObject();

    std::shared_ptr<jleObject> spawnChildObject(const std::string &objName);

    void saveObjectTemplate(jleRelativePath &path);

    std::shared_ptr<jleObject> spawnChildObjectFromTemplate(const jleRelativePath &path);

    void injectTemplate(const nlohmann::json &json);

    // Called from components
    void destroyComponent(jleComponent *component);

    void destroyObject();

    int componentCount();

    std::vector<std::shared_ptr<jleComponent>> &customComponents();

    void attachChildObject(const std::shared_ptr<jleObject> &object);

    void detachObjectFromParent();

    std::vector<std::shared_ptr<jleObject>> &childObjects();

    void tryFindChildWithInstanceId(int instanceId, std::shared_ptr<jleObject> &outObject);

    void
    toJson(nlohmann::json &j_out) override
    {
    }

    void
    fromJson(const nlohmann::json &j_in) override
    {
    }

    jleObject *parent();

    [[nodiscard]] std::weak_ptr<jleObject> weakPtrToThis();

    static void processJsonData(const nlohmann::json &j, std::shared_ptr<jleObject> &o);

    static std::shared_ptr<jleObject> processChildJsonData(const nlohmann::json &j, std::shared_ptr<jleObject> &o);

    static nlohmann::json objectTemplateJson(const jleRelativePath &path);

    // If this object is based on a template
    std::optional<std::string> _templatePath{};

    int instanceID() const;

    jleTransform &getTransform();

private:
    friend class jleScene;

    explicit jleObject(jleScene *scene);

    void propagateOwnedByScene(jleScene* scene);

    void startComponents();

    void updateComponents(float dt);

    void updateChildren(float dt);

    bool _pendingKill = false;

    bool _isStarted = false;

    uint32_t __instanceID;

protected:
    std::vector<std::shared_ptr<jleComponent>> _components{};

    jleTransform _transform;

    std::vector<std::shared_ptr<jleObject>> __childObjects{};

    jleObject *_parentObject = nullptr;

    jleScene *_containedInScene = nullptr;

    virtual void
    setupDefaultObject()
    {
    }

    friend void to_json(nlohmann::json &j, const std::shared_ptr<jleObject> &o);

    friend void from_json(const nlohmann::json &j, std::shared_ptr<jleObject> &o);
};

void to_json(nlohmann::json &j, const std::shared_ptr<jleObject> &o);

void from_json(const nlohmann::json &j, std::shared_ptr<jleObject> &o);

#include "jleObject.inl"

#endif