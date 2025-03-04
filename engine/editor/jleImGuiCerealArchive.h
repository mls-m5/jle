// Copyright (c) 2023. Johan Lind

#ifndef JLEIMGUICEREALARCHIVE_H
#define JLEIMGUICEREALARCHIVE_H

#ifdef BUILD_EDITOR
// The following macro only works on arithmetic types: float, int, double, etc, and std::string
#define JLE_TOOLTIP_ARITHMETIC(TYPE, TIP, NAME) jleToolTip<TYPE>(NAME){TIP};
#else
#define JLE_TOOLTIP_ARITHMETIC(TYPE, TIP, NAME) TYPE NAME;
#endif

#ifdef BUILD_EDITOR

#include "cereal/cereal.hpp"
#include "cereal/details/helpers.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/polymorphic.hpp"

#include "jleExternalSerialization.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "ImGui/imgui.h"
#include "ImGui/ImGuizmo.h"

#include "jleComponent.h"
#include <ImGui/imgui_internal.h>
#include <filesystem>
#include <glm/gtx/matrix_decompose.hpp>
#include <iostream>
#include <jleResourceRef.h>
#include <jleTransform.h>

// A tooltip utility for showing tooltips in the editor
// Works on arithmetic types: float, int, double, etc, and std::string
template <typename T>
struct jleToolTip {
    explicit jleToolTip<T>(const std::string_view tip) : tip_view{tip} {}

    jleToolTip<T>(T i){item = i;}
    operator T(){return item;}

    T item{};

    template <class Archive>
    T save_minimal(
        Archive const & ) const
    {
        return item;
    }

    template <class Archive>
    void load_minimal( Archive const &,
                 T const & value )
    {
        item = value;
    }

