set_project("client")

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

target("client", function()
    set_kind("binary")

    add_includedirs("include")
    add_headerfiles("include/*.hpp", "include/**/*.hpp")

    add_files("src/*.cpp", "src/**/*.cpp")
    add_files("src/*.cppm", "src/**/*.cppm")

    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("client.config.cppm.in")
    add_includedirs("$(buildir)/$(plat)/$(arch)/$(mode)/client.*.cppm", { public = true })

    if is_plat("windows") then
        add_defines("WIN64", "_WIN64")
    end

    add_deps("common", { configs = { shared = true } })
    add_deps("logger", { configs = { static = true } })
    add_deps("network", { configs = { shared = true } })
    add_deps("jsengine", { configs = { shared = true } })

    after_build(function(target)

    end)
end)
