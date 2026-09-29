#include <cstdlib>
#include <fstream>
#include <sgl/save/SaveFile.hpp>
#include <system_error>

namespace sgl
{

std::expected<std::string, SaveError> readTextFile(const std::filesystem::path& path)
{
    std::error_code ec;
    if(!std::filesystem::exists(path, ec))
    {
        if(ec)
        {
            return std::unexpected(SaveError::Io);
        }
        return std::unexpected(SaveError::NotFound);
    }

    std::ifstream in(path, std::ios::binary);
    if(!in)
    {
        return std::unexpected(SaveError::Io);
    }

    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if(!in.good() && !in.eof())
    {
        return std::unexpected(SaveError::Io);
    }
    return contents;
}

std::expected<void, SaveError> writeTextFileAtomic(const std::filesystem::path& path, std::string_view contents)
{
    const std::filesystem::path tmpPath = std::filesystem::path(path.native() + std::filesystem::path(".tmp").native());

    {
        std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
        if(!out)
        {
            return std::unexpected(SaveError::Io);
        }
        out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        if(!out)
        {
            return std::unexpected(SaveError::Io);
        }
    }

    std::error_code ec;
    std::filesystem::rename(tmpPath, path, ec);
    if(ec)
    {
        std::error_code removeTargetEc;
        std::filesystem::remove(path, removeTargetEc);
        ec.clear();
        std::filesystem::rename(tmpPath, path, ec);
        if(ec)
        {
            std::error_code removeTmpEc;
            std::filesystem::remove(tmpPath, removeTmpEc);
            return std::unexpected(SaveError::Io);
        }
    }
    return {};
}

std::expected<std::filesystem::path, SaveError> userDataDir(std::string_view app)
{
#if defined(_WIN32)
    #if defined(_MSC_VER)
        #pragma warning(push)
        #pragma warning(disable : 4996) // getenv; _dupenv_s would split this from the POSIX path
    #endif
    const char* appData = std::getenv("APPDATA");
    #if defined(_MSC_VER)
        #pragma warning(pop)
    #endif
    if(appData == nullptr || appData[0] == '\0')
    {
        return std::unexpected(SaveError::Io);
    }
    return std::filesystem::path(appData) / std::filesystem::path(app);
#else
    if(const char* xdg = std::getenv("XDG_DATA_HOME"); xdg != nullptr && xdg[0] != '\0')
    {
        return std::filesystem::path(xdg) / std::filesystem::path(app);
    }
    const char* home = std::getenv("HOME");
    if(home == nullptr || home[0] == '\0')
    {
        return std::unexpected(SaveError::Io);
    }
    return std::filesystem::path(home) / ".local" / "share" / std::filesystem::path(app);
#endif
}

} // namespace sgl
