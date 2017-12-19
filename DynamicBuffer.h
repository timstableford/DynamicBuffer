#ifndef __DYNAMIC_BUFFER_H__
#define __DYNAMIC_BUFFER_H__

// For compilation on Arduino to work.
#undef min
#undef max
#include <cstdint>
#include <functional>

#define SLOT_FREE (-1)

template <class T>
class GenericBuffer {
public:
    virtual T& operator[](int index) = 0;
    virtual uint16_t size() = 0;
};

template <class T>
class ArrayBufferWrapper : public GenericBuffer<T> {
public:
    ArrayBufferWrapper(T* data, uint16_t length) {
        m_data = data;
        m_length = length;
    }
    T& operator[](int index) {
        return m_data[index];
    }
    inline uint16_t size() {
        return m_length;
    }
private:
    T* m_data;
    uint16_t m_length;
};

template <class T>
class DynamicBuffer {
public:
    class Buffer : public GenericBuffer<T> {
        public:
            Buffer(DynamicBuffer<T> &buffer, int8_t slot) : m_buffer(buffer), m_slot(slot) {
                int8_t chunkCount = 0;
                for (int8_t i = 0; i < m_buffer.m_numChunks; i++) {
                    if (m_buffer.m_chunkMap[i] == m_slot) {
                        chunkCount++;
                    }
                }
                m_size = chunkCount * m_buffer.m_chunkSize;
            }

            T& operator[](int index) {
                int8_t chunkOffset = index / m_buffer.m_chunkSize;
                int bufferOffset = index % m_buffer.m_chunkSize;
                int8_t chunkCount = 0;
                for (int8_t i = 0; i < m_buffer.m_numChunks; i++) {
                    if (m_buffer.m_chunkMap[i] == m_slot) {
                        if (chunkOffset == chunkCount) {
                            return m_buffer.m_buffer[i * m_buffer.m_chunkSize + bufferOffset];
                        }
                        chunkCount++;
                    }
                }
                return const_cast<T&>(m_invalid);
            }

            void writeTo(std::function<void(T* data, uint16_t offset, uint16_t dataLength)> callback) {
                uint16_t chunkCount = 0;
                for (int8_t i = 0; i < m_buffer.m_numChunks; i++) {
                    if (m_buffer.m_chunkMap[i] == m_slot) {
                        callback(m_buffer.m_buffer + (i * m_buffer.m_chunkSize), chunkCount * m_buffer.m_chunkSize, m_buffer.m_chunkSize);
                        chunkCount++;
                    }
                }
            }

            inline uint16_t size() {
                return m_size;
            }
        private:
            DynamicBuffer<T> &m_buffer;
            int8_t m_slot;
            uint16_t m_size;
            const uint8_t m_invalid = 0;
    };
    DynamicBuffer(uint16_t chunkSize, int8_t numChunks) {
        m_buffer = new T[chunkSize * numChunks];
        m_chunkMap = new int8_t[numChunks];
        m_chunkSize = chunkSize;
        m_numChunks = numChunks;
        for (int8_t i = 0; i < m_numChunks; i++) {
            m_chunkMap[i] = SLOT_FREE;
        }
    }
    ~DynamicBuffer() {
        delete[] m_buffer;
    }

    Buffer getBuffer(int8_t slot) {
        return Buffer(*this, slot);
    }

    uint16_t getFree() {
        uint8_t numFree = 0;
        for (int8_t i = 0; i < m_numChunks; i++) {
            if (m_chunkMap[i] == SLOT_FREE) {
                numFree++;
            }
        }
        return numFree * m_chunkSize;
    }

    int8_t allocate(uint16_t size) {
        if (size > getFree()) {
            return SLOT_FREE;
        }
        int8_t slot = getFreeSlotNum();
        uint16_t allocated = 0;
        for (int8_t i = 0; i < m_numChunks && allocated < size; i++) {
            if (m_chunkMap[i] == SLOT_FREE) {
                m_chunkMap[i] = slot;
                allocated += m_chunkSize;
            }
        }
        return slot;
    }

    void free(int8_t slot) {
        for (int8_t i = 0; i < m_numChunks; i++) {
            if (m_chunkMap[i] == slot) {
                m_chunkMap[i] = SLOT_FREE;
            }
        }
    }

private:
    T *m_buffer;
    int8_t *m_chunkMap;
    uint16_t m_chunkSize;
    int8_t m_numChunks;

    bool isSlotUsed(int8_t slot) {
        for (int8_t i = 0; i < m_numChunks; i++) {
            if (m_chunkMap[i] == slot) {
                return true;
            }
        }
        return false;
    }

    int8_t getFreeSlotNum() {
        for (int8_t i = 0; i < m_numChunks; i++) {
            if (!isSlotUsed(i)) {
                return i;
            }
        }
        return SLOT_FREE;
    }
};

#endif // __DYNAMIC_BUFFER_H__
