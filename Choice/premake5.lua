project "Choice"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    systemversion "latest"

    targetdir(bindir)
    objdir(objbindir)
    debugdir "%{wks.location}"

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "src",
        "%{wks.location}/Quatation/src",
        "%{Dependency.glfw}/include",
        "%{Dependency.glad}/include",
        "%{Dependency.glm}",
        "%{Dependency.cgltf}"
    }

    links
    {
        "Quatation"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }

    filter "system:windows"
        defines "EXE"
    
    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"
