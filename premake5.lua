newoption
{
	trigger = "wx-version",
	value = "VERSION",
	description = "wxWidgets MSVC library version suffix, such as 33 for wxmsw33u_core or 32 for wxmsw32u_core."
}

local wxVersion = _OPTIONS["wx-version"] or "33"

workspace "Apricity"
	location "Build/ProjectFiles"
	startproject "Apricity"
	architecture "x64"
	configurations { "Debug", "Release" }
	platforms { "x64" }

project "Apricity"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++20"
	characterset "Unicode"
	staticruntime "Off"

	filter "system:windows"
		systemversion "latest"

	filter {}

	targetdir "Build/%{cfg.buildcfg}/Output"
	objdir "Build/%{cfg.buildcfg}/Intermediates/%{prj.name}"

	files
	{
		"Apricity/**.cpp",
		"Apricity/**.h",
		"Apricity/**.rc"
	}

	includedirs
	{
		"Apricity"
	}

	defines
	{
		"_WINDOWS",
		"_CRT_SECURE_NO_WARNINGS",
		"WXUSINGDLL"
	}

	links
	{
		"sqlite3"
	}

	filter "configurations:Debug"
		defines
		{
			"_DEBUG"
		}
		symbols "On"
		links
		{
			"wxmsw" .. wxVersion .. "ud_stc",
			"wxmsw" .. wxVersion .. "ud_core",
			"wxbase" .. wxVersion .. "ud"
		}

	filter "configurations:Release"
		defines
		{
			"NDEBUG"
		}
		optimize "On"
		links
		{
			"wxmsw" .. wxVersion .. "u_stc",
			"wxmsw" .. wxVersion .. "u_core",
			"wxbase" .. wxVersion .. "u"
		}
