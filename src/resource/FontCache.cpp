
#include "FontCache.h"
#include "MasterCache.h"
#include "../graphics/Graphics.h"
#include "../Types.h"
#include "../Log.h"

#include <fstream>

namespace rob
{

    FontCache::FontCache(Graphics *graphics, MasterCache *cache)
        : m_graphics(graphics)
        , m_cache(cache)
    { }

    ROB_DEFINE_RESOURCE_CACHE_DTOR(FontCache)

    static bool LoadFont(std::istream &in, Font &font, MasterCache *cache);

    bool FontCache::Load(const char * const filename, Font &font)
    {
        std::ifstream in(filename, std::ios_base::binary);
        if (!in.is_open())
        {
            log::Error("Could not open font file ", filename);
            return false;
        }

        return LoadFont(in, font, m_cache);
    }

    void FontCache::Unload(Font font)
    {
//        for (size_t i = 0; i < font.GetTextureCount(); i++)
//            m_cache->UnloadTexture(font.GetTexture(i));
    }

    static constexpr uint8_t BmfVersion = 3;

    struct BmfInfoBlock
    {
        static constexpr uint8_t TYPE = 0x01;

        int16_t font_size;
        uint8_t bit_field;
        uint8_t char_set;
        uint16_t stretch_h;
        uint8_t anti_aliasing;
        uint8_t padding_up;
        uint8_t padding_right;
        uint8_t padding_down;
        uint8_t padding_left;
        uint8_t spacing_horizontal;
        uint8_t spacing_vertical;
        uint8_t outline;
    };

    struct BmfCommonBlock
    {
        static constexpr uint8_t TYPE = 0x02;

        uint16_t line_height;
        uint16_t base;
        uint16_t scale_w;
        uint16_t scale_h;
        uint16_t pages;
        uint8_t bit_field;
        uint8_t alpha_channel;
        uint8_t red_channel;
        uint8_t green_channel;
        uint8_t blue_channel;
    };

    struct BmfPageBlock
    {
        static constexpr uint8_t TYPE = 0x03;
    };

    struct BmfCharBlock
    {
        static constexpr uint8_t TYPE = 0x04;

        uint32_t id;
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
        int16_t offset_x;
        int16_t offset_y;
        uint16_t advance_x;
        uint8_t page;
        uint8_t channel;
    };

    struct BmfKerningPairBlock
    {
        static constexpr uint8_t TYPE = 0x05;

        uint32_t first;
        uint32_t second;
        int16_t amount;
    };

    template <class T>
    T ReadValue(std::istream &file)
    {
        T value;
        file.read(reinterpret_cast<char*>(&value), sizeof(T));
        return value;
    }

    size_t ReadString(std::istream &file, char (&buffer)[64])
    {
        file.getline(buffer, 64, '\0');
        return file.gcount();
    }

    void ChangePageTextureExtension(char (&buffer)[64])
    {
        size_t pos = 64 - 1;
        while (pos && buffer[pos] != '.')
            pos--;
        buffer[++pos] = 't';
        buffer[++pos] = 'e';
        buffer[++pos] = 'x';
        buffer[++pos] = '\0';
    }

