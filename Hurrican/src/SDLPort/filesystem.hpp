#ifndef _HURRICAN_FILESYSTEM_HPP_
#define _HURRICAN_FILESYSTEM_HPP_

#include <cstdint>
#include <filesystem>

#if defined(AMIGA_MINIGL)

// libstdc++ for m68k-amigaos does not get file status right on libnix
// (every file looks missing), so the status queries Hurrican uses are done
// with dos.library here. Everything else, fs::path included, is the
// standard one: a name declared in this namespace hides the std version.
namespace amigafs {

using namespace std::filesystem;

// fib_DirEntryType of p: < 0 file, > 0 drawer, 0 if it cannot be examined
inline long EntryType(const path &p, std::uintmax_t *size = nullptr) {
    BPTR lock = Lock(p.c_str(), ACCESS_READ);
    if (!lock)
        return 0;

    long type = 0;
    auto *fib = static_cast<FileInfoBlock *>(AllocDosObject(DOS_FIB, nullptr));
    if (fib && Examine(lock, fib)) {
        type = fib->fib_DirEntryType;
        if (size)
            *size = fib->fib_Size;
    }
    if (fib)
        FreeDosObject(DOS_FIB, fib);
    UnLock(lock);
    return type;
}

inline bool exists(const path &p) {
    return EntryType(p) != 0;
}

inline bool is_regular_file(const path &p) {
    return EntryType(p) < 0;
}

inline bool is_directory(const path &p) {
    return EntryType(p) > 0;
}

inline std::uintmax_t file_size(const path &p) {
    std::uintmax_t size = 0;
    if (EntryType(p, &size) >= 0)
        return static_cast<std::uintmax_t>(-1);
    return size;
}

inline bool remove(const path &p) {
    return DeleteFile(p.c_str()) != 0;
}

}  // namespace amigafs

namespace fs = amigafs;

#else

namespace fs = std::filesystem;

#endif

#endif  // _HURRICAN_FILESYSTEM_HPP_
