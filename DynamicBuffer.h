#ifndef __DYNAMIC_BUFFER_H__
#define __DYNAMIC_BUFFER_H__

// For compilation on Arduino to work.
#undef min
#undef max
#include <cstdint>
#include <functional>

#define SLOT_FREE (-1)

/**
  * \brief Generic class to access various buffer types.
  *        This class implements the dynamic fragmented buffer type and an array wrapper.
  */
template <class T>
class GenericBuffer {
public:
    /**
      * \brief Array accessor operator.
      * \param index The index to lookup.
      * \return Reference to the array element.
      */
    virtual T& operator[](int index) = 0;
    virtual uint16_t size() = 0;
};

/**
  * \brief Wraps a standard array in a GenericBuffer for use with other classes.
  */
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

/**
  * \brief This class allows allocating slots in a fixed size buffer and provides a class
           that allows the fragmented buffer slot to be accessed sequentially.
           This means that no matter how many variable sizes allocs and frees are done on it
           fragmentation will only ever be an O(n) issue.
  */
template <class T>
class DynamicBuffer {
public:
    /**
      * \brief Accessor class to access DynamicBuffers underlying slots. This uses
               GenericBuffer for compatibility with other utility classes.
      */
    class Buffer : public GenericBuffer<T> {
        public:
            /**
              * \brief This constructor should only be called by the DynamicBuffer class.
                       It can however be manually constructed if required.
                \param buffer A reference to the dynamic buffer object containing the allocated
                       slot and memory.
                \param slot The slot to access in the underlying DynamicBuffer.
              */
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
    /**
      * \brief Created a DynamicBuffer class. This function calls new so
               constructing this object should only be done on startup for
               memory constrained devices. The size of the memory allocated is
               chunkSize * numChunks * sizeof(T).
        \param chunkSize The number of objects to allocate to each chunk.
               For representing structures and classes this could be 1.
               When representing arrays of types such as char[] chunk size would
               be larger.
        \param numChunks The number of chunks to allocate. The maximum number of
               chunks is also equivalent to the maximum number of slots.
      */
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

    /**
      * \brief Constructs a buffer accessor class for the given slot.
      * \return The constructed buffer. This may or may not be valid.
                The only way of knowing is to call isSlotUsed before hand to
                make sure some memory is allocated to that slot.
      */
    Buffer getBuffer(int8_t slot) {
        return Buffer(*this, slot);
    }

    /**
      * \brief Gets the free data in the buffer.
      * \return The free data is represented by numChunks * chunkSize.
                It may not represent bytes, unless of course T is int8_t or uint8_t.
      */
    uint16_t getFree() {
        uint8_t numFree = 0;
        for (int8_t i = 0; i < m_numChunks; i++) {
            if (m_chunkMap[i] == SLOT_FREE) {
                numFree++;
            }
        }
        return numFree * m_chunkSize;
    }

    /**
      * \brief Allocates a slot of the given size.
      * \param size Size of the number of elements to allocate.
      * \return SLOT_FREE on failure, or otherwise a valid slot > 0.
      */
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

    /**
      * \brief De-allocates a given slot marking it as available.
      * \param slot The slot to free.
      */
    void free(int8_t slot) {
        for (int8_t i = 0; i < m_numChunks; i++) {
            if (m_chunkMap[i] == slot) {
                m_chunkMap[i] = SLOT_FREE;
            }
        }
    }

    /**
      * \brief Calculates whether any memory is allocated for the given slot number.
      * \return True if any slots have been assigned the given slot number.
      */
    bool isSlotUsed(int8_t slot) {
        for (int8_t i = 0; i < m_numChunks; i++) {
            if (m_chunkMap[i] == slot) {
                return true;
            }
        }
        return false;
    }

private:
    T *m_buffer;
    int8_t *m_chunkMap;
    uint16_t m_chunkSize;
    int8_t m_numChunks;

    /**
      * \brief Calculates the first unused slot number.
      */
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