    const std::string_view tip_view;
};


    namespace cereal
    {

    class jleImGuiCerealArchiveInternal : public OutputArchive<jleImGuiCerealArchiveInternal>
    {
    public:
        jleImGuiCerealArchiveInternal() : OutputArchive<jleImGuiCerealArchiveInternal>(this) {}
        ~jleImGuiCerealArchiveInternal() override = default;

        std::string nextPolymorhphicTypeName{};

    };

    class jleImGuiCerealArchive : public InputArchive<jleImGuiCerealArchive>
    {

    public:
        jleImGuiCerealArchive() : InputArchive<jleImGuiCerealArchive>(this) {}

        ~jleImGuiCerealArchive() override = default;

        template <class T>
        void
        draw(jleImGuiCerealArchive &ar, std::string name, T &value)
        {
            elementCount += 1;

            if (name == "") {
                name = std::to_string(elementCount);
            }

            draw_ui(ar, name.c_str(), value);
        }

    private:
        template <class T>
        void
        draw_ui(jleImGuiCerealArchive &ar, std::string name, std::vector<T> &vector)
        {
            int removeIndex = -1;

            if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                int i = 0;
                for (auto &&value : vector) {
                    name = std::to_string(i);

                    draw_ui(ar, name.c_str(), value);

                    ImGui::PushID(elementCount++);
                    ImGui::SameLine();
                    if (ImGui::Button("-")) {
                        removeIndex = i;
                    }
                    ImGui::PopID();

                    i++;
                }

                ImGui::TreePop();
            }

            if (removeIndex != -1) {
                vector.erase(vector.begin() + removeIndex);
            }

            ImGui::PushID(elementCount++);
            if (ImGui::Button("+")) {
                vector.push_back({});
            }
            ImGui::PopID();
        }

#define STD_MAPS_IMPL                                                                                                  \
    elementCount += 1;                                                                                                 \
                                                                                                                       \
    ImGui::PushID(elementCount);                                                                                       \
                                                                                                                       \
    if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {                                             \
        int e = 0;                                                                                                     \
        for (auto &kvp : map) {                                                                                        \
            ImGui::PushID(e++);                                                                                        \
                                                                                                                       \
            std::string elementName;                                                                                   \
            if constexpr (std::is_arithmetic<KEY>()) {                                                                 \
                elementName = std::to_string(kvp.first);                                                               \
            } else {                                                                                                   \
                elementName = std::to_string(e);                                                                       \
            }                                                                                                          \
                                                                                                                       \
            draw_ui(ar, elementName.c_str(), kvp.second);                                                              \
                                                                                                                       \
            ImGui::PopID();                                                                                            \
        }                                                                                                              \
                                                                                                                       \
        ImGui::TreePop();                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    ImGui::PopID();

        template <class KEY, class VALUE>
        void
        draw_ui(jleImGuiCerealArchive &ar, std::string name, std::map<KEY, VALUE> &map)
        {
            STD_MAPS_IMPL
        }

        template <class KEY, class VALUE>
        void
        draw_ui(jleImGuiCerealArchive &ar, std::string name, std::unordered_map<KEY, VALUE> &map)
        {
            STD_MAPS_IMPL
        }

        template <class T>
        void
        draw_ui(jleImGuiCerealArchive &ar, std::string name, T &value)
        {
            elementCount += 1;

            ImGui::PushID(elementCount);

            if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                ar(value);
                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        template <class T>
        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, jleToolTip<T> &value)
        {
            ImGui::PushID(elementCount++);

            draw_ui(ar, name, value.item);

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 220, 180, 255));
            ImGui::Text("   %s", value.tip_view.data());
            ImGui::PopStyleColor();

            ImGui::PopID();
        }

        template <class T>
        void
        draw_ui(jleImGuiCerealArchive &ar, std::string name, jleToolTip<T> &value)
        {
            ImGui::PushID(elementCount++);

            draw_ui(ar, name, value.item);

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 220, 180, 255));
            ImGui::Text("   %s", value.tip_view.data());
            ImGui::PopStyleColor();

            ImGui::PopID();
        }

        template <class T>
        void
        draw_ui(jleImGuiCerealArchive &ar, std::string name, const T &value)
        {

            ImGui::PushID(elementCount++);
            auto cpy = value;
            draw_ui(ar, name.c_str(), cpy);

            ImGui::PopID();
        }

        template <class T>
        void
        draw_ui(jleImGuiCerealArchive &ar, std::string name, std::shared_ptr<T> &ptr)
        {
            ImGui::PushID(elementCount++);

            jleImGuiCerealArchiveInternal archiveInternal;
            archiveInternal(ptr);

            ImGui::PopID();
        }

        bool draw_ui_reference(jleImGuiCerealArchive &ar, const char *name, std::string &value, std::vector<std::string> fileExtensions){
            ImGui::PushID(elementCount++);

            std::vector<char> charData(value.begin(), value.end());
            charData.resize(1000);

            bool isEditedAndDeactivated = false;

            if (ImGui::InputText(LeftLabelImGui(name).c_str(), &charData[0], charData.size())) {
                value = std::string(charData.data());
            }

            std::vector<std::string> directories;
            directories.push_back(jlePath{"ER:/"}.getRealPath());
            directories.push_back(jlePath{"ED:/"}.getRealPath());
            directories.push_back(jlePath{"GR:/"}.getRealPath());

            std::vector<jlePath> relevantResources;

            for (auto &dir : directories) {
                for (auto &file : std::filesystem::recursive_directory_iterator(dir)) {
                    if(file.is_regular_file()) {
                        auto ext = file.path().extension().string();
                        for(auto &fileExtension : fileExtensions) {
                            if(ext == std::string{'.' + fileExtension}) {
                                relevantResources.emplace_back(file.path().string(), false);
                                break;
                            }
                        }
                    }
                }
            }

            //bool isOpen = false;
            //static bool isOpen = false;
            static std::set<int> isOpenSet;

            bool isOpen = isOpenSet.find(elementCount) != isOpenSet.end();

            if(relevantResources.size() != 0) {

                bool isFocused = ImGui::IsItemFocused();
                isOpen |= ImGui::IsItemActive();
                if (isOpen) {
                    ImGui::SetNextWindowPos({ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y});
                    ImGui::SetNextWindowSize({ImGui::GetItemRectSize().x, 0});
                    if (ImGui::Begin(std::string{"popup_" + std::string{name}}.c_str(),
                                     &isOpen,
                                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                         ImGuiWindowFlags_Tooltip)) {
                        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
                        isFocused |= ImGui::IsWindowFocused();
                        // static const char* autocomplete[] = {"cats", "dogs", "rabbits", "turtles"};
                        for (int i = 0; i < relevantResources.size(); i++) {
                            if (strstr(relevantResources[i].getVirtualPath().c_str(), value.c_str()) == NULL)
                                continue;
                            if (ImGui::Selectable(relevantResources[i].getVirtualPath().c_str()) ||
                                (ImGui::IsItemFocused() && ImGui::IsKeyPressedMap(ImGuiKey_Enter))) {
                                value = relevantResources[i].getVirtualPath();
                                isEditedAndDeactivated = true;
                                isOpen = false;
                            }
                        }
                    }
                    ImGui::End();
                    isOpen &= isFocused;
                }
            }else {
                isOpen = false;
            }

            if(isOpen) {
                isOpenSet.insert(elementCount);
            }else {
                isOpenSet.erase(elementCount);
            }

            if(ImGui::IsItemDeactivatedAfterEdit()){
                isEditedAndDeactivated = true;
            }

            ImGui::PopID();
            return isEditedAndDeactivated;
        }

        template <class T>
        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, jleResourceRef<T> &value)
        {
            ImGui::PushID(elementCount++);

            std::string copy = value.path._virtualPath;

            static std::unique_ptr<T> dummyResource;
            if(!dummyResource) {
                dummyResource = std::make_unique<T>();
            }

            auto fileExtensionAssociated = dummyResource->getFileAssociationList();

            bool isEditedAndDeactivated = draw_ui_reference(ar, std::string{name + std::string{" (ref)"}}.c_str(), value.path._virtualPath, fileExtensionAssociated);
            if (isEditedAndDeactivated) {
                value.load_minimal(ar, value.path._virtualPath);
            }

            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, jlePath &value)
        {
            ImGui::PushID(elementCount++);

            std::string copy = value._virtualPath;
            draw_ui(ar, std::string{name + std::string{" (path)"}}.c_str(), value._virtualPath);
            if (copy != value._virtualPath) {
                // Assign value to "itself", re-creating the real path
                value = jlePath{value._virtualPath};
            }

            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, uint32_t &value)
        {
            ImGui::PushID(elementCount++);

            ImGui::DragScalar(LeftLabelImGui(name).c_str(), ImGuiDataType_U32, &value, 0.2f, nullptr, nullptr, "%u");
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, int32_t &value)
        {
            ImGui::PushID(elementCount++);

            ImGui::DragScalar(LeftLabelImGui(name).c_str(), ImGuiDataType_S32, &value, 0.2f, nullptr, nullptr, "%d");
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, uint64_t &value)
        {
            ImGui::PushID(elementCount++);

            ImGui::DragScalar(LeftLabelImGui(name).c_str(), ImGuiDataType_U64, &value, 0.2f, nullptr, nullptr, "%llu");
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, int64_t &value)
        {
            ImGui::PushID(elementCount++);

            ImGui::DragScalar(LeftLabelImGui(name).c_str(), ImGuiDataType_S64, &value, 0.2f, nullptr, nullptr, "%lld");
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, float &value)
        {
            ImGui::PushID(elementCount++);

            ImGui::DragScalar(
                LeftLabelImGui(name).c_str(), ImGuiDataType_Float, &value, 0.005f, nullptr, nullptr, "%f");
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, double &value)
        {
            ImGui::PushID(elementCount++);

            ImGui::DragScalar(
                LeftLabelImGui(name).c_str(), ImGuiDataType_Double, &value, 0.005f, nullptr, nullptr, "%f");
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, bool &value)
        {
            ImGui::PushID(elementCount++);

            ImGui::Checkbox(LeftLabelImGui(name).c_str(), &value);
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, std::string &value)
        {
            ImGui::PushID(elementCount++);

            std::vector<char> charData(value.begin(), value.end());
            charData.resize(1000);

            if (ImGui::InputText(LeftLabelImGui(name).c_str(), &charData[0], charData.size())) {
                value = std::string(charData.data());
            }
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, glm::vec2 &value)
        {
            ImGui::PushID(elementCount++);

            ImGui::DragFloat2(LeftLabelImGui(name).c_str(), &value[0]);
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, glm::vec3 &value)
        {
            ImGui::PushID(elementCount++);

            ImGui::DragFloat3(LeftLabelImGui(name).c_str(), &value[0]);
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, glm::vec4 &value)
        {
            ImGui::PushID(elementCount++);

            ImGui::DragFloat4(LeftLabelImGui(name).c_str(), &value[0]);
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, glm::quat &value)
        {
            ImGui::PushID(elementCount++);

            glm::vec3 euler = glm::eulerAngles(value);

            if (ImGui::DragFloat3(LeftLabelImGui(name).c_str(), &euler[0])) {
                value = glm::quat(euler);
            }
            ImGui::PopID();
        }

        void
        draw_ui(jleImGuiCerealArchive &ar, const char *name, jleTransform &value)
        {
            ImGui::PushID(elementCount++);

            glm::mat4 cpyMat = value.getLocalMatrix();
            auto *cpyMatPtr = &cpyMat[0][0];

            float matrixTranslation[3], matrixRotation[3], matrixScale[3];
            ImGui::Text("%s", name);
            ImGuizmo::DecomposeMatrixToComponents((float *)cpyMatPtr, matrixTranslation, matrixRotation, matrixScale);
            ImGui::DragFloat3("Translation", matrixTranslation);
            ImGui::DragFloat3("Rotation", matrixRotation);
            ImGui::DragFloat3("Scaling", matrixScale);
            ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, cpyMatPtr);

            value.setLocalMatrix(cpyMat);

            ImGui::PopID();
        }

        static std::string
        LeftLabelImGui(const char *const label)
        {
            float width = ImGui::CalcItemWidth();

            float x = ImGui::GetCursorPosX();
            ImGui::Text("%s", label);
            ImGui::SameLine();
            ImGui::SetCursorPosX(x + width * 0.3f + ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::SetNextItemWidth(-1);

            std::string labelID = "##";
            labelID += label;

            return labelID;
        }
    public:
        int elementCount = 0;
    };

    template <class T>
    inline void
    CEREAL_LOAD_FUNCTION_NAME(jleImGuiCerealArchive &ar, NameValuePair<T> &t)
    {
        std::string newName = t.name;

        if (newName.size() >= 2 && newName[0] == '_' && newName[1] == '_') {
            // Don't render out variables starting with "__".
            return;
        }

        if (newName[0] == '_') {
            newName = newName.substr(1, newName.size());
        }

        if (std::islower(newName[0])) {
            newName[0] = std::toupper(newName[0]);
        }

        ar.draw<T>(ar, newName, t.value);
    }

    template <class T, traits::EnableIf<std::is_arithmetic<T>::value> = traits::sfinae>
    inline void
    CEREAL_LOAD_FUNCTION_NAME(jleImGuiCerealArchive &ar, T &t)
    {
        ar.draw<T>(ar, "", t);
    }

    template <class T>
    inline void
    CEREAL_SAVE_FUNCTION_NAME(
        jleImGuiCerealArchiveInternal &ar,
        NameValuePair<cereal::memory_detail::PtrWrapper<const std::shared_ptr<const T> &>> const &t)
    {
        std::shared_ptr<T> f = std::const_pointer_cast<T>(t.value.ptr);

        if(auto component = std::dynamic_pointer_cast<jleComponent>(f)) {
            jleImGuiCerealArchive nonPolymorphicArchive;
            nonPolymorphicArchive.elementCount += 1;

            ImGui::PushID(nonPolymorphicArchive.elementCount);

            if (ImGui::TreeNodeEx(std::string{ar.nextPolymorhphicTypeName + " (ptr)"}.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                nonPolymorphicArchive(*f);
                component->editorInspectorImGuiRender();
                ImGui::TreePop();
            }

            ImGui::PopID();
        }else {
            jleImGuiCerealArchive nonPolymorphicArchive;
            nonPolymorphicArchive.draw(nonPolymorphicArchive, ar.nextPolymorhphicTypeName + " (ptr)", *f.get());
        }
    }

    template <class T>
    inline void
    CEREAL_SAVE_FUNCTION_NAME(jleImGuiCerealArchiveInternal &ar, NameValuePair<T> const &t)
    {
        if constexpr (std::is_same_v<std::string &, T>) {
            ar.nextPolymorhphicTypeName = t.value;
        }
    }

    template <class T, traits::EnableIf<std::is_arithmetic<T>::value> = traits::sfinae>
    inline void
    CEREAL_SAVE_FUNCTION_NAME(jleImGuiCerealArchiveInternal &ar, T &t)
    {
        // Do nothing
    }

    } // namespace cereal

    CEREAL_REGISTER_ARCHIVE(cereal::jleImGuiCerealArchiveInternal)
    CEREAL_REGISTER_ARCHIVE(cereal::jleImGuiCerealArchive)

#endif // BUILD_EDITOR

#endif // JLEIMGUICEREALARCHIVE_H
