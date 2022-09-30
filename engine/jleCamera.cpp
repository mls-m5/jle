// Copyright (c) 2022. Johan Lind
#include "jleCamera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <plog/Log.h>

jleCamera::jleCamera(jleCameraProjection projection) {
    mProjectionType = projection;

    if (projection == jleCameraProjection::Orthographic) {
        SetOrthographicProjection(512, 512, 1000.f, -1000.f);
    }
    else {
        SetPerspectiveProjection(90.f, 512, 512, 1000.f, -1000.f);
    }
}

void jleCamera::SetPerspectiveProjection(float fov,
                                         uint32_t screenWidth,
                                         uint32_t screenHeight,
                                         float farPlane,
                                         float nearPlane) {
    mProjectionMatrix = glm::perspective(
        fov, (float)screenWidth / (float)screenHeight, nearPlane, farPlane);
}

void jleCamera::SetOrthographicProjection(uint32_t screenWidth,
                                          uint32_t screenHeight,
                                          float farPlane,
                                          float nearPlane) {
    mProjectionMatrix = glm::ortho(static_cast<float>(0.f),
                                   static_cast<float>(screenWidth),
                                   static_cast<float>(screenHeight),
                                   static_cast<float>(0.f),
                                   nearPlane,
                                   farPlane);
}

glm::mat4 jleCamera::GetProjectionViewMatrix() const {
    switch (mProjectionType) {
    case jleCameraProjection::Perspective: {
        const glm::mat4 view = glm::lookAt(mPosition, mPosition + mFront, mUp);
        return mProjectionMatrix * view;
    }
    case jleCameraProjection::Orthographic: {
        glm::mat4 view{1.f};
        view = glm::translate(view, mPosition);
        // view = glm::rotate(view, glm::radians(mCameraRotationDegrees),
        // glm::vec3{}) view = glm::scale(view, glm::vec3{1.f});
        // TODO: Support camera rotation, zooming?
        return mProjectionMatrix * view;
    }
    }

    return glm::identity<glm::mat4>();
}

void jleCamera::SetPerspectiveMouseSensitivity(float sensitivity) {
    mMouseSensitivity = sensitivity;
}

void jleCamera::ApplyPerspectiveMouseMovementDelta(glm::vec2 delta) {
    delta *= mMouseSensitivity;
    mYaw = glm::mod(mYaw + delta.x, 360.0f);
    mPitch += delta.y;

    mPitch = glm::clamp(mPitch, -89.f, 89.f);

    CalculatePerspectiveVectors();
}

// TODO:
// https://gitlab.com/muffinman007/OpenGL_360_Camera_Quarternion/blob/master/Camera.h

void jleCamera::CalculatePerspectiveVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    front.y = sin(glm::radians(mPitch));
    front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    mFront = glm::normalize(front);
    mRight = glm::normalize(glm::cross(mFront, mWorldUp));
    mUp = glm::normalize(glm::cross(mRight, mFront));

    if (mPitch > 90.f || mPitch < 270.f) {
        mRight *= -1;
    }
}

void jleCamera::MovePerspectiveForward(float speed) {
    mPosition += mFront * speed;
}

void jleCamera::MovePerspectiveBackward(float speed) {
    mPosition -= mFront * speed;
}

void jleCamera::MovePerspectiveRight(float speed) {
    mPosition += mRight * speed;
}

void jleCamera::MovePerspectiveLeft(float speed) {
    mPosition -= mRight * speed;
}

void jleCamera::MovePerspectiveUp(float speed) { mPosition += mUp * speed; }

void jleCamera::MovePerspectiveDown(float speed) { mPosition -= mUp * speed; }
