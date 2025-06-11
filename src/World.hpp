#pragma once

#include <vector>
#include <memory>
#include "Chunk.hpp"
#include "Shader.hpp"

class World
{
private:
    std::vector<std::unique_ptr<Chunk>> m_chunks;

public:
    World();

    void render(Shader &shader);
};
