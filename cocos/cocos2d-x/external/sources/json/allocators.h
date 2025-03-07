
#ifndef RAPIDJSON_ALLOCATORS_H_
#define RAPIDJSON_ALLOCATORS_H_

#include "rapidjson.h"

RAPIDJSON_NAMESPACE_BEGIN


/*! \class rapidjson::Allocator
    \brief Concept for allocating, resizing and freeing memory block.
    
    Note that Malloc() and Realloc() are non-static but Free() is static.
    
    So if an allocator need to support Free(), it needs to put its pointer in 
    the header of memory block.

\code
concept Allocator {
    static const bool kNeedFree;    //!< Whether this allocator needs to call Free().

    void* Malloc(size_t size);

    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize);

    static void Free(void *ptr);
};
\endcode
*/


/*! This class is just wrapper for standard C library memory routines.
    \note implements Allocator concept
*/
class CrtAllocator {
public:
    static const bool kNeedFree = true;
    void* Malloc(size_t size) { 
        if (size) //  behavior of malloc(0) is implementation defined.
            return std::malloc(size);
        else
            return NULL; // standardize to returning NULL.
    }
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) {
        (void)originalSize;
        if (newSize == 0) {
            std::free(originalPtr);
            return NULL;
        }
        return std::realloc(originalPtr, newSize);
    }
    static void Free(void *ptr) { std::free(ptr); }
};


/*! This allocator allocate memory blocks from pre-allocated memory chunks. 

    It does not free memory blocks. And Realloc() only allocate new memory.

    The memory chunks are allocated by BaseAllocator, which is CrtAllocator by default.

    User may also supply a buffer as the first chunk.

    If the user-buffer is full then additional chunks are allocated by BaseAllocator.

    The user-buffer is not deallocated by this allocator.

    \tparam BaseAllocator the allocator type for allocating memory chunks. Default is CrtAllocator.
    \note implements Allocator concept
*/
template <typename BaseAllocator = CrtAllocator>
class MemoryPoolAllocator {
public:
    static const bool kNeedFree = false;    //!< Tell users that no need to call Free() with this allocator. (concept Allocator)

    /*! \param chunkSize The size of memory chunk. The default is kDefaultChunkSize.
        \param baseAllocator The allocator for allocating memory chunks.
    */
    MemoryPoolAllocator(size_t chunkSize = kDefaultChunkCapacity, BaseAllocator* baseAllocator = 0) : 
        chunkHead_(0), chunk_capacity_(chunkSize), userBuffer_(0), baseAllocator_(baseAllocator), ownBaseAllocator_(0)
    {
    }

    /*! The user buffer will be used firstly. When it is full, memory pool allocates new chunk with chunk size.

        The user buffer will not be deallocated when this allocator is destructed.

        \param buffer User supplied buffer.
        \param size Size of the buffer in bytes. It must at least larger than sizeof(ChunkHeader).
        \param chunkSize The size of memory chunk. The default is kDefaultChunkSize.
        \param baseAllocator The allocator for allocating memory chunks.
    */
    MemoryPoolAllocator(void *buffer, size_t size, size_t chunkSize = kDefaultChunkCapacity, BaseAllocator* baseAllocator = 0) :
        chunkHead_(0), chunk_capacity_(chunkSize), userBuffer_(buffer), baseAllocator_(baseAllocator), ownBaseAllocator_(0)
    {
        RAPIDJSON_ASSERT(buffer != 0);
        RAPIDJSON_ASSERT(size > sizeof(ChunkHeader));
        chunkHead_ = reinterpret_cast<ChunkHeader*>(buffer);
        chunkHead_->capacity = size - sizeof(ChunkHeader);
        chunkHead_->size = 0;
        chunkHead_->next = 0;
    }

    /*! This deallocates all memory chunks, excluding the user-supplied buffer.
    */
    ~MemoryPoolAllocator() {
        Clear();
        RAPIDJSON_DELETE(ownBaseAllocator_);
    }

