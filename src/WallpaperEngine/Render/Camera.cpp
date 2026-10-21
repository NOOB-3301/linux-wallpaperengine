#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

#include "WallpaperEngine/Application/ApplicationContext.h"
#include "WallpaperEngine/Application/WallpaperApplication.h"
#include "WallpaperEngine/Render/Wallpapers/CScene.h"

using namespace WallpaperEngine;
using namespace WallpaperEngine::Render;

Camera::Camera (Wallpapers::CScene& scene, const SceneData::Camera& camera) :
    m_width (0), m_height (0), m_camera (camera), m_scene (scene) {
    // get the lookat position
    // TODO: ENSURE THIS IS ONLY USED WHEN NOT DOING AN ORTOGRAPHIC CAMERA AS IT THROWS OFF POINTS
    this->m_lookat = glm::lookAt (this->getEye (), this->getCenter (), this->getUp ());
}

Camera::~Camera () = default;

const glm::vec3& Camera::getCenter () const { return this->m_camera.configuration.center; }

const glm::vec3& Camera::getEye () const { return this->m_camera.configuration.eye; }

const glm::vec3& Camera::getUp () const { return this->m_camera.configuration.up; }

const glm::mat4& Camera::getProjection () const { return this->m_projection; }

const glm::mat4& Camera::getLookAt () const { return this->m_lookat; }

bool Camera::isOrthogonal () const { return this->m_isOrthogonal; }

Wallpapers::CScene& Camera::getScene () const { return this->m_scene; }

float Camera::getWidth () const { return this->m_width; }

float Camera::getHeight () const { return this->m_height; }

float Camera::getFov () const { return this->m_camera.projection.fov->value->getFloat (); }

float Camera::getNearZ () const { return this->m_camera.projection.nearz->value->getFloat (); }

float Camera::getParallaxZoom () const { return this->m_parallaxZoom; }

float Camera::getFarZ () const { return this->m_camera.projection.farz->value->getFloat (); }

void Camera::setOrthogonalProjection (const float width, const float height) {
    this->m_width = width;
    this->m_height = height;

    float nearz = this->m_camera.projection.nearz->value->getFloat ();
    float farz = this->m_camera.projection.farz->value->getFloat ();

    // macOS port: parallax moves layers by up to (1 - 1/zoom)/2 of the scene size, and the zoom hides exactly that
    // margin. WPENGINE_PARALLAX_ZOOM changes it (1 = no margin, parallax shift is then 0).
    this->m_parallaxZoom = 1.0f;
    if (this->m_camera.parallax.enabled->value->getBool ()
	&& !this->m_scene.getContext ().getApp ().getContext ().settings.mouse.disableparallax) {
	const char* custom = getenv ("WPENGINE_PARALLAX_ZOOM");
	this->m_parallaxZoom = std::max (1.0f, custom != nullptr ? static_cast<float> (atof (custom)) : 1.06f);
    }
    const float zoom = this->m_parallaxZoom;

    this->m_projection = glm::ortho<float> (
	-width / 2.0 / zoom, width / 2.0 / zoom, -height / 2.0 / zoom, height / 2.0 / zoom, nearz, farz
    );
    this->m_projection = glm::translate (this->m_projection, this->getEye ());
    this->m_isOrthogonal = true;
}