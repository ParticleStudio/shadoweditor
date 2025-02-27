set_project("behaviortree")

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

add_requires("lexy")
add_requires("nlohmann_json")
add_requires("magic_enum")
add_requires("conan::minicoro/0.1.3", { alias = "minicoro" })

target("behaviortree", function()
    set_kind("$(kind)")

    if is_plat("windows") then
        add_defines("WIN32", "_WIN32")
    end

    if is_kind("shared") then
        add_defines("BEHAVIORTREE_SHARED_LIB", "BEHAVIORTREE_EXPORT", { public = true })
    end

    add_packages("lexy")
    add_packages("nlohmann_json", { public = true })
    add_packages("magic_enum", { public = true })
    add_packages("minicoro")

    add_includedirs("include", { public = true })
    add_headerfiles("behaviortree/(*.h)", "behaviortree/(*.hpp)")
    add_headerfiles("lexy/(*.h)", "lexy/(*.hpp)", "lexy/**/(*.hpp)", "lexy_ext/(*.hpp)")
    add_headerfiles("nlohmann/(*.h)", "nlohmann/(*.hpp)", "nlohmann/**/(*.h)", "nlohmann/**/(*.hpp)")

    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("behaviortree.config.cppm.in")
    add_files("$(buildir)/$(plat)/$(arch)/$(mode)/behaviortree.*.cppm", { public = true })

    add_files("src/*.cpp", "src/**/*.cpp")
    add_files("src/*.cppm", "src/**/*.cppm", { public = true })

    add_defines("LEXY_HAS_UNICODE_DATABASE")

    add_deps("common", { configs = { shared = true } })

    after_build(function(target)
        --local outdir = "$(buildir)/$(plat)/$(arch)/$(mode)"
        --os.cp("conf", outdir)
        --os.cp("script", outdir)
    end)
end)
