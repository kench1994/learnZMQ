if is_os("windows") then
    set_config("arch", "x86")
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
add_requires("vcpkg::cppzmq", "vcpkg::czmq", "vcpkg::zeromq", {configs={runtime_link="shared", shared=true, debug=true}})
target("asyncCS")
    set_kind("binary")
    set_symbols("debug")
    
    add_files("*.cpp")
    add_packages("vcpkg::cppzmq", "vcpkg::czmq", "vcpkg::zeromq")
    after_build(function(target)
        import("target.action.install")(target)
        print("echo" .. "$(buildir)/$(plat)/$(arch)/$(mode)")
        --windows
        os.cp("$(buildir)/$(plat)/$(arch)/$(mode)/bin/*.dll", "$(buildir)/$(plat)/$(arch)/$(mode)")
    end)