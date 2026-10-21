#include "GLFWMouseInput.h"
#include <glm/common.hpp>
#include <cstdio>
#include <cstdlib>

#include "WallpaperEngine/Render/Drivers/GLFWOpenGLDriver.h"

extern "C" void wpGlobalCursor (double* x, double* y);

using namespace WallpaperEngine::Input::Drivers;

GLFWMouseInput::GLFWMouseInput (const Render::Drivers::GLFWOpenGLDriver& driver) : m_driver (driver) { }

void GLFWMouseInput::update () {
    if (!this->m_driver.getApp ().getContext ().settings.mouse.enabled) {
	this->m_reportedPosition = { 0, 0 };
	return;
    }

    const int leftClickState = glfwGetMouseButton (this->m_driver.getWindow (), GLFW_MOUSE_BUTTON_LEFT);
    const int rightClickState = glfwGetMouseButton (this->m_driver.getWindow (), GLFW_MOUSE_BUTTON_RIGHT);

    this->m_leftClick = leftClickState == GLFW_RELEASE ? MouseClickStatus::Released : MouseClickStatus::Clicked;
    this->m_rightClick = rightClickState == GLFW_RELEASE ? MouseClickStatus::Released : MouseClickStatus::Clicked;

    // update current mouse position
    // the wallpaper window never receives mouse events on macOS, so read the global cursor instead
    double globalX, globalY;
    int windowX, windowY;
    wpGlobalCursor (&globalX, &globalY);
    glfwGetWindowPos (this->m_driver.getWindow (), &windowX, &windowY);
    // points -> framebuffer pixels (Retina)
    int windowWidth, windowHeight;
    glfwGetWindowSize (this->m_driver.getWindow (), &windowWidth, &windowHeight);
    const glm::ivec2 pixels = this->m_driver.getFramebufferSize ();
    const double scaleX = windowWidth > 0 ? static_cast<double> (pixels.x) / windowWidth : 1.0;
    const double scaleY = windowHeight > 0 ? static_cast<double> (pixels.y) / windowHeight : 1.0;
    this->m_mousePosition = { (globalX - windowX) * scaleX, (globalY - windowY) * scaleY };
    // debug/testing: WPENGINE_MOUSE="0.1,0.5" pins the cursor to a fraction of the window
    if (const char* fake = getenv ("WPENGINE_MOUSE")) {
	double fx = 0.5, fy = 0.5;
	sscanf (fake, "%lf,%lf", &fx, &fy);
	this->m_mousePosition = { fx * pixels.x, fy * pixels.y };
    }

    // Convert from GLFW coordinate system (Y=0 at top) to OpenGL coordinate system (Y=0 at bottom)
    const glm::ivec2 framebufferSize = this->m_driver.getFramebufferSize ();
    this->m_mousePosition.y = static_cast<double> (framebufferSize.y) - this->m_mousePosition.y;

    // interpolate to the new position
    this->m_reportedPosition = glm::mix (this->m_reportedPosition, this->m_mousePosition, 1.0);
}

glm::dvec2 GLFWMouseInput::position () const { return this->m_reportedPosition; }

WallpaperEngine::Input::MouseClickStatus GLFWMouseInput::leftClick () const { return m_leftClick; }

WallpaperEngine::Input::MouseClickStatus GLFWMouseInput::rightClick () const { return m_rightClick; }