// Copyright (c) 2022. Johan Lind

#ifndef HEXABLO_OCHARACTER_H
#define HEXABLO_OCHARACTER_H

#include "jleObject.h"
#include "cTransform.h"
#include "cSprite.h"
#include "cAseprite.h"

#include <memory>
#include <iostream>

class oCharacter : public jle::jleObject {
    JLE_REGISTER_OBJECT_TYPE(oCharacter)
public:

    enum class oCharacterDirection {
        west,
        northwest,
        north,
        northeast,
        east,
        southeast,
        south,
        southwest
    };

    void SetCharacterDirection(oCharacterDirection direction);

    void SetHexagonPlacementTeleport(int q, int r);

    void SetHexagonPlacementInterp(int q, int r);

    void SetupDefaultObject() override;

    void Start() override;

    void Update(float dt) override;

    void ToJson(nlohmann::json &j_out) override;

    void FromJson(const nlohmann::json &j_in) override;

    std::shared_ptr<cTransform> mTransform{nullptr};
    std::shared_ptr<jle::cAseprite> mAseprite{nullptr};

protected:

    int mHexagonQ{10}, mHexagonR{10}, mHexagonPixelX{}, mHexagonPixelY{};

    float mInterpingX{}, mInterpingY{};
    float mInterpingAlpha{0.f};
    bool mInterpingPosition{false};
    float mInterpBetweenHexasSpeed = 1.f;

    oCharacterDirection mCharacterDirection{};

    int mWestTextureX{};
    int mWestTextureY{};

    int mNorthwestTextureX{32};
    int mNorthwestTextureY{};

    int mNorthTextureX{64};
    int mNorthTextureY{};

    int mNortheastTextureX{96};
    int mNortheastTextureY{};

    int mEastTextureX{128};
    int mEastTextureY{};

    int mSoutheastTextureX{160};
    int mSoutheastTextureY{};

    int mSouthTextureX{192};
    int mSouthTextureY{};

    int mSouthwestTextureX{224};
    int mSouthwestTextureY{};

};


#endif //HEXABLO_OCHARACTER_H
