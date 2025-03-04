// Copyright (c) 2023. Johan Lind

#pragma once

#ifdef BUILD_EDITOR

#include <string>

#include "jleGameEngine.h"

#include <memory>
#include <utility>
#include <vector>

class iEditorImGuiWindow {
public:
    explicit iEditorImGuiWindow(std::string window_name)
        : window_name{std::move(window_name)} {}

    virtual ~iEditorImGuiWindow() = default;

    virtual void update(jleGameEngine &ge) = 0;

    virtual void openWindow() { isOpened = true; }

    virtual void closeWindow() { isOpened = false; }

    bool opened() const { return isOpened; }

    const std::string &windowName() { return window_name; }

protected:
    std::string window_name;
    bool isOpened = true;
};

#endif // BUILD_EDITOR
