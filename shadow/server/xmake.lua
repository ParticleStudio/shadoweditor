set_project("server")

-- version
set_version("0.0.1", { build = "%Y%m%d%H%M" })

-- set xmake min version
set_xmakever("2.9.3")

-- set warning all as error
--set_warnings("all", "error")

set_languages("c17", "cxx20") -- https://xmake.io/#/zh-cn/manual/project_target?id=targetset_languages

add_rules("mode.debug", "mode.release", "mode.valgrind")

set_encodings("utf-8")

if is_mode("release") then
    --set_optimize("smallest")
    if is_plat("windows") then
        add_ldflags("/LTCG")
    end
end

add_requires("nlohmann_json")
target("server", function()
    set_kind("binary")

    if is_plat("windows") then
        add_defines("WIN64", "_WIN64", "_WIN32_WINNT=0x0601")
    end

    --add_defines(
    --        "COMMON_SHARED_LIB",
    --        "LOGGER_SHARED_LIB",
    --        "IPC_SHARED_LIB",
    --        "JSENGINE_SHARED_LIB",
    --        "NETWORK_SHARED_LIB",
    --        "BEHAVIORTREE_SHARED_LIB"
    --)

    add_packages("nlohmann_json", { public = true })

    add_deps("common")
    add_deps("logger")
    --add_deps("ipc")
    add_deps("jsengine")
    add_deps("network")

    add_includedirs("include")
    add_headerfiles("include/*.hpp", "include/**/*.hpp")

    add_files("src/*.cpp", "src/**/*.cpp")
    add_files("src/*.cppm", "src/**/*.cppm")

    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("server.config.cppm.in")
    add_includedirs("$(buildir)/$(plat)/$(arch)/$(mode)/server.*.cppm", { public = true })

    after_build(function(target)

    end)
end)

add_requires("doctest")
target("server_test", function()
    set_kind("binary")

    add_packages("doctest")

    add_files("test/*.cpp", "test/**/*.cpp")
end)
