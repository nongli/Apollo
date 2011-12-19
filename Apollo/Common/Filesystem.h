#pragma once

#include <string>
#include <vector>
#include "Types.h"

namespace Apollo {
    class File {
    public:
        enum Type {
            DIRECTORY,
            FILE,
        };

    public:
        static File Load(const std::string& path);
        File(const std::string& path, File* parent=nullptr);

        Type GetType() const;
        std::string GetFileName() const;
        std::string GetFullPath() const;

        UINT AppendAllLeafChildren(std::vector<File>& files) const;

    public:
        static std::string GetBaseName(const std::string& path);

    private:
        void AppendAllLeafChildren(std::vector<File>& files, UINT& count) const;
        File() {}

    private:
        bool m_isLeaf;
        std::string m_localpath;
        std::string m_completePath;
        std::vector<File> m_children;
        File* m_parent;
    };

    class FileSystem {
    public:
        static void TraverseFolder(File&);
    };
}
