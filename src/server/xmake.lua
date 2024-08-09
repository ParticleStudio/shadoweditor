set_project("server")

-- version
set_version("0.0.1", { build = "%Y%m%d%H%M" })

-- set xmake min version
set_xmakever("2.9.3")

-- set warning all as error
--set_warnings("all", "error")

set_languages("c99", "c++20")

add_rules("mode.debug", "mode.release")

if is_mode("release") then
    --set_optimize("smallest")
    if is_plat("windows") then
        add_ldflags("/LTCG")
    end
end

target("server", function()
    set_kind("binary")

    add_includedirs("include")
    add_headerfiles("include/*.hpp", "include/**/*.hpp")

    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("server.config.h.in")
    add_includedirs("$(buildir)/$(plat)/$(arch)/$(mode)", { public = true })

    add_files("src/*.cpp", "src/*.cppm", "src/**/*.cpp", "src/**/*.cppm")

    if is_plat("windows") then
        add_defines("WIN64", "_WIN64", "_WIN32_WINNT=0x0601")
    end

    add_deps("common", { configs = { shared = true } })
    add_deps("logger", { configs = { static = true } })
    add_deps("net", { configs = { shared = true } })
    add_deps("jsengine", { configs = { shared = true } })

    after_build(function(target)

    end)
end)
