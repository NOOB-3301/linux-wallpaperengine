#include "Puppet.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>

using namespace WallpaperEngine::Render::Objects;

namespace {
class Reader {
public:
    explicit Reader (const std::vector<char>& data) : m_data (data) { }
    bool eof () const { return m_pos >= m_data.size (); }
    char peek () const { return m_pos < m_data.size () ? m_data[m_pos] : 0; }
    template <typename T> T read () {
	T value {};
	if (m_pos + sizeof (T) <= m_data.size ()) {
	    std::memcpy (&value, m_data.data () + m_pos, sizeof (T));
	}
	m_pos += sizeof (T);
	return value;
    }
    std::string str () {
	std::string out;
	while (m_pos < m_data.size () && m_data[m_pos] != '\0') {
	    out.push_back (m_data[m_pos++]);
	}
	m_pos++;
	return out;
    }
    void skip (size_t bytes) { m_pos += bytes; }
    size_t pos () const { return m_pos; }
    // moves to the next occurrence of `marker` at or after the current position; false when there is none
    bool seekTo (const char* marker) {
	const size_t length = std::strlen (marker);
	for (size_t at = m_pos; at + length <= m_data.size (); at++) {
	    if (std::memcmp (m_data.data () + at, marker, length) == 0) {
		m_pos = at;
		return true;
	    }
	}
	return false;
    }
    glm::mat4 mat4 () {
	glm::mat4 m;
	for (int column = 0; column < 4; column++) {
	    for (int row = 0; row < 4; row++) {
		m[column][row] = read<float> ();
	    }
	}
	return m;
    }
    // "MDLV0021\0" -> 21; 0 when the prefix does not match
    int version (const char* prefix) {
	char text[9] = {};
	for (char& c : text) c = read<char> ();
	if (std::strncmp (text, prefix, 3) != 0) return 0;
	return std::atoi (text + 4);
    }

private:
    const std::vector<char>& m_data;
    size_t m_pos = 0;
};

glm::dquat toQuaternion (const glm::vec3& euler) {
    return glm::angleAxis (static_cast<double> (euler.z), glm::dvec3 (0, 0, 1))
	* glm::angleAxis (static_cast<double> (euler.y), glm::dvec3 (0, 1, 0))
	* glm::angleAxis (static_cast<double> (euler.x), glm::dvec3 (1, 0, 0));
}

Puppet::Mode toMode (const std::string& mode) {
    if (mode == "mirror") return Puppet::Mode::Mirror;
    if (mode == "single") return Puppet::Mode::Single;
    return Puppet::Mode::Loop;
}
} // namespace

