
#ifndef H_ROB_RESOURCE_BUILDER_H
#define H_ROB_RESOURCE_BUILDER_H

#include <string>
#include <vector>

namespace rob
{

    class ResourceBuilder
    {
    public:
        ResourceBuilder();
        virtual ~ResourceBuilder() { }

        bool TryBuild(const std::string &filename, const std::string &destFilename);
        virtual bool Build(const std::string &filename, const std::string &destFilename) = 0;

    protected:
        std::vector<std::string> m_extensions;
        std::string m_newExtension;
        ResourceBuilder *m_next;

        friend class MasterBuilder;
        static ResourceBuilder *m_head;
        static ResourceBuilder *m_tail;
    };

} // rob

#endif // H_ROB_RESOURCE_BUILDER_H

