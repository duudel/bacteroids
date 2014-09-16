
#include "Font.h"
#include "../Assert.h"

namespace rob
{

    Font::Font()
        : m_base(0)
        , m_ascender(0)
        , m_descender(0)
        , m_lineSpacing(0)
    { }

    void Font::SetBase(uint16_t base)
    { m_base = base; }

    uint16_t Font::GetBase() const
    { return m_base; }

    void Font::SetHeight(int16_t ascender, int16_t descender)
    {
        m_ascender = ascender;
        m_descender = descender;
    }

    int16_t Font::GetAscender() const
    { return m_ascender; }

    int16_t Font::GetDescender() const
    { return m_descender; }

    void Font::SetLineSpacing(uint16_t spacing)
    { m_lineSpacing = spacing; }

    uint16_t Font::GetLineSpacing() const
    { return m_lineSpacing; }

    void Font::AddGlyph(uint32_t index, const Glyph &glyph)
    {
        ROB_ASSERT(index < MAX_GLYPHS);
        m_glyph[index] = glyph;
    }

    void Font::AddGlyphMapping(uint32_t character, uint32_t index)
    {
        ROB_ASSERT(character < MAX_GLYPHS);
        m_characterToGlyph[character] = index;
    }

    const Glyph& Font::GetGlyph(uint32_t character) const
    {
        ROB_ASSERT(character < MAX_GLYPHS)
        const uint32_t index = m_characterToGlyph[character];
        return m_glyph[index];
    }

    void Font::AddTexture(size_t page, TextureHandle texture)
    {
        ROB_ASSERT(page < MAX_TEXTURE_PAGES);
        m_textures[page] = texture;
    }

    TextureHandle Font::GetTexture(size_t page) const
    {
        ROB_ASSERT(page < MAX_TEXTURE_PAGES);
        return m_textures[page];
    }

} // rob
