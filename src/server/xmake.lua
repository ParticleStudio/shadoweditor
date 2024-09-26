set_project("server")

-- version
set_version("0.0.1", { build = "%Y%m%d%H%M" })

-- set xmake min version
set_xmakever("2.9.3")

-- set warning all as error
--set_warnings("all", "error")

set_languages("c99", "c++20")

add_rules("mode.debug", "mode.release", "mode.valgrind")

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

    add_files("src/*.cpp", "src/**/*.cpp")
    add_files("src/*.cppm", "src/**/*.cppm")

    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("server.config.cppm.in")
    add_includedirs("$(buildir)/$(plat)/$(arch)/$(mode)/server.*.cppm", { public = true })

    if is_plat("windows") then
        add_defines("WIN64", "_WIN64", "_WIN32_WINNT=0x0601")
    end

    add_defines(
            "COMMON_SHARED_LIB",
            "LOGGER_SHARED_LIB",
            "IPC_SHARED_LIB",
            "JSENGINE_SHARED_LIB",
            "NETWORK_SHARED_LIB",
            "BEHAVIORTREE_SHARED_LIB"
    )

    add_deps("common", { configs = { shared = true } })
    add_deps("logger", { configs = { static = true } })
    --add_deps("ipc", { configs = { shared = true } })
    add_deps("jsengine", { configs = { shared = true } })
    add_deps("network", { configs = { shared = true } })

    after_build(function(target)

    end)
end)
