#include "ObjectLoader.hpp"

namespace ObjLoader
{

    std::shared_ptr<Mesh> loadObjMesh(const std::string& path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning;
    std::string error;

    const bool loaded = tinyobj::LoadObj(
        &attrib,
        &shapes,
        &materials,
        &warning,
        &error,
        path.c_str());

    if (!loaded || !error.empty())
    {
        throw std::runtime_error("Failed to load OBJ: " + path + "\n" + error + warning);
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (const auto& shape : shapes)
    {
        const std::size_t vertexStart = vertices.size();

        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};
            vertex.position = glm::vec3(
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]);

            vertex.color = glm::vec3(1.0f, 0.0f, 1.0f); 
            vertices.push_back(vertex);
            indices.push_back(static_cast<uint32_t>(vertexStart + vertices.size() - vertexStart - 1));
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}
}