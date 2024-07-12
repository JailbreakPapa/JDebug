set(BACKEND_COMMON_HDR_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Backend.h
)

set(Win32_GL2_SRC_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_Win32.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL2.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Backend_Win32_GL2.cpp
)
set(Win32_GL2_HDR_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_Win32.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL2.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Include_Windows.h
)

set(Win32_VK_SRC_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_Win32.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_VK.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Backend_Win32_VK.cpp
)
set(Win32_VK_HDR_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_Win32.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_VK.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Vulkan/ShadersCompiledSPV.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Vulkan/vk_mem_alloc.h
)


set(SDL_GL2_SRC_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_SDL.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL2.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Backend_SDL_GL2.cpp
)
set(SDL_GL2_HDR_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_SDL.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL2.h
)

set(SDL_GL3_SRC_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_SDL.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL3.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Backend_SDL_GL3.cpp
)
set(SDL_GL3_HDR_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_SDL.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL3.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Include_GL3.h
)

set(SDL_VK_SRC_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_SDL.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_VK.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Backend_SDL_VK.cpp
)
set(SDL_VK_HDR_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_SDL.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_VK.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Vulkan/ShadersCompiledSPV.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Vulkan/vk_mem_alloc.h
)

set(SDL_SDLrenderer_SRC_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_SDL.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_SDL.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Backend_SDL_SDLrenderer.cpp
)
set(SDL_SDLrenderer_HDR_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_SDL.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_SDL.h
)

set(SFML_GL2_SRC_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_SFML.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL2.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Backend_SFML_GL2.cpp
)
set(SFML_GL2_HDR_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_SFML.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL2.h
)

set(GLFW_GL2_SRC_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_GLFW.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL2.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Backend_GLFW_GL2.cpp
)
set(GLFW_GL2_HDR_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_GLFW.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL2.h
)

set(GLFW_GL3_SRC_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_GLFW.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL3.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Backend_GLFW_GL3.cpp
)
set(GLFW_GL3_HDR_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_GLFW.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_GL3.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Include_GL3.h
)

set(GLFW_VK_SRC_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_GLFW.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_VK.cpp
	${PROJECT_SOURCE_DIR}/Backends/APUI_Backend_GLFW_VK.cpp
)
set(GLFW_VK_HDR_FILES
	${PROJECT_SOURCE_DIR}/Backends/APUI_Platform_GLFW.h
	${PROJECT_SOURCE_DIR}/Backends/APUI_Renderer_VK.h
)