std::unique_ptr<Puppet> Puppet::parse (const std::vector<char>& data) {
    constexpr uint32_t stdVertex = 4 * (3 + 4 + 4 + 2);
    constexpr uint32_t altVertex = 4 * (3 + 4 + 4 + 2 + 7);
    constexpr uint32_t stdHerald = 0x01800009;
    constexpr uint32_t altHerald = 0x0180000F;
    constexpr uint32_t boneFrameSize = 4 * 9;

    Reader f (data);
    auto puppet = std::make_unique<Puppet> ();

    if (f.version ("MDL") == 0) return nullptr;
    if (f.read<int32_t> () == 9) return nullptr; // incomplete puppet
    f.read<int32_t> ();
    f.read<int32_t> ();
    f.str (); // material
    f.read<int32_t> ();

    bool alt = false;
    uint32_t current = f.read<uint32_t> ();
    if (current == 0) {
	alt = true;
	while (current != altHerald && !f.eof ()) current = f.read<uint32_t> ();
	current = f.read<uint32_t> ();
    } else if (current == stdHerald) {
	current = f.read<uint32_t> ();
    }
    const uint32_t stride = alt ? altVertex : stdVertex;
    if (current == 0 || current % stride != 0) return nullptr;

    puppet->vertices.resize (current / stride);
    for (auto& v : puppet->vertices) {
	v.position = { f.read<float> (), f.read<float> (), f.read<float> () };
	if (alt) f.skip (7 * 4);
	for (auto& b : v.bones) b = f.read<uint32_t> ();
	v.weights = { f.read<float> (), f.read<float> (), f.read<float> (), f.read<float> () };
	v.texcoord = { f.read<float> (), f.read<float> () };
    }

    const uint32_t indexBytes = f.read<uint32_t> ();
    if (indexBytes % 6 != 0) return nullptr;
    puppet->indices.resize (indexBytes / 2);
    for (auto& index : puppet->indices) index = f.read<uint16_t> ();
    for (const auto index : puppet->indices) {
	if (index >= puppet->vertices.size ()) return nullptr;
    }

    if (!f.seekTo ("MDLS")) return nullptr;
    const int mdls = f.version ("MDL");
    f.read<uint32_t> (); // end offset
    const uint16_t boneCount = f.read<uint16_t> ();
    f.read<uint16_t> ();
    puppet->bones.resize (boneCount);
    for (uint32_t i = 0; i < boneCount; i++) {
	auto& bone = puppet->bones[i];
	f.str (); // name
	f.read<int32_t> ();
	bone.parent = f.read<uint32_t> ();
	if (bone.hasParent () && bone.parent >= i) return nullptr;
	if (f.read<uint32_t> () != 64) return nullptr;
	bone.transform = f.mat4 ();
	f.str (); // simulation json
    }

    if (mdls > 1) {
	f.read<int16_t> ();
	if (f.read<uint8_t> ()) f.skip (static_cast<size_t> (boneCount) * 64);
	const uint32_t unknown = f.read<uint32_t> ();
	f.skip (static_cast<size_t> (unknown) * 12);
	f.read<uint32_t> ();
	if (f.read<uint8_t> ()) f.skip (static_cast<size_t> (boneCount) * (12 + 64));
	if (f.read<uint8_t> ()) f.skip (static_cast<size_t> (boneCount) * 4);
    }

    // optional MDAT (attachments) and padding before MDLA
    std::string type, version;
    while (type != "MDLA" && !f.eof ()) {
	const std::string prefix = f.str ();
	if (prefix.size () != 8) continue;
	type = prefix.substr (0, 4);
	version = prefix.substr (4, 4);
	if (type == "MDAT") {
	    f.read<uint32_t> ();
	    const uint16_t count = f.read<uint16_t> ();
	    for (uint16_t i = 0; i < count; i++) {
		Attachment attachment;
		attachment.bone = f.read<uint16_t> ();
		attachment.name = f.str ();
		attachment.transform = f.mat4 ();
		puppet->attachments.push_back (std::move (attachment));
	    }
	}
    }

    const int mdla = type == "MDLA" ? std::atoi (version.c_str ()) : 0;
    if (mdla != 0) {
	f.read<uint32_t> ();
	const uint32_t animationCount = f.read<uint32_t> ();
	puppet->animations.resize (animationCount);
	for (auto& animation : puppet->animations) {
	    // animations are separated by zero padding of varying length (not always a multiple of 4)
	    while (!f.eof () && f.peek () == 0) f.skip (1);
	    animation.id = f.read<int32_t> ();
	    if (animation.id <= 0) return nullptr;
	    f.read<int32_t> ();
	    std::string name = f.str ();
	    if (name.empty ()) name = f.str ();
	    animation.mode = toMode (f.str ());
	    animation.fps = f.read<float> ();
	    animation.length = std::max (1, f.read<int32_t> ());
	    f.read<int32_t> ();
	    const uint32_t boneFrames = f.read<uint32_t> ();
	    animation.bones.resize (boneFrames);
	    for (auto& frames : animation.bones) {
		f.read<int32_t> ();
		const uint32_t bytes = f.read<uint32_t> ();
		if (bytes % boneFrameSize != 0) return nullptr;
		frames.resize (bytes / boneFrameSize);
		for (auto& frame : frames) {
		    frame.position = { f.read<float> (), f.read<float> (), f.read<float> () };
		    frame.angle = { f.read<float> (), f.read<float> (), f.read<float> () };
		    frame.scale = { f.read<float> (), f.read<float> (), f.read<float> () };
		    frame.quaternion = toQuaternion (frame.angle);
		}
	    }
	    if (!alt && mdla != 3) {
		// optional animation events: {"frame":1,"name":"..."}
		const uint32_t extra = f.read<uint32_t> ();
		for (uint32_t i = 0; i < extra && i < 4096 && !f.eof (); i++) {
		    f.read<float> ();
		    f.str ();
		}
	    }
	}
    }

    // rest pose
    std::vector<glm::mat4> combined (puppet->bones.size ());
    for (size_t i = 0; i < puppet->bones.size (); i++) {
	auto& bone = puppet->bones[i];
	combined[i] = (bone.hasParent () ? combined[bone.parent] : glm::mat4 (1.0f)) * bone.transform;
	bone.offset = glm::inverse (combined[i]);
    }
    puppet->m_world = combined;
    puppet->m_skin.assign (puppet->bones.size (), glm::mat4 (1.0f));
    return puppet;
}

void Puppet::setLayers (const std::vector<Layer>& layers) {
    m_layers.clear ();
    m_layers.resize (layers.size ());
    m_globalBlend = 1.0;
    double totalBlend = 0.0;
    for (const auto& layer : layers) {
	if (layer.visible) totalBlend += layer.blend;
    }
    // last layer first, like the reference implementation
    for (size_t r = layers.size (); r-- > 0;) {
	const auto& layer = layers[r];
	auto& active = m_layers[r];
	active.layer = layer;
	const auto it = std::find_if (animations.begin (), animations.end (), [&] (const auto& a) {
	    return a.id == layer.id;
	});
	if (it == animations.end () || !layer.visible) continue;
	active.animation = &*it;
	if (totalBlend > 1.0) {
	    active.blend = layer.blend / totalBlend;
	    m_globalBlend = 0.0;
	} else {
	    active.blend = m_globalBlend * layer.blend;
	    m_globalBlend = std::max (0.0, m_globalBlend * (1.0 - layer.blend));
	}
    }
}

