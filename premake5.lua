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

include "Choice"
include "Quatation"