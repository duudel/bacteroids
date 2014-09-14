
#ifndef H_ROB_TEXTURE_BUILDER_H
#define H_ROB_TEXTURE_BUILDER_H

#include "ResourceBuilder.h"

namespace rob
{

    class TextureBuilder : public ResourceBuilder
    {
    public:
        TextureBuilder();
        virtual bool Build(const std::string &filename, const std::string &destFilename) override;
    };

} // rob

#endif // H_ROB_TEXTURE_BUILDER_H

