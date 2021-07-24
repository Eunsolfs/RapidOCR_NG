#pragma once

#ifdef _WIN32
#define my_strtol wcstol
#define my_strrchr wcsrchr
#define my_strcasecmp _wcsicmp
#define my_strdup _strdup
#else
#define my_strtol strtol
#define my_strrchr strrchr
#define my_strcasecmp strcasecmp
#define my_strdup strdup
#endif

#ifdef WIN32
#include <codecvt>

inline std::wstring string2wstring(const std::string& str, const std::string& locale)
{
    typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
    std::wstring_convert<F> strCnv(new F(locale));
    return strCnv.from_bytes(str);
}

inline std::wstring  strToWstr(std::string str) {
    if (str.length() == 0)
        return L"";
  return  string2wstring(str, "zh-CN");
  
}

#endif


inline void getInputName(Ort::Session* session, string & inputName) {
    size_t numInputNodes = session->GetInputCount();
    if (numInputNodes > 0) {
        Ort::AllocatorWithDefaultOptions allocator;
        {
            char* t = session->GetInputName(0, allocator);
            inputName = t;
            allocator.Free(t);
        }
    }
}

inline void getOutputName(Ort::Session* session, string& outputName) {
    size_t numOutputNodes = session->GetInputCount();
    if (numOutputNodes > 0) {
        Ort::AllocatorWithDefaultOptions allocator;
        {
            char* t = session->GetOutputName(0, allocator);
            outputName = t;
            allocator.Free(t);
        }
    }
}


inline cv::Mat makePadding(cv::Mat& src, const int padding) {
    if (padding <= 0) return src;
    cv::Scalar paddingScalar = { 255, 255, 255 };
    cv::Mat paddingSrc;
    cv::copyMakeBorder(src, paddingSrc, padding, padding, padding, padding, cv::BORDER_ISOLATED, paddingScalar);
    return paddingSrc;
}