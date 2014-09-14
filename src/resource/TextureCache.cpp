
#include "TextureCache.h"
#include "../graphics/Graphics.h"
#include "../graphics/Texture.h"
#include "../memory/LinearAllocator.h"

#include "../Log.h"

#include <fstream>

namespace rob
{

    TextureCache::TextureCache(Graphics *graphics)
        : m_graphics(graphics)
    { }

    ROB_DEFINE_RESOURCE_CACHE_DTOR(TextureCache)

    TextureHandle TextureCache::Load(const char * const filename)
    {
        std::ifstream in(filename, std::ios_base::binary);
        if (!in.is_open())
        {
            log::Error("Could not open texture file ", filename);
            return InvalidHandle;
        }

        size_t width = 0;
        size_t height = 0;
        size_t st_format = 0;
        in.read(reinterpret_cast<char*>(&width), sizeof(size_t));
        in.read(reinterpret_cast<char*>(&height), sizeof(size_t));
        in.read(reinterpret_cast<char*>(&st_format), sizeof(size_t));
        if (!in)
        {
            log::Error("Invalid texture file ", filename);
            return InvalidHandle;
        }
        const size_t imageSize = width * height * st_format;

        LinearAllocator alloc(imageSize);
        char *imageData = static_cast<char*>(alloc.Allocate(imageSize));

        in.read(imageData, imageSize);

        TextureHandle handle = m_graphics->CreateTexture();
        m_graphics->BindTexture(0, handle);
        Texture *texture = m_graphics->GetTexture(handle);
        texture->TexImage(width, height, static_cast<Texture::Format>(st_format), imageData);

        return handle;
    }

    void TextureCache::Unload(TextureHandle texture)
    {
        m_graphics->DestroyTexture(texture);
    }

} // rob
