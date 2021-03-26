workspace "Choice"
    architecture "x64"
    startproject "Choice"

    configurations
    {
        "Debug",
        "Release"
    }

bindir = "%{wks.location}/bin/%{cfg.buildcfg}"
objbindir = "%{wks.location}/bin/%{cfg.buildcfg}-obj"

Dependency = {}
Dependency["glfw"] = "%{wks.location}/vendor/glfw"
Dependency["glad"] = "%{wks.location}/vendor/glad"
Dependency["glm"] = "%{wks.location}/vendor/glm"
Dependency["cgltf"] = "%{wks.location}/vendor/cgltf"
Dependency["imgui"] = "%{wks.location}/vendor/imgui"
Dependency["ImGuiFileDialog"] = "%{wks.location}/vendor/ImGuiFileDialog"

include "Choice"
include "Quatation"