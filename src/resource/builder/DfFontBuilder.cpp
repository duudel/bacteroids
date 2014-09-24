
#include "DfFontBuilder.h"
#include "../BmfFont.internal.h"
#include "../../renderer/Font.h"
#include "../../util/StreamUtil.h"

#include "../../Log.h"
#include "../../math/Functions.h"

#include "../../filesystem/FileCopy.h"

#include <fstream>
#include <FreeImage.h>

#include <cstring>

namespace rob
{

    DfFontBuilder::DfFontBuilder()
    {
        m_extensions.push_back(".df.fnt");
        m_newExtension = ".df.fnt";
    }

    struct Image
    {
        Image() : data(nullptr), width(0), height(0), bytespp(0) { }

        uint8_t *data;
        size_t width;
        size_t height;
        size_t bytespp;
    };

    static bool LoadFont(std::istream &in, Font &font, char (&pageFiles)[4][64], size_t &pageCount);
    static bool LoadImage(const char * const filename, uint8_t *&imageData, size_t &width, size_t &height, size_t &bytespp);
    static bool SaveImage(const char * const filename, const uint8_t * const imageData,
                          const size_t width, const size_t height, const size_t bytespp);
    static bool ConvertToDistanceField(const Font &font, const Image (&pageImages)[4], const size_t pageCount,
                                       const std::string &directory, const char (&pageFiles)[4][64]);

    bool DfFontBuilder::Build(const std::string &directory, const std::string &filename,
                              const std::string &destDirectory, const std::string &destFilename)
    {
        std::ifstream in(filename.c_str(), std::ios::binary);
        if (!in.is_open()) return false;

        Font font;
        char pageFiles[4][64];
        size_t pageCount = 0;
        if (!LoadFont(in, font, pageFiles, pageCount))
            return false;

        Image pageImages[4];
        for (size_t i = 0; i < pageCount; i++)
        {
            const std::string filename = directory + "/" + pageFiles[i];
            if (!LoadImage(filename.c_str(), pageImages[i].data,
                           pageImages[i].width, pageImages[i].height, pageImages[i].bytespp))
            {
                return false;
            }
        }

        const bool result = ConvertToDistanceField(font, pageImages, pageCount, directory, pageFiles);

        for (size_t i = 0; i < pageCount; i++)
            delete[] pageImages[i].data;

        FileCopy(filename.c_str(), destFilename.c_str());

        return result;
    }

