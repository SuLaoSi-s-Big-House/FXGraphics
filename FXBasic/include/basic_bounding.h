#ifndef _BASIC_BOUNDING_H_
#define _BASIC_BOUNDING_H_

#include <limits>
#include <algorithm>
#include "basic_vector.h"

namespace FX {

    template<typename T = float>
    struct BasicBounding {

        BasicBounding(void)
        {
            reset();
        }

        BasicBounding(const vec3<T>& point)
        {
            m_max = m_min = point;
        }

        bool operator==(const BasicBounding& other) const
        {
            return m_max == other.m_max && m_min == other.m_min;
        }

        bool operator!=(const BasicBounding& other) const
        {
            return !(*this == other);
        }

        bool valid(void) const
        {
            return m_max.x >= m_min.x && m_max.y >= m_min.y && m_max.z >= m_min.z;
        }

        void expand(const vec3<T>& point)
        {
            m_min.x = std::min(point.x, m_min.x);
            m_min.y = std::min(point.y, m_min.y);
            m_min.z = std::min(point.z, m_min.z);
            m_max.x = std::max(point.x, m_max.x);
            m_max.y = std::max(point.y, m_max.y);
            m_max.z = std::max(point.z, m_max.z);
        }

        void expand(const BasicBounding& other)
        {
            m_min.x = std::min(other.m_min.x, m_min.x);
            m_min.y = std::min(other.m_min.y, m_min.y);
            m_min.z = std::min(other.m_min.z, m_min.z);
            m_max.x = std::max(other.m_max.x, m_max.x);
            m_max.y = std::max(other.m_max.y, m_max.y);
            m_max.z = std::max(other.m_max.z, m_max.z);
        }

        vec3<T> center(void) const
        {
            return vec3<T>{(m_max.x + m_min.x) / 2, (m_max.y + m_min.y) / 2, (m_max.z + m_min.z) / 2};
        }

        void reset(void)
        {
            m_max.x = m_max.y = m_max.z = std::numeric_limits<T>::lowest();
            m_min.x = m_min.y = m_min.z = std::numeric_limits<T>::max();
        }

        bool contains(const vec3<T>& point) const
        {
            return point.x <= m_max.x && point.x >= m_min.x &&
                point.y <= m_max.y && point.y >= m_min.y &&
                point.z <= m_max.z && point.z >= m_min.z;
        }

        bool contains(const BasicBounding& other) const
        {
            return other.m_max.x <= m_max.x && other.m_min.x >= m_min.x &&
                other.m_max.y <= m_max.y && other.m_min.y >= m_min.y &&
                other.m_max.z <= m_max.z && other.m_min.z >= m_min.z;
        }

        vec3<T> m_max;
        vec3<T> m_min;
    };

} // namespace FX

#endif // _BASIC_BOUNDING_H_
