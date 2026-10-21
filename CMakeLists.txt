cmake_minimum_required(VERSION 3.16)
project(wpengine-macos CXX C OBJCXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing -Wno-deprecated-declarations")
find_package(PkgConfig REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Freetype REQUIRED)
pkg_check_modules(LZ4 REQUIRED liblz4)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(MPV REQUIRED mpv)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale libswresample)
find_library(OPENGL_FW OpenGL)
find_library(COCOA_FW Cocoa)
find_library(IOKIT_FW IOKit)

set(ENABLE_OPT OFF)
set(SPIRV_CROSS_FORCE_PIC ON)
set(KISSFFT_TEST OFF CACHE BOOL "" FORCE)
set(KISSFFT_TOOLS OFF CACHE BOOL "" FORCE)
set(KISSFFT_STATIC ON CACHE BOOL "" FORCE)
set(QJS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(src/External/glslang-WallpaperEngine glslang)
add_subdirectory(src/External/SPIRV-Cross-WallpaperEngine spirv-cross)
add_subdirectory(src/External/kissfft kissfft)
add_subdirectory(src/External/argparse argparse)
add_subdirectory(src/External/quickjs quickjs)

# embed scene-script builtins
set(WPENGINE_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(WPENGINE_BUILTINS_JS "${CMAKE_CURRENT_SOURCE_DIR}/src/WallpaperEngine/Scripting/resources/builtins.js")
file(READ "${WPENGINE_BUILTINS_JS}" WPENGINE_SCRIPT_BUILTINS)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/WallpaperEngine/Scripting/resources/Builtins.generated.h.in"
    "${WPENGINE_GENERATED_DIR}/WallpaperEngine/Scripting/Builtins.generated.h" @ONLY)

set(SOURCES
    src/External/MimeTypes/MimeTypes.cpp
    src/Steam/FileSystem/FileSystem.cpp
    src/WallpaperEngine/Logging/Log.cpp
    src/WallpaperEngine/Debugging/CallStack.cpp
    src/WallpaperEngine/Application/ApplicationContext.cpp
    src/WallpaperEngine/Application/WallpaperApplication.cpp
    src/WallpaperEngine/Assets/AssetLoadException.cpp
    src/WallpaperEngine/Assets/AssetLocator.cpp
    src/WallpaperEngine/FileSystem/Container.cpp
    src/WallpaperEngine/FileSystem/Adapters/Directory.cpp
    src/WallpaperEngine/FileSystem/Adapters/Package.cpp
    src/WallpaperEngine/FileSystem/Adapters/Virtual.cpp
    src/WallpaperEngine/FileSystem/Adapters/MediaCover.cpp
    src/WallpaperEngine/Audio/Drivers/Recorders/PulseAudioPlaybackRecorder.cpp
    src/WallpaperEngine/Audio/Drivers/Recorders/PlaybackRecorder.cpp
    src/WallpaperEngine/Audio/Drivers/Detectors/PulseAudioPlayingDetector.cpp
    src/WallpaperEngine/Audio/Drivers/Detectors/AudioPlayingDetector.cpp
    src/WallpaperEngine/Audio/Drivers/AudioDriver.cpp
    src/WallpaperEngine/Audio/Drivers/SDLAudioDriver.cpp
    src/WallpaperEngine/Audio/AudioContext.cpp
    src/WallpaperEngine/Audio/AudioStream.cpp
    src/WallpaperEngine/Input/InputContext.cpp
    src/WallpaperEngine/Input/Drivers/GLFWMouseInput.cpp
    src/WallpaperEngine/Media/MediaSource.cpp
    src/WallpaperEngine/Media/DBusMediaSource.cpp
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariable.cpp
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableFloat.cpp
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableInteger.cpp
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableVector2.cpp
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableVector3.cpp
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableVector4.cpp
    src/WallpaperEngine/Render/Shaders/Shader.cpp
    src/WallpaperEngine/Render/Shaders/ShaderUnit.cpp
    src/WallpaperEngine/Render/Shaders/GLSLContext.cpp
    src/WallpaperEngine/Render/Helpers/ContextAware.cpp
    src/WallpaperEngine/Render/Drivers/VideoFactories.cpp
    src/WallpaperEngine/Render/Drivers/Detectors/FullScreenDetector.cpp
    src/WallpaperEngine/Render/Drivers/Output/Output.cpp
    src/WallpaperEngine/Render/Drivers/Output/GLFWWindowOutput.cpp
    src/WallpaperEngine/Render/Drivers/Output/GLFWOutputViewport.cpp
    src/WallpaperEngine/Render/Drivers/Output/OutputViewport.cpp
    src/WallpaperEngine/Render/Drivers/GLFWOpenGLDriver.cpp
    src/WallpaperEngine/Render/Drivers/VideoDriver.cpp
    src/WallpaperEngine/Render/RenderContext.cpp
    src/WallpaperEngine/Render/TextureCache.cpp
    src/WallpaperEngine/Render/FBOProvider.cpp
    src/WallpaperEngine/Render/Helpers/ContextAware.cpp
    src/WallpaperEngine/Render/CWallpaper.cpp
    src/WallpaperEngine/Render/WallpaperState.cpp
    src/WallpaperEngine/Render/Wallpapers/CScene.cpp
    src/WallpaperEngine/Render/Wallpapers/CVideo.cpp
    src/WallpaperEngine/Render/Camera.cpp
    src/WallpaperEngine/Render/CObject.cpp
    src/WallpaperEngine/Render/CTexture.cpp
    src/WallpaperEngine/Render/AlbumTexture.cpp
    src/WallpaperEngine/Render/Objects/CRenderable.cpp
    src/WallpaperEngine/Render/Objects/CImage.cpp
    src/WallpaperEngine/Render/Objects/Puppet.cpp
    src/WallpaperEngine/Render/Objects/CSound.cpp
    src/WallpaperEngine/Render/Objects/CParticle.cpp
    src/WallpaperEngine/Render/Objects/CText.cpp
    src/WallpaperEngine/Render/CFBO.cpp
    src/WallpaperEngine/Render/Objects/Effects/CPass.cpp
    src/WallpaperEngine/Data/Model/DynamicValue.cpp
    src/WallpaperEngine/Data/Utils/BinaryReader.cpp
    src/WallpaperEngine/Data/Parsers/DynamicValueParser.cpp
    src/WallpaperEngine/Data/Parsers/EffectParser.cpp
    src/WallpaperEngine/Data/Parsers/ProjectParser.cpp
    src/WallpaperEngine/Data/Parsers/WallpaperParser.cpp
    src/WallpaperEngine/Data/Parsers/UserSettingParser.cpp
    src/WallpaperEngine/Data/Parsers/ObjectParser.cpp
    src/WallpaperEngine/Data/Parsers/MaterialParser.cpp
    src/WallpaperEngine/Data/Parsers/ModelParser.cpp
    src/WallpaperEngine/Data/Parsers/ShaderConstantParser.cpp
    src/WallpaperEngine/Data/Parsers/PackageParser.cpp
    src/WallpaperEngine/Data/Parsers/PropertyParser.cpp
    src/WallpaperEngine/Data/Parsers/TextureParser.cpp
    src/WallpaperEngine/Data/Builders/ColorBuilder.cpp
    src/WallpaperEngine/Data/Builders/VectorBuilder.cpp
    src/WallpaperEngine/Data/Dumpers/StringPrinter.cpp
    src/WallpaperEngine/Data/JSON.cpp
    src/WallpaperEngine/VideoPlayback/MPV/GLPlayer.cpp
    src/WallpaperEngine/VideoPlayback/MPV/MemoryStreamProtocol.cpp
    src/WallpaperEngine/Scripting/Adapters/ObjectAdapter.cpp
    src/WallpaperEngine/Scripting/Adapters/VectorAdapter.cpp
    src/WallpaperEngine/Scripting/Adapters/ScriptableObjectAdapter.cpp
    src/WallpaperEngine/Scripting/Modules/ColorModule.cpp
    src/WallpaperEngine/Scripting/Modules/MathModule.cpp
    src/WallpaperEngine/Scripting/Modules/ScriptModule.cpp
    src/WallpaperEngine/Scripting/ConsoleObject.cpp
    src/WallpaperEngine/Scripting/EngineObject.cpp
    src/WallpaperEngine/Scripting/InputObject.cpp
    src/WallpaperEngine/Scripting/SceneObject.cpp
    src/WallpaperEngine/Scripting/ScriptPropertiesObject.cpp
    src/WallpaperEngine/Scripting/ScriptEngine.cpp
    src/WallpaperEngine/Scripting/ScriptableObject.cpp
    src/WallpaperEngine/Maths.cpp
)
add_executable(wpengine src/main.cpp src/WallpaperEngine/Render/Drivers/MacDesktopWindow.mm ${SOURCES})
target_compile_definitions(wpengine PRIVATE ERRORONLY=0 DEMOMODE=0)
target_include_directories(wpengine PRIVATE
    ${WPENGINE_GENERATED_DIR} src
    src/External/glslang-WallpaperEngine src/External/SPIRV-Cross-WallpaperEngine
    src/External/MimeTypes src/External/json/include src/External/stb src/External/kissfft
    ${GLEW_INCLUDE_DIRS} ${LZ4_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${MPV_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS} /opt/homebrew/include)
target_link_directories(wpengine PRIVATE ${LZ4_LIBRARY_DIRS} ${SDL2_LIBRARY_DIRS} ${MPV_LIBRARY_DIRS} ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(wpengine PRIVATE
    ${OPENGL_FW} ${COCOA_FW} ${IOKIT_FW} GLEW::GLEW glfw ZLIB::ZLIB Freetype::Freetype
    ${LZ4_LIBRARIES} ${SDL2_LIBRARIES} ${MPV_LIBRARIES} ${FFMPEG_LIBRARIES}
    kissfft glslang spirv-cross-core spirv-cross-glsl argparse qjs)
