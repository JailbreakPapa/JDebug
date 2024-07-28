# Make sure this project is built when the Editor is built
ns_add_as_runtime_dependency(ShaderCompilerDXC)

ns_add_dependency("ShaderCompiler" "ShaderCompilerDXC")