    void Clear() {
        while (chunkHead_ && chunkHead_ != userBuffer_) {
            ChunkHeader* next = chunkHead_->next;
            baseAllocator_->Free(chunkHead_);
            chunkHead_ = next;
        }
        if (chunkHead_ && chunkHead_ == userBuffer_)
            chunkHead_->size = 0; // Clear user buffer
    }

    /*! \return total capacity in bytes.
    */
    size_t Capacity() const {
        size_t capacity = 0;
        for (ChunkHeader* c = chunkHead_; c != 0; c = c->next)
            capacity += c->capacity;
        return capacity;
    }

    /*! \return total used bytes.
    */
    size_t Size() const {
        size_t size = 0;
        for (ChunkHeader* c = chunkHead_; c != 0; c = c->next)
            size += c->size;
        return size;
    }

    void* Malloc(size_t size) {
        if (!size)
            return NULL;

        size = RAPIDJSON_ALIGN(size);
        if (chunkHead_ == 0 || chunkHead_->size + size > chunkHead_->capacity)
            if (!AddChunk(chunk_capacity_ > size ? chunk_capacity_ : size))
                return NULL;

        void *buffer = reinterpret_cast<char *>(chunkHead_) + RAPIDJSON_ALIGN(sizeof(ChunkHeader)) + chunkHead_->size;
        chunkHead_->size += size;
        return buffer;
    }

    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) {
        if (originalPtr == 0)
            return Malloc(newSize);

        if (newSize == 0)
            return NULL;

        originalSize = RAPIDJSON_ALIGN(originalSize);
        newSize = RAPIDJSON_ALIGN(newSize);

        if (originalSize >= newSize)
            return originalPtr;

        if (originalPtr == reinterpret_cast<char *>(chunkHead_) + RAPIDJSON_ALIGN(sizeof(ChunkHeader)) + chunkHead_->size - originalSize) {
            size_t increment = static_cast<size_t>(newSize - originalSize);
            if (chunkHead_->size + increment <= chunkHead_->capacity) {
                chunkHead_->size += increment;
                return originalPtr;
            }
        }

        if (void* newBuffer = Malloc(newSize)) {
            if (originalSize)
                std::memcpy(newBuffer, originalPtr, originalSize);
            return newBuffer;
        }
        else
            return NULL;
    }

    static void Free(void *ptr) { (void)ptr; } // Do nothing

private:
    MemoryPoolAllocator(const MemoryPoolAllocator& rhs) /* = delete */;
    MemoryPoolAllocator& operator=(const MemoryPoolAllocator& rhs) /* = delete */;

    /*! \param capacity Capacity of the chunk in bytes.
        \return true if success.
    */
    bool AddChunk(size_t capacity) {
        if (!baseAllocator_)
            ownBaseAllocator_ = baseAllocator_ = RAPIDJSON_NEW(BaseAllocator());
        if (ChunkHeader* chunk = reinterpret_cast<ChunkHeader*>(baseAllocator_->Malloc(RAPIDJSON_ALIGN(sizeof(ChunkHeader)) + capacity))) {
            chunk->capacity = capacity;
            chunk->size = 0;
            chunk->next = chunkHead_;
            chunkHead_ =  chunk;
            return true;
        }
        else
            return false;
    }

    static const int kDefaultChunkCapacity = 64 * 1024; //!< Default chunk capacity.

    /*! Chunks are stored as a singly linked list.
    */
    struct ChunkHeader {
        size_t capacity;    //!< Capacity of the chunk in bytes (excluding the header itself).
        size_t size;        //!< Current size of allocated memory in bytes.
        ChunkHeader *next;  //!< Next chunk in the linked list.
    };

    ChunkHeader *chunkHead_;    //!< Head of the chunk linked-list. Only the head chunk serves allocation.
    size_t chunk_capacity_;     //!< The minimum capacity of chunk when they are allocated.
    void *userBuffer_;          //!< User supplied buffer.
    BaseAllocator* baseAllocator_;  //!< base allocator for allocating memory chunks.
    BaseAllocator* ownBaseAllocator_;   //!< base allocator created by this object.
};

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_ENCODINGS_H_
