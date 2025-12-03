#ifndef _BASIC_IMAGE_H_
#define _BASIC_IMAGE_H_

#include <memory>

namespace FX {

    class BasicImage {
    public:
        BasicImage(void) = default;
        BasicImage(unsigned int width, unsigned int height, unsigned char channels, const unsigned char* pData);
        ~BasicImage(void) = default;

        void setData(unsigned int width, unsigned int height, unsigned char channels, const unsigned char* pData);

        unsigned int width(void) const;
        unsigned int height(void) const;
        unsigned char channels(void) const;
        const unsigned char* data(void) const;

        bool valid(void) const;

        BasicImage(const BasicImage& other);
        BasicImage& operator=(const BasicImage& other);

    private:
        std::unique_ptr<unsigned char> m_pData = nullptr;
        unsigned int m_width = 0;
        unsigned int m_height = 0;
        unsigned char m_channels = 0;
    };

} // namespace FX

#endif // _BASIC_IMAGE_H_
