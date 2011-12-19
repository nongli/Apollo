#include "Filesystem.h"
#include "Error.h"
#include <assert.h>
#ifdef UNREFERENCED_PARAMETER
#undef UNREFERENCED_PARAMETER
#endif
#include <Windows.h>
#include <sstream>

using namespace std;

namespace Apollo {

File File::Load(const string& path) {
    DWORD attributes = GetFileAttributes(&path[0]);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        throw ApolloException("File does not exist.");
    }

    File file;
    file.m_parent = nullptr;
    file.m_localpath = GetBaseName(path);
    file.m_completePath = path;
    file.m_isLeaf = (attributes & FILE_ATTRIBUTE_DIRECTORY) ? false : true;
    return file;
}

File::File(const string& path, File* parent) {
    stringstream stream;
    if (parent) {
        stream << parent->GetFullPath() << "\\";
    }
    stream << path;
        
    m_completePath = stream.str();
    m_localpath = path;
    m_parent = parent;

    DWORD attributes = GetFileAttributes(&m_completePath[0]);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        throw ApolloException("File does not exist.");
    }

    m_isLeaf = (attributes & FILE_ATTRIBUTE_DIRECTORY) ? false : true;

    if (parent) {
        parent->m_children.push_back(*this);
    }
}

string File::GetBaseName(const string& path) {
    size_t found = path.find_last_of("\\");
    if (found != string::npos && found != path.length()) {
        return path.substr(found + 1, path.length());
    }
    return "";
}

File::Type File::GetType() const {
    return m_isLeaf ? File::FILE : File::DIRECTORY;
}
    
string File::GetFileName() const {
    return m_localpath;
}

string File::GetFullPath() const {
    return m_completePath;
}

UINT File::AppendAllLeafChildren(vector<File>& files) const {
    UINT count = 0;
    AppendAllLeafChildren(files, count);
    return count;
}

void File::AppendAllLeafChildren(vector<File>& files, UINT& count) const {
    if (GetType() == File::FILE) {
        files.push_back(*this);
        count++;
    } else {
        vector<File>::const_iterator iter;
        for (iter = m_children.cbegin(); iter != m_children.cend(); iter++) {
            iter->AppendAllLeafChildren(files, count);
        }
    }
}

void FileSystem::TraverseFolder(File& root) {
    assert(root.GetType() == File::DIRECTORY);

    stringstream stream;
    stream << root.GetFullPath();
    stream << "\\*";

    string blah = stream.str();
    WIN32_FIND_DATA info;
    HANDLE dir = FindFirstFile(stream.str().c_str(), &info);
    if (dir == INVALID_HANDLE_VALUE) {
        throw ApolloException("Could not traverse folder.  Invalid path");
    }

    do {
        if (!(strcmp(info.cFileName, ".") == 0 || strcmp(info.cFileName, "..") == 0)) {
            File child(info.cFileName, &root);
            if (child.GetType() == File::DIRECTORY) {
                TraverseFolder(child);
            }
        }
    } while (FindNextFile(dir, &info));
    FindClose(dir);
}

}
