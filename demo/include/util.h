#include "common.h"
#include "mesh.h"

namespace spe
{

std::unique_ptr<Mesh> generate_mesh_from_rigidbody(RigidBody& body, uint32_t circle_polygon_count = 13);

std::vector<uint32_t> triangulate(const std::vector<Vec2>& vertices);

} // namespace spe