    static bool LoadFont(std::istream &in, Font &font, MasterCache *cache)
    {
        if (! (ReadValue<uint8_t>(in) == 'B'
            && ReadValue<uint8_t>(in) == 'M'
            && ReadValue<uint8_t>(in) == 'F'))
        {
            log::Error("Font cache: Invalid file header.");
            return false;
        }

        const uint8_t version = ReadValue<uint8_t>(in);
        if (version != BmfVersion)
        {
            log::Error("Font cache: Wrong version. Expected ", BmfVersion, ", found ", static_cast<unsigned int>(version));
        }

        size_t pageCount = 0;

        while (in)
        {
            const uint8_t block_type = ReadValue<uint8_t>(in);
            if (in.eof()) break;
            const uint32_t block_size = ReadValue<uint32_t>(in);

            switch (block_type)
            {
            case BmfInfoBlock::TYPE:
                {
                    const std::ios::streampos pos = in.tellg();
                    BmfInfoBlock block;
                    block.font_size         = ReadValue<int16_t>(in);
                    block.bit_field         = ReadValue<uint8_t>(in);
                    block.char_set          = ReadValue<uint8_t>(in);
                    block.stretch_h         = ReadValue<uint16_t>(in);
                    block.anti_aliasing     = ReadValue<uint8_t>(in);
                    block.padding_up        = ReadValue<uint8_t>(in);
                    block.padding_right     = ReadValue<uint8_t>(in);
                    block.padding_down      = ReadValue<uint8_t>(in);
                    block.padding_left      = ReadValue<uint8_t>(in);
                    block.spacing_horizontal = ReadValue<uint8_t>(in);
                    block.spacing_vertical  = ReadValue<uint8_t>(in);
                    block.outline           = ReadValue<uint8_t>(in);

                    // Skip font name
                    const size_t block_read_size = in.tellg() - pos;
                    const size_t block_discard_size = block_size - block_read_size;
                    in.seekg(block_discard_size, std::ios::cur);

                    font.SetLineSpacing(block.spacing_vertical);
                }
                break;

            case BmfCommonBlock::TYPE:
                {
                    BmfCommonBlock block;
                    block.line_height    = ReadValue<uint16_t>(in);
                    block.base           = ReadValue<uint16_t>(in);
                    block.scale_w        = ReadValue<uint16_t>(in);
                    block.scale_h        = ReadValue<uint16_t>(in);
                    block.pages          = ReadValue<uint16_t>(in);
                    block.bit_field      = ReadValue<uint8_t>(in);
                    block.alpha_channel  = ReadValue<uint8_t>(in);
                    block.red_channel    = ReadValue<uint8_t>(in);
                    block.green_channel  = ReadValue<uint8_t>(in);
                    block.blue_channel   = ReadValue<uint8_t>(in);
                    pageCount = block.pages;

                    font.SetBase(block.base);
                }
                break;

            case BmfPageBlock::TYPE:
                {
                    char pageName[64];
                    size_t pageNameLen = 0;
                    const std::ios::streampos startPos = in.tellg();
                    for (size_t i = 0; i < pageCount; i++)
                    {
                        in.seekg(startPos + std::ios::streamoff(i * pageNameLen), std::ios::beg);
                        size_t len = ReadString(in, pageName);
                        if (!pageNameLen) pageNameLen = len;

                        ChangePageTextureExtension(pageName);
                        // NOTE: To prevent from using the char[64] version decay to char*
                        const ResourceID texture(+pageName);
                        font.AddTexture(i, cache->GetTexture(texture));
                    }
                }
                break;

            case BmfCharBlock::TYPE:
                {
                    const size_t characterCount = block_size / 20;
                    for (size_t i = 0; i < characterCount; i++)
                    {
                        BmfCharBlock block;
                        block.id        = ReadValue<uint32_t>(in);
                        block.x         = ReadValue<uint16_t>(in);
                        block.y         = ReadValue<uint16_t>(in);
                        block.width     = ReadValue<uint16_t>(in);
                        block.height    = ReadValue<uint16_t>(in);
                        block.offset_x  = ReadValue<int16_t>(in);
                        block.offset_y  = ReadValue<int16_t>(in);
                        block.advance_x = ReadValue<uint16_t>(in);
                        block.page      = ReadValue<uint8_t>(in);
                        block.channel   = ReadValue<uint8_t>(in);

                        Glyph glyph;
                        glyph.m_x       = block.x;
                        glyph.m_y       = block.y;
                        glyph.m_width   = block.width;
                        glyph.m_height  = block.height;
                        glyph.m_offsetX = block.offset_x;
                        glyph.m_offsetY = block.offset_y;
                        glyph.m_advance = block.advance_x;
                        glyph.m_textureIdx = block.page;

                        font.AddGlyph(block.id, glyph);
                    }
                }
                break;

            case BmfKerningPairBlock::TYPE:
                {
//                    in.seekg(block_size, std::ios::cur);
                    const size_t kerningPairs = block_size / 10;
                    for (size_t i = 0; i < kerningPairs; i++)
                    {
                        BmfKerningPairBlock block;
                        block.first = ReadValue<uint32_t>(in);
                        block.second = ReadValue<uint32_t>(in);
                        block.amount = ReadValue<int16_t>(in);
                        font.AddKerning(block.first, block.second, block.amount);
                    }
                }
                break;

            default:
                log::Error("Font cache: Invalid block type ", static_cast<unsigned int>(block_type));
                return false;
            }
        }
        return true;
    }

} // rob