// Copyright (c) 2023. Johan Lind

#include "jleEditorGameControllerWidget.h"

#include "jleResource.h"

#include "ImGui/imgui.h"

#include "plog/Log.h"

jleEditorGameControllerWidget::jleEditorGameControllerWidget(
    const std::string &window_name)
    : iEditorImGuiWindow{window_name} {
    _playGameIcon = gCore->resources().loadResourceFromFile<jleTexture>(jleRelativePath{"ED:/icons/play.png"});
    _restartGameIcon =
        gCore->resources().loadResourceFromFile<jleTexture>(jleRelativePath{"ED:/icons/replay.png"});
    _pauseGameIcon =
        gCore->resources().loadResourceFromFile<jleTexture>(jleRelativePath{"ED:/icons/pause.png"});
    _nextFrameIcon =
        gCore->resources().loadResourceFromFile<jleTexture>(jleRelativePath{"ED:/icons/next_frame.png"});
}

void jleEditorGameControllerWidget::update(jleGameEngine &ge) {

    const ImVec2 iconSize{ImGui::GetWindowHeight() - 3,
                          ImGui::GetWindowHeight() - 3};

    if (ge.isGameKilled()) {
        if (ImGui::Button(("start Game"))) {
            LOG_VERBOSE << "Starting the game.";
            ge.startGame();
        }
    }
    else {
        if (ImGui::Button(("Restart Game"))) {
            LOG_VERBOSE << "Restarting the game.";
            ge.restartGame();
        }
    }

    ImGui::SameLine();

    if (!ge.isGameHalted()) {
        if (ImGui::Button(("Pause Game"))) {
            LOG_VERBOSE << "Pausing the game.";
            ge.haltGame();
        }
    }
    else {
        if (ImGui::Button(("Continue Game"))) {
            LOG_VERBOSE << "Continue playing the game.";
            ge.unhaltGame();
        }
        ImGui::SameLine();
        if (ImGui::Button(("Skip Frame"))) {
            LOG_VERBOSE << "Going one frame forward.";
            if (!ge.isGameKilled()) {
                ge.executeNextFrame();
            }
        }
    }
}
