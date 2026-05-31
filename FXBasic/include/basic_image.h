#ifndef _BASIC_IMAGE_H_
#define _BASIC_IMAGE_H_

#include <assert.h>
#include "basic_log.h"

namespace FX {

    template<typename T = unsigned char>
    class BasicImage {
    public:
        BasicImage(void) = default;

        BasicImage::BasicImage(unsigned int width, unsigned int height, unsigned char channels, T* pData, bool reference = false)
        {
            if (width == 0 || height == 0 || channels == 0 || (reference && pData == nullptr))
            {
                BasicLog::out(BasicLog::kWarn, "Invalid data when creating image.");
                return;
            }

            m_width = width;
            m_height = height;
            m_channels = channels;
            m_reference = reference;

            accept(width * height * channels, pData, reference);
        }

        BasicImage::~BasicImage(void)
        {
            release();
        }

        void BasicImage::setData(unsigned int width, unsigned int height, unsigned char channels, T* pData, bool reference = false)
        {
            if (width == 0 || height == 0 || channels == 0 || (reference && pData == nullptr))
            {
                BasicLog::out(BasicLog::kWarn, "Invalid data when modifying image.");
                return;
            }

            release();

            m_width = width;
            m_height = height;
            m_channels = channels;
            m_reference = reference;

            accept(width * height * channels, pData, reference);
        }

        unsigned int BasicImage::width(void) const
        {
            return m_width;
        }

        unsigned int BasicImage::height(void) const
        {
            return m_height;
        }

        unsigned char BasicImage::channels(void) const
        {
            return m_channels;
        }

        bool isReference(void) const
        {
            return m_reference;
        }

        const T* BasicImage::data(void) const
        {
            return m_pData;
        }

        bool BasicImage::valid(void) const
        {
            return m_width > 0 && m_height > 0 && m_channels > 0 && m_pData != nullptr;
        }

        BasicImage::BasicImage(const BasicImage& other)
        {
            m_reference = other.m_reference;

            if (other.valid())
            {
                m_width = other.m_width;
                m_height = other.m_height;
                m_channels = other.m_channels;
                accept(other.m_width * other.m_height * other.m_channels, other.m_pData, other.m_reference);
            }
        }

        BasicImage& BasicImage::operator=(const BasicImage& other)
        {
            if (this == &other)
            {
                return *this;
            }

            release();

            m_reference = other.m_reference;

            if (other.valid())
            {
                m_width = other.m_width;
                m_height = other.m_height;
                m_channels = other.m_channels;
                accept(other.m_width * other.m_height * other.m_channels, other.m_pData, other.m_reference);
            }
            else
            {
                m_width = m_height = 0;
                m_channels = 0;
            }

            return *this;
        }

        BasicImage::BasicImage(BasicImage&& other)
        {
            m_reference = other.m_reference;

            if (other.valid())
            {
                m_width = other.m_width;
                m_height = other.m_height;
                m_channels = other.m_channels;
                accept(other.m_width * other.m_height * other.m_channels, other.m_pData, other.m_reference);
                other.m_pData = nullptr;
            }
        }

        BasicImage& BasicImage::operator=(BasicImage&& other)
        {
            if (this == &other)
            {
                return *this;
            }

            release();

            m_reference = other.m_reference;

            if (other.valid())
            {
                m_width = other.m_width;
                m_height = other.m_height;
                m_channels = other.m_channels;
                accept(other.m_width * other.m_height * other.m_channels, other.m_pData, other.m_reference);
                other.m_pData = nullptr;
            }
            else
            {
                m_width = m_height = 0;
                m_channels = 0;
            }

            return *this;
        }

    private:
        void BasicImage::accept(unsigned int pixels, T* pData, bool reference)
        {
            if (reference)
            {
                assert(pData != nullptr);
                m_pData = pData;
            }
            else
            {
                assert(m_pData == nullptr);
                m_pData = new T[pixels];
                pData == nullptr ? memset(m_pData, 0, pixels * sizeof(T)) : memcpy(m_pData, pData, pixels * sizeof(T));
            }
        }

        void BasicImage::release(void)
        {
            if (m_reference == false && m_pData != nullptr)
            {
                delete[] m_pData;
                m_pData = nullptr;
            }
        }

    private:
        T* m_pData = nullptr;
        unsigned int m_width = 0;
        unsigned int m_height = 0;
        unsigned char m_channels = 0;
        bool m_reference = true;
    };

} // namespace FX

#endif // _BASIC_IMAGE_H_
