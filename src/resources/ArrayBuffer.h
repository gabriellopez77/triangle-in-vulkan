#pragma once

#include <cassert>

#include "Defs.h"


namespace resources {
    template<typename T>
    class ArrayBuffer {
    public:
        ArrayBuffer(i32 length) : m_maxSize(length) {
            m_data = new T[length];
        }
        ~ArrayBuffer() { delete[] m_data; }


        i32 length() const { return m_length; }
        i32 sizeInBytes() const { return m_length * sizeof(T); }

        const T* data() const { return m_data; }

        void clear() { m_length = 0; }

        T& getEnd() {
            i32 index = m_length++;
            assert(index < m_maxSize);
            return m_data[index];
        }

        void addRange(const ArrayBuffer& other)
        {
            i32 itemsCount = other.m_length;
            auto otherData = other.m_data;

            if (m_length + itemsCount >= m_maxSize)
                itemsCount = m_maxSize - m_length;

            if (itemsCount > other.m_length)
                assert(false && "this itemsCount is greater than other size");


            for (i32 i = 0; i < itemsCount; i++)
                m_data[i] = otherData[i];

            m_length += itemsCount;
        }

        T& operator[](i32 i) { return m_data[i]; };
        const T& operator[](i32 i) const { return m_data[i]; };

    private:
        T* m_data;

        const i32 m_maxSize;
        i32 m_length = 0;
    };
}