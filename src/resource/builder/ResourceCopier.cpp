
#include "ResourceCopier.h"
#include "../../filesystem/FileCopy.h"

namespace rob
{

    ResourceCopier::ResourceCopier()
    {
        m_extensions.push_back(".ion");
        m_extensions.push_back(".wav");
    }

    bool ResourceCopier::Build(const std::string &filename, const std::string &destFilename)
    {
        FileCopy(filename.c_str(), destFilename.c_str());
        return true;
    }

} // rob
