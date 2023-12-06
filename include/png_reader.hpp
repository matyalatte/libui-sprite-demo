#pragma once
#include <stdint.h>

class PngReader {
 private:
    unsigned char *m_data;
    int m_width;
    int m_height;
    int m_has_alpha;

 public:
    PngReader() : m_data(NULL), m_width(0), m_height(0), m_has_alpha(0) {}

    ~PngReader()
    {
        if (m_data)
            free(m_data);
    }

    unsigned char *GetData() { return m_data; }

    void GetSize(int *width, int *height)
    {
        *width = m_width;
        *height = m_height;
    }

    int HasAlpha() { return m_has_alpha; }

    int ReadFromFile(const char* file_name);
};
