if is_os("windows") then
    set_config("arch", "x86")
    add_cxflags("/EHsc", {force = true})
    if(is_mode("release")) then
        set_config("vs_runtime", "MD")
        add_cxflags("-MD", {force = true})
        set_installdir("build/windows/x86/release")
    else
        set_config("vs_runtime", "MDd")
        add_cxflags("-MDd", {force = true})
        set_installdir("build/windows/x86/debug")
    end
else
    add_cxflags("-MD", {force = true})
end
add_requires("vcpkg::czmq", "vcpkg::zeromq", {configs={runtime_link="shared", shared=true, debug=true}})
-- target("zmq")
--     set_kind("binary")
--     set_symbols("debug")
    
--     add_files("zmq/*.cpp")
--     add_packages("vcpkg::czmq", "vcpkg::zeromq")
--     after_build(function(target)
--         import("target.action.install")(target)
--         print("echo" .. "$(buildir)/$(plat)/$(arch)/$(mode)")
--         --windows
--         os.cp("$(buildir)/$(plat)/$(arch)/$(mode)/bin/*.dll", "$(buildir)/$(plat)/$(arch)/$(mode)")
--     end)

target("asio")    
    set_kind("binary")
    set_symbols("debug")
    add_includedirs("asio/Utils/")
    add_headerfiles("asio/*.hpp", "asio/*.h")
    add_files("asio/*.cpp")
    add_packages("vcpkg::czmq", "vcpkg::zeromq")
    add_cxxflags("/EHsc", {force = true})
    add_cxflags("-Od", {force = true})
    after_build(function(target)
        import("target.action.install")(target)
        print("echo" .. "$(buildir)/$(plat)/$(arch)/$(mode)")
        --windows
        os.cp("$(buildir)/$(plat)/$(arch)/$(mode)/bin/*.dll", "$(buildir)/$(plat)/$(arch)/$(mode)")
    end)