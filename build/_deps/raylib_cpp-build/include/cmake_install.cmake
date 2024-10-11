# Install script for directory: /Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Library/Developer/CommandLineTools/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/AudioDevice.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/AudioStream.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/AutomationEventList.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/BoundingBox.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Camera2D.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Camera3D.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Color.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/FileData.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/FileText.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Font.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Functions.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Gamepad.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Image.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Keyboard.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Material.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Matrix.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Mesh.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/MeshUnmanaged.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Model.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/ModelAnimation.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Mouse.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Music.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Ray.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/RayCollision.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/RaylibException.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/raylib-cpp-utils.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/raylib-cpp.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/raylib.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/raymath.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Rectangle.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/RenderTexture.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Shader.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Sound.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Text.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Texture.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/TextureUnmanaged.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Touch.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Vector2.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Vector3.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Vector4.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/VrStereoConfig.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Wave.hpp"
    "/Users/aaratha/projects/wrangler/build/_deps/raylib_cpp-src/include/Window.hpp"
    )
endif()

