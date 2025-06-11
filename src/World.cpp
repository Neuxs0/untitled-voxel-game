#include "World.hpp"

World::World()
{
    // Create one chunk at the world origin
    auto chunk = std::make_unique<Chunk>(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Generate the mesh for the newly created chunk
    chunk->generateMesh();

    m_chunks.push_back(std::move(chunk));
}

void World::render(Shader &shader)
{
    // Render all chunks managed by the world
    for (const auto &chunk : m_chunks)
        chunk->render(shader);
}