    static bool LoadFont(std::istream &in, Font &font, char (&pageFiles)[4][64], size_t &pageCount)
    {
        static constexpr uint8_t BmfVersion = 3;

        if (! (ReadValue<uint8_t>(in) == 'B'
            && ReadValue<uint8_t>(in) == 'M'
            && ReadValue<uint8_t>(in) == 'F'))
        {
            log::Error("Df Font builder: Invalid file header.");
            return false;
        }

        const uint8_t version = ReadValue<uint8_t>(in);
        if (version != BmfVersion)
        {
            log::Error("Df Font builder: Wrong version. Expected ", BmfVersion, ", found ", static_cast<unsigned int>(version));
        }

        while (in)
        {
            const uint8_t block_type = ReadValue<uint8_t>(in);
            if (in.eof()) break;
            const uint32_t block_size = ReadValue<uint32_t>(in);

            switch (block_type)
            {
            case BmfInfoBlock::TYPE:
                in.seekg(block_size, std::ios::cur);
                log::Debug("Df Font builder: Info block skipped");
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

                    log::Debug("Df Font builder: Common block skipped");
                }
                break;

            case BmfPageBlock::TYPE:
                {
                    size_t pageNameLen = 0;
                    const std::ios::streampos startPos = in.tellg();
                    for (size_t i = 0; i < pageCount; i++)
                    {
                        in.seekg(startPos + std::ios::streamoff(i * pageNameLen), std::ios::beg);
                        size_t len = ReadString(in, pageFiles[i]);
                        if (!pageNameLen) pageNameLen = len;
                    }
                    log::Debug("Df Font builder: Page block read");
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
                    log::Debug("Df Font builder: Character block read");
                }
                break;

            case BmfKerningPairBlock::TYPE:
                in.seekg(block_size, std::ios::cur);
                log::Debug("Df Font builder: Kerning pair block skipped");
                break;

            default:
                log::Error("Df Font builder: Invalid block type ", static_cast<unsigned int>(block_type), " at offset ",
                           in.tellg());
                return false;
            }
        }
        return true;
    }


    static bool LoadImage(const char * const filename, uint8_t *&imageData, size_t &width, size_t &height, size_t &bytespp)
    {
        FREE_IMAGE_FORMAT format = ::FreeImage_GetFileType(filename, 0);
        if (format == FIF_UNKNOWN)
        {
            format = ::FreeImage_GetFIFFromFilename(filename);
            if (format == FIF_UNKNOWN)
            {
                log::Error("Could not determine image file type for ", filename);
                return false;
            }
        }
        FIBITMAP *bitmap = ::FreeImage_Load(format, filename, 0);

        if (!bitmap)
        {
            log::Error("Could not load image ", filename);
            return false;
        }

        if (::FreeImage_GetImageType(bitmap) != FIT_BITMAP)
        {
            log::Error("Invalid image type in ", filename);
            return false;
        }

        const FREE_IMAGE_COLOR_TYPE colorType = ::FreeImage_GetColorType(bitmap);
        const size_t bpp = ::FreeImage_GetBPP(bitmap);
        switch (colorType)
        {
        case FIC_RGB:
//            log::Info("Image color type rgb");
            // For some reason rgba-png has color type FIC_RGB with bpp=32.
            if (bpp == 32)
                bytespp = 4;
            else
                bytespp = 3;
            break;
        case FIC_RGBALPHA:
//            log::Info("Image color type rgba");
            bytespp = 4;
            break;
        default:
            log::Error("Unsupported image color type in image ", filename);
            return false;
        }

        width = ::FreeImage_GetWidth(bitmap);
        height = ::FreeImage_GetHeight(bitmap);
        const size_t imageSize = width * height * bytespp;

        imageData = new unsigned char[imageSize];
        uint8_t *data = imageData;
        if (bytespp == 3)
        {
            for(size_t y = 0; y < height; y++)
            {
                const BYTE *bits = FreeImage_GetScanLine(bitmap, y);
                for(size_t x = 0; x < width; x++)
                {
                    data[0] = bits[FI_RGBA_RED];
                    data[1] = bits[FI_RGBA_GREEN];
                    data[2] = bits[FI_RGBA_BLUE];

                    bits += bytespp;
                    data += bytespp;
                }
            }
        }
        else
        {
            for(size_t y = 0; y < height; y++)
            {
                const BYTE *bits = FreeImage_GetScanLine(bitmap, y);
                for(size_t x = 0; x < width; x++)
                {
                    data[0] = bits[FI_RGBA_RED];
                    data[1] = bits[FI_RGBA_GREEN];
                    data[2] = bits[FI_RGBA_BLUE];
                    data[3] = bits[FI_RGBA_ALPHA];

                    bits += bytespp;
                    data += bytespp;
                }
            }
        }

        ::FreeImage_Unload(bitmap);

        return true;
    }

    static bool SaveImage(const char * const filename, const uint8_t * const imageData, const size_t width, const size_t height, const size_t bytespp)
    {
        FREE_IMAGE_FORMAT format = ::FreeImage_GetFIFFromFilename(filename);
        if (format == FIF_UNKNOWN)
        {
            log::Error("Could not determine image file type for ", filename);
            return false;
        }

        FIBITMAP *bitmap = ::FreeImage_Allocate(width, height, bytespp * 8, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
        if (!bitmap)
        {
            log::Error("Could not load image ", filename);
            return false;
        }

        const uint8_t *data = imageData;
        if (bytespp == 3)
        {
            for(size_t y = 0; y < height; y++)
            {
                BYTE *bits = FreeImage_GetScanLine(bitmap, y);
                for(size_t x = 0; x < width; x++)
                {
                    bits[FI_RGBA_RED] = data[0];
                    bits[FI_RGBA_GREEN] = data[1];
                    bits[FI_RGBA_BLUE] = data[2];

                    bits += bytespp;
                    data += bytespp;
                }
            }
        }
        else
        {
            for(size_t y = 0; y < height; y++)
            {
                BYTE *bits = FreeImage_GetScanLine(bitmap, y);
                for(size_t x = 0; x < width; x++)
                {
                    bits[FI_RGBA_RED] = data[0];
                    bits[FI_RGBA_GREEN] = data[1];
                    bits[FI_RGBA_BLUE] = data[2];
                    bits[FI_RGBA_ALPHA] = data[3];

                    bits += bytespp;
                    data += bytespp;
                }
            }
        }

        ::FreeImage_Save(format, bitmap, filename, 0);

        ::FreeImage_Unload(bitmap);
        return true;
    }

    struct Point
    {
        float dx, dy;

        Point() : dx(0.0f), dy(0.0f) { }
        Point(float dx_, float dy_) : dx(dx_), dy(dy_) { }

        float Dist2() const { return dx * dx + dy * dy; }
        float Dist() const { return Sqrt(Dist2()); }

        static const Point Outside;

        static Point Get(const int16_t x, const int16_t y,
                         const Point * const points, const int32_t w, const int32_t h)
        {
            if (x >= 0 && x < w && y >= 0 && y < h)
                return points[y * w + x];
            return Outside;
        }

        static Point Compare(const Point * const points, const int32_t w, const int32_t h,
                             const int16_t x, const int16_t y,
                             const int16_t offsetx, const int16_t offsety, const Point &pt)
        {
            const Point &other1 = Get(x + offsetx, y + offsety, points, w, h);
            const Point other2(other1.dx + offsetx, other1.dy + offsety);
            return other2.Dist2() < pt.Dist2() ? other2 : pt;
        }

        static void Generate(Point * const points, const int32_t width, const int32_t height)
        {
            for (int16_t y = 0; y < height; y++)
            {
                for (int16_t x = 0; x < width; x++)
                {
                    Point pt = points[y * width + x];
                    pt = Point::Compare(points, width, height, x, y, -1, 0, pt);
                    pt = Point::Compare(points, width, height, x, y, 0, -1, pt);
                    pt = Point::Compare(points, width, height, x, y, -1, -1, pt);
                    pt = Point::Compare(points, width, height, x, y, 1, -1, pt);
                    points[y * width + x] = pt;
                }

                for (int16_t x = width - 1; x >= 0 ; x--)
                {
                    Point pt = points[y * width + x];
                    pt = Point::Compare(points, width, height, x, y, 1, 0, pt);
                    points[y * width + x] = pt;
                }
            }

            for (int16_t y = height - 1; y >= 0; y--)
            {
                for (int16_t x = 0; x < width; x++)
                {
                    Point pt = points[y * width + x];
                    pt = Point::Compare(points, width, height, x, y, 1, 0, pt);
                    pt = Point::Compare(points, width, height, x, y, 0, 1, pt);
                    pt = Point::Compare(points, width, height, x, y, -1, 1, pt);
                    pt = Point::Compare(points, width, height, x, y, 1, 1, pt);
                    points[y * width + x] = pt;
                }

                for (int16_t x = width - 1; x >= 0 ; x--)
                {
                    Point pt = points[y * width + x];
                    pt = Point::Compare(points, width, height, x, y, -1, 0, pt);
                    points[y * width + x] = pt;
                }
            }
        }
    };

    const Point Point::Outside(10000, 10000);

    static bool ConvertGlyphToDf(const Glyph &glyph, const Image &image, uint8_t *data)
    {
        Point *points1 = new Point[glyph.m_width * glyph.m_height];
        Point *points2 = new Point[glyph.m_width * glyph.m_height];

        for (int16_t y = 0; y < glyph.m_height; y++)
        {
            const size_t iy = (image.height - 1 - (glyph.m_y + y)) * image.width;
            for (int16_t x = 0; x < glyph.m_width; x++)
            {
                const size_t ix = glyph.m_x + x;
                const uint8_t *pixel = &image.data[(iy + ix) * image.bytespp];
                const bool set = pixel[0] > 0x7f;
                if (set)
                {
                    points1[y * glyph.m_width + x] = Point::Outside;
                    points2[y * glyph.m_width + x] = Point(0, 0);
                }
                else
                {
                    points1[y * glyph.m_width + x] = Point(0, 0);
                    points2[y * glyph.m_width + x] = Point::Outside;
                }
            }
        }

        Point::Generate(points1, glyph.m_width, glyph.m_height);
        Point::Generate(points2, glyph.m_width, glyph.m_height);

        const int16_t spread = Min(glyph.m_width, glyph.m_height);
        const int16_t min = -spread;
        const int16_t max = spread;

        for (int16_t y = 0; y < glyph.m_height; y++)
        {
            const size_t iy = (image.height - 1 - (glyph.m_y + y)) * image.width;
            for (int16_t x = 0; x < glyph.m_width; x++)
            {
                const Point &pt1 = points1[y * glyph.m_width + x];
                const Point &pt2 = points2[y * glyph.m_width + x];

                float dist = pt1.Dist() - pt2.Dist();
                dist = (dist < 0)
                    ? (-128 * (dist - min) / min)
                    : (128 + 128 * dist / max);

                const uint8_t value = Clamp<int16_t>(dist, 0, 255);

                const size_t ix = glyph.m_x + x;
                uint8_t *pixel = &data[(iy + ix) * image.bytespp];
                pixel[0] = value;
                pixel[1] = value;
                pixel[2] = value;
//                pixel[3] = value;
                pixel[3] = 0xff;
            }
        }

        delete[] points1;
        delete[] points2;

        return true;
    }

    static bool ConvertToDistanceField(const Font &font, const Image (&pageImages)[4], const size_t pageCount,
                                       const std::string &directory, const char (&pageFiles)[4][64])
    {
        uint8_t *data[4] = { nullptr };
        for (size_t i = 0; i < pageCount; i++)
        {
            const Image &image = pageImages[i];
            const size_t imageSize = image.width * image.height * image.bytespp;
            data[i] = new uint8_t[imageSize];
            std::memset(data[i], 0xff000000, imageSize);
        }

        for (size_t i = 0; i < font.GetGlyphCount(); i++)
        {
            const Glyph &glyph = font.GetGlyphByIndex(i);
            const Image &image = pageImages[glyph.m_textureIdx];
            ConvertGlyphToDf(glyph, image, data[glyph.m_textureIdx]);
        }

        for (size_t i = 0; i < pageCount; i++)
        {
            const Image &image = pageImages[i];
            const std::string filename = (directory + "/df." + pageFiles[i]);
            SaveImage(filename.c_str(), data[i], image.width, image.height, image.bytespp);
            delete[] data[i];
        }
        return true;
    }

} // rob
