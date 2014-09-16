
#ifndef H_ROB_FONT_H
#define H_ROB_FONT_H

#include "../graphics/GraphicsTypes.h"
#include "../Types.h"

namespace rob
{

    struct Glyph
    {
        // Position in texture
        int16_t m_x, m_y;
        // Size in pixels
        int16_t m_width, m_height;
        int16_t m_offsetX;
        int16_t m_offsetY;
        // Advance in x direction
        int16_t m_advance;

        uint16_t m_textureIdx;
    };

    class Font
    {
    public:
        Font();

        bool IsReady() const;

        void SetBase(uint16_t base);
        uint16_t GetBase() const;

        void SetHeight(int16_t ascender, int16_t descender);
        int16_t GetAscender() const;
        int16_t GetDescender() const;

        void SetHorizontalSpacing(uint16_t spacing);
        uint16_t GetHorizontalSpacing() const;

        void SetLineSpacing(uint16_t spacing);
        uint16_t GetLineSpacing() const;

        void AddGlyph(uint32_t index, const Glyph &glyph);
        void AddGlyphMapping(uint32_t character, uint32_t index);
        const Glyph& GetGlyph(uint32_t character) const;

        void AddTexture(size_t page, TextureHandle texture);
        TextureHandle GetTexture(size_t page) const;

    private:
        uint16_t m_base;
        int16_t m_ascender;
        int16_t m_descender;
        uint16_t m_horiSpacing;
        uint16_t m_lineSpacing;

        static const size_t MAX_GLYPHS = 512;
        Glyph       m_glyph[MAX_GLYPHS];
        uint32_t    m_characterToGlyph[MAX_GLYPHS];
        size_t m_glyphCount;

        static const size_t MAX_TEXTURE_PAGES = 4;
        TextureHandle m_textures[MAX_TEXTURE_PAGES];

    };

} // rob

#endif // H_ROB_FONT_H

