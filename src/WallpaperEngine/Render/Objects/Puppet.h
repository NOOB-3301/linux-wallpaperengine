#pragma once

// macOS port: Wallpaper Engine puppet warp (.mdl): mesh, skeleton, bone animations and attachment points.
// Parser and animation blending follow catsout/wallpaper-engine-kde-plugin (WPMdlParser / WPPuppet), on glm.

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace WallpaperEngine::Render::Objects {
class Puppet {
public:
    struct Vertex {
	glm::vec3 position;
	uint32_t bones[4];
	glm::vec4 weights;
	glm::vec2 texcoord;
    };
    struct Bone {
	glm::mat4 transform { 1.0f };
	uint32_t parent = 0xFFFFFFFFu;
	glm::mat4 offset { 1.0f }; // inverse of the rest pose world transform
	[[nodiscard]] bool hasParent () const { return parent != 0xFFFFFFFFu; }
    };
    struct Frame {
	glm::vec3 position;
	glm::vec3 angle;
	glm::vec3 scale;
	glm::dquat quaternion;
    };
    enum class Mode { Loop, Mirror, Single };
    struct Animation {
	int id = 0;
	double fps = 30;
	int length = 1;
	Mode mode = Mode::Loop;
	std::vector<std::vector<Frame>> bones; // frames per bone
    };
    struct Attachment {
	std::string name;
	uint32_t bone = 0;
	glm::mat4 transform { 1.0f };
    };
    /** Animation layer as configured on the image object */
    struct Layer {
	int id = 0;
	double rate = 1;
	double blend = 1;
	bool visible = true;
    };

    /** Parses an .mdl file; nullopt when it is not a usable puppet */
    static std::unique_ptr<Puppet> parse (const std::vector<char>& data);

    /** Sets the active animation layers (call again when their settings change) */
    void setLayers (const std::vector<Layer>& layers);
    /** Advances the animations by `delta` seconds and recomputes the skinning matrices */
    void update (double delta);
    /** Skinned vertex positions for the current frame (model space, y up) */
    [[nodiscard]] std::vector<glm::vec3> skinnedPositions () const;
    /** Animated transform of a named attachment point in model space, if the puppet has it */
    [[nodiscard]] std::optional<glm::mat4> attachment (const std::string& name) const;

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<Bone> bones;
    std::vector<Animation> animations;
    std::vector<Attachment> attachments;

private:
    struct ActiveLayer {
	Layer layer;
	double blend = 0;
	double time = 0;
	const Animation* animation = nullptr;
	size_t frameA = 0, frameB = 0;
	double t = 0;
    };
    std::vector<ActiveLayer> m_layers;
    double m_globalBlend = 1.0;
    std::vector<glm::mat4> m_world;  // animated world transform per bone
    std::vector<glm::mat4> m_skin;   // m_world * offset
};
} // namespace WallpaperEngine::Render::Objects
