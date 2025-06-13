#pragma once

#include <string>
#include <string_view>
#include <optional>

/**
 * @class EmbeddedTextures
 * @brief Provides access to texture data that has been embedded into the executable.
 *
 * This class uses a map to look up texture data by its original filename, returning
 * a string_view of the raw binary data.
 */
class EmbeddedTextures
{
public:
    /**
     * @brief Retrieves the raw binary data for a texture.
     * @param filename The simple filename of the texture (e.g., "stone.png").
     * @return An optional containing a string_view of the texture's binary data if found, otherwise std::nullopt.
     */
    static std::optional<std::string_view> get(const std::string& filename);

private:
    // The static map that holds the texture data. It's initialized on first access.
    static void initialize();
};
