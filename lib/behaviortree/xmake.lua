set_project("behaviortree")

-- version
set_version("0.0.1", { build = "%Y%m%d%H%M" })

-- set xmake min version
set_xmakever("2.9.3")

-- set warning all as error
--set_warnings("all", "error")

set_languages("c99", "c++20")

add_rules("mode.debug", "mode.release")

if is_mode("release") then
    set_optimize("smallest")
    if is_plat("windows") then
        add_ldflags("/LTCG")
    end
end

add_requires("lexy")
add_requires("nlohmann_json")
add_requires("magic_enum", { configs = { prefixdir = "magic_enum" } })
add_requires("conan::minicoro/0.1.3", { alias = "minicoro" })

target("behaviortree", function()
    set_kind("shared")

    add_packages("lexy")
    add_packages("nlohmann_json", { public = true })
    add_packages("magic_enum", { public = true })
    add_packages("minicoro")

    add_includedirs("include", { public = true })
    add_headerfiles("behaviortree/(*.h)", "behaviortree/(*.hpp)")
    add_headerfiles("lexy/(*.h)", "lexy/(*.hpp)", "lexy/**/(*.hpp)", "lexy_ext/(*.hpp)")
    add_headerfiles("nlohmann/(*.h)", "nlohmann/(*.hpp)", "nlohmann/**/(*.h)", "nlohmann/**/(*.hpp)")

    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("behaviortree.config.h.in")
    add_includedirs("$(buildir)/$(plat)/$(arch)/$(mode)", { public = true })

    add_files("src/*.cpp", "src/*.cppm", "src/**/*.cpp", "src/**/*.cppm")

    add_defines("LEXY_HAS_UNICODE_DATABASE")
    add_defines("SHARED_LIB")
    if is_plat("windows") then
        add_defines("WIN32", "_WIN32", "DLLEXPORT")

        if is_kind("shared") then
            add_rules("utils.symbols.export_all", { export_classes = true })
        end
    end

    after_build(function(target)
        --local outdir = "$(buildir)/$(plat)/$(arch)/$(mode)"
        --os.cp("conf", outdir)
        --os.cp("script", outdir)
    end)
end)
