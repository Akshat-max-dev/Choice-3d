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
Dependency["compressonator"] = "%{wks.location}/vendor/compressonator"
Dependency["stb_image"] = "%{wks.location}/vendor/stb_image"
Dependency["gli"] = "%{wks.location}/vendor/gli"
Dependency["tinyexr"] = "%{wks.location}/vendor/tinyexr"
Dependency["filesystem"] = "%{wks.location}/vendor/filesystem"

include "Choice"
include "Quatation"