void Puppet::update (const double delta) {
    for (auto& active : m_layers) {
	if (active.animation == nullptr) continue;
	const auto& a = *active.animation;
	const double frameTime = 1.0 / a.fps;
	const bool mirror = a.mode == Mode::Mirror;
	const size_t length = static_cast<size_t> (a.length) * (mirror ? 2 : 1);
	const double maxTime = a.length / a.fps * (mirror ? 2.0 : 1.0);
	active.time += delta * active.layer.rate;
	if (a.mode == Mode::Single) {
	    active.time = std::min (active.time, maxTime - frameTime);
	}
	active.time = std::fmod (active.time, maxTime);
	const double rate = active.time / frameTime;
	active.frameA = static_cast<size_t> (rate) % length;
	active.frameB = (active.frameA + 1) % length;
	active.t = rate - std::floor (rate);
	if (mirror) {
	    const auto fold = [&] (size_t f) { return f >= static_cast<size_t> (a.length) ? (a.length - 1) - (f - a.length) : f; };
	    active.frameA = fold (active.frameA);
	    active.frameB = fold (active.frameB);
	}
    }

    const glm::dquat identity (1, 0, 0, 0);
    for (size_t i = 0; i < bones.size (); i++) {
	const auto& bone = bones[i];
	glm::vec3 translation = glm::vec3 (bone.transform[3]) * static_cast<float> (m_globalBlend);
	glm::vec3 scale = glm::vec3 (1.0f) * static_cast<float> (m_globalBlend);
	glm::dquat rotation = identity;

	for (const auto& active : m_layers) {
	    if (active.animation == nullptr || i >= active.animation->bones.size ()) continue;
	    const auto& frames = active.animation->bones[i];
	    if (frames.empty ()) continue;
	    const auto& base = frames[0];
	    const auto& fa = frames[std::min (active.frameA, frames.size () - 1)];
	    const auto& fb = frames[std::min (active.frameB, frames.size () - 1)];
	    const double t = active.t;
	    const float tf = static_cast<float> (t);
	    const double layerBlend = active.layer.blend;

	    const auto deltaA = fa.quaternion * glm::conjugate (base.quaternion);
	    const auto deltaB = fb.quaternion * glm::conjugate (base.quaternion);
	    rotation *= glm::slerp (glm::slerp (deltaA, deltaB, t), identity, 1.0 - layerBlend)
		* glm::slerp (base.quaternion, identity, 1.0 - active.blend);
	    translation += static_cast<float> (active.blend) * base.position
		+ static_cast<float> (layerBlend) * ((fa.position - base.position) * (1.0f - tf) + (fb.position - base.position) * tf);
	    scale += static_cast<float> (active.blend) * base.scale
		+ static_cast<float> (layerBlend) * ((fa.scale - base.scale) * (1.0f - tf) + (fb.scale - base.scale) * tf);
	}

	glm::mat4 local = glm::translate (glm::mat4 (1.0f), translation)
	    * glm::mat4_cast (glm::quat (glm::slerp (rotation, identity, m_globalBlend))) * glm::scale (glm::mat4 (1.0f), scale);
	m_world[i] = (bone.hasParent () ? m_world[bone.parent] : glm::mat4 (1.0f)) * local;
	m_skin[i] = m_world[i] * bone.offset;
    }
}

std::vector<glm::vec3> Puppet::skinnedPositions () const {
    std::vector<glm::vec3> out;
    out.reserve (vertices.size ());
    for (const auto& v : vertices) {
	glm::vec4 sum (0.0f);
	float total = 0.0f;
	for (int k = 0; k < 4; k++) {
	    const float w = v.weights[k];
	    if (w <= 0.0f || v.bones[k] >= m_skin.size ()) continue;
	    sum += w * (m_skin[v.bones[k]] * glm::vec4 (v.position, 1.0f));
	    total += w;
	}
	out.push_back (total > 0.0f ? glm::vec3 (sum) / total : v.position);
    }
    return out;
}

std::optional<glm::mat4> Puppet::attachment (const std::string& name) const {
    for (const auto& attachment : attachments) {
	if (attachment.name == name && attachment.bone < m_world.size ()) {
	    // the attachment matrix is relative to its bone
	    return m_world[attachment.bone] * attachment.transform;
	}
    }
    return std::nullopt;
}
