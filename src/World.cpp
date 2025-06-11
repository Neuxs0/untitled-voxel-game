#include "World.hpp"

World::World()
{
    // Create one chunk at the world origin
    auto chunk = std::make_unique<Chunk>(glm::vec3(0.0f, -0.8f, 0.0f));

    chunk->generateMesh();

    m_chunks.push_back(std::move(chunk));
}

void World::render(Shader &shader)
{
    for (const auto &chunk : m_chunks)
        chunk->renderOpaque(shader);

    glDepthMask(GL_FALSE); // Disable writing to the depth buffer
    for (const auto &chunk : m_chunks)
        chunk->renderTransparent(shader);
    
    glDepthMask(GL_TRUE); // Re-enable depth writing
}
