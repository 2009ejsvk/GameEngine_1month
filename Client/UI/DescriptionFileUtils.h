#pragma once

#include <Windows.h>
#include <cstdio>
#include <string>

namespace DescriptionFileUtils
{
    inline std::wstring TrimTrailingSeparators(const std::wstring& Path)
    {
        size_t End = Path.size();

        while (End > 0 &&
            (Path[End - 1] == L'\\' || Path[End - 1] == L'/'))
        {
            --End;
        }

        return Path.substr(0, End);
    }

    inline std::wstring GetParentDirectoryPath(const TCHAR* FullPath)
    {
        if (!FullPath || !*FullPath)
            return std::wstring();

        const std::wstring TrimmedPath = TrimTrailingSeparators(FullPath);
        const size_t SeparatorIndex =
            TrimmedPath.find_last_of(L"\\/");

        if (SeparatorIndex == std::wstring::npos)
            return std::wstring();

        return TrimmedPath.substr(0, SeparatorIndex);
    }

    inline std::wstring JoinPath(
        const std::wstring& BasePath,
        const wchar_t* Suffix)
    {
        if (BasePath.empty())
            return std::wstring(Suffix ? Suffix : L"");

        std::wstring Result = TrimTrailingSeparators(BasePath);

        if (!Suffix || !*Suffix)
            return Result;

        Result += L"\\";
        Result += Suffix;
        return Result;
    }

    inline bool LoadUtf8TextFile(
        const std::wstring& FullPath,
        std::string& OutText)
    {
        OutText.clear();

        FILE* File = nullptr;
        _wfopen_s(&File, FullPath.c_str(), L"rb");

        if (!File)
            return false;

        fseek(File, 0, SEEK_END);
        const long FileSize = ftell(File);
        fseek(File, 0, SEEK_SET);

        if (FileSize < 0)
        {
            fclose(File);
            return false;
        }

        OutText.resize(static_cast<size_t>(FileSize));

        if (FileSize > 0)
        {
            const size_t ReadSize = fread(
                &OutText[0],
                1,
                static_cast<size_t>(FileSize),
                File);

            if (ReadSize != static_cast<size_t>(FileSize))
            {
                fclose(File);
                OutText.clear();
                return false;
            }
        }

        fclose(File);

        if (OutText.size() >= 3 &&
            static_cast<unsigned char>(OutText[0]) == 0xEF &&
            static_cast<unsigned char>(OutText[1]) == 0xBB &&
            static_cast<unsigned char>(OutText[2]) == 0xBF)
        {
            OutText.erase(0, 3);
        }

        return true;
    }
}
