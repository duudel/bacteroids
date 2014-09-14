
#ifndef H_ROB_RESOURCE_COPIER_H
#define H_ROB_RESOURCE_COPIER_H

#include "ResourceBuilder.h"

namespace rob
{

    class ResourceCopier : public ResourceBuilder
    {
    public:
        ResourceCopier();
        virtual bool Build(const std::string &filename, const std::string &destFilename) override;
    };

} // rob

#endif // H_ROB_RESOURCE_COPIER_H

