
#include "Font.h"
#include "../Assert.h"
#include "../Log.h"

namespace rob
{

    size_t CharToIndex(uint32_t character)
    {
        static const size_t firstPrintable = 32;
        return character - firstPrintable;
    }

    Font::Font()
        : m_base(0)
        , m_ascender(0)
        , m_descender(0)
        , m_horiSpacing(0)
        , m_lineSpacing(0)
        , m_glyphCount(0)
        , m_textureCount(0)
    {
//        for (size_t i = 0; i < MAX_KERNING_GLYPHS; i++)
//            for (size_t j = 0; j < MAX_KERNING_GLYPHS; j++)
//                m_kerning[i][j] = 0;
    }

    bool Font::IsReady() const
    { return m_glyphCount > 0; }

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

    void Font::SetHorizontalSpacing(uint16_t spacing)
    { m_horiSpacing = spacing; }

    uint16_t Font::GetHorizontalSpacing() const
    { return m_horiSpacing; }

    void Font::SetLineSpacing(uint16_t spacing)
    { m_lineSpacing = spacing; }

    uint16_t Font::GetLineSpacing() const
    { return m_lineSpacing; }

    void Font::AddGlyph(uint32_t character, const Glyph &glyph)
    {
        const size_t index = CharToIndex(character);
        ROB_ASSERT(index < MAX_GLYPHS);
        m_glyph[index] = glyph;
        m_glyphCount++;
    }

    const Glyph& Font::GetGlyph(uint32_t character) const
    {
        const size_t index = CharToIndex(character);
        ROB_ASSERT(index < MAX_GLYPHS);
        return m_glyph[index];
    }

    void Font::AddTexture(size_t page, TextureHandle texture)
    {
        ROB_ASSERT(page < MAX_TEXTURE_PAGES);
        m_textures[page] = texture;
        m_textureCount = (page + 1 > m_textureCount) ? page + 1 : m_textureCount;
    }

    TextureHandle Font::GetTexture(size_t page) const
    {
        ROB_ASSERT(page < MAX_TEXTURE_PAGES);
        return m_textures[page];
    }

    size_t Font::GetTextureCount() const
    { return m_textureCount; }

    static int8_t g_kerningMap[256][256];

    void Font::AddKerning(uint32_t c1, uint32_t c2, int16_t kerning)
    {
        const size_t i1 = CharToIndex(c1);
        const size_t i2 = CharToIndex(c2);
//        ROB_ASSERT(i1 < MAX_KERNING_GLYPHS); ROB_ASSERT(i2 < MAX_KERNING_GLYPHS);
//        if (i1 > MAX_KERNING_GLYPHS || i2 > MAX_KERNING_GLYPHS)
//            log::Info("!!!Kerning: ", kerning);
//            return;
//        log::Info("Kerning: ", kerning);
//        m_kerning[i1][i2] = kerning;
        g_kerningMap[i1][i2] = kerning;
    }

    int8_t Font::GetKerning(uint32_t c1, uint32_t c2)
    {
        const size_t i1 = CharToIndex(c1);
        const size_t i2 = CharToIndex(c2);
//        if (i1 > MAX_KERNING_GLYPHS || i2 > MAX_KERNING_GLYPHS) return 0;
//        ROB_ASSERT(i1 < MAX_KERNING_GLYPHS); ROB_ASSERT(i2 < MAX_KERNING_GLYPHS);
//        return m_kerning[i1][i2];
        return g_kerningMap[i1][i2];
    }

} // rob
