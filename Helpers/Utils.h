#pragma once
#include <d3d11.h>
#include <string>
#include <vector>
#include <Windows.h>
#include <chrono>
#include <iostream>

namespace Utils {
    inline LPCWSTR GetMessageFromHr(HRESULT hr)
    {
        LPVOID errorMsg;
        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        DWORD langId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

        if (FormatMessage(flags, NULL, hr, langId, (LPWSTR)&errorMsg, 0, NULL) != 0)
        {
            return (LPCWSTR)errorMsg;
        }
        return L"Unknown error";
        
    }
    
    inline LPCWSTR GetMessageFromBlob(const char* c)
    {
        size_t length = strlen(c) + 1;
        wchar_t* wText = new wchar_t[length];
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wText, length, c, _TRUNCATE);
        return wText;
    }
    
    inline void UnloadD3D11Resource(ID3D11Resource** resource)
    {
        if(*resource)
        {
            (*resource)->Release();
        }
        *resource = nullptr;
    }
    
    inline std::vector<std::string> Split(const std::string& str, const char character)
    {
        std::vector<std::string> result;
        int ref = 0;
        for (int i = 0; i < str.length(); ++i)
        {
            if(str[i] == character)
            {
                result.push_back(str.substr(ref, i - ref));
                ref = i + 1;
            }
            if(i == str.length() - 1)
            {
                result.push_back(str.substr(ref, str.length() - ref));
            }
        }

        return result;
    }
    
    template <typename T>
    inline T* CopyArray(const T* other, uint32_t numElements)
    {
        if (other) {
            T* newArray = new T[numElements];
            std::memcpy(newArray, other, numElements * sizeof(T));
            return newArray;
        }
        return nullptr;
    }

    static std::chrono::time_point<std::chrono::steady_clock> timeStamp;
    inline void StartTimeMeasure()
    {
        timeStamp = std::chrono::steady_clock::now();
    }
        
    inline void StopTimeMeasure(std::string message)
    {
        auto time = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(time - timeStamp).count();
        std::string units = "microseconds";

        if(elapsedTime > 1000)
        {
            elapsedTime = elapsedTime / 1000;
            units = "milliseconds";
        }
        if(elapsedTime > 1000)
        {
            elapsedTime = elapsedTime / 1000;
            units = "seconds";
        }

        std::cout << message << " took: " << elapsedTime << ' ' << units << " to complete" << std::endl;
    }
}