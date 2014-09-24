
#ifndef H_ROB_DF_FONT_BUILDER_H
#define H_ROB_DF_FONT_BUILDER_H

#include "ResourceBuilder.h"

namespace rob
{

    class DfFontBuilder : public ResourceBuilder
    {
    public:
        DfFontBuilder();
        bool Build(const std::string &directory, const std::string &filename,
                   const std::string &destDirectory, const std::string &destFilename) override;
    };

} // rob

#endif // H_ROB_DF_FONT_BUILDER_H

