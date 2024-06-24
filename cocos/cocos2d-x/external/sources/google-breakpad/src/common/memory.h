
#ifndef GOOGLE_BREAKPAD_COMMON_MEMORY_H_
#define GOOGLE_BREAKPAD_COMMON_MEMORY_H_

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include <memory>
#include <vector>

#if defined(MEMORY_SANITIZER)
#include <sanitizer/msan_interface.h>
#endif

#ifdef __APPLE__
#define sys_mmap mmap
#define sys_mmap2 mmap
#define sys_munmap munmap
#define MAP_ANONYMOUS MAP_ANON
#else
#include "third_party/lss/linux_syscall_support.h"
#endif

namespace google_breakpad {

class PageAllocator {
 public:
  PageAllocator()
      : page_size_(getpagesize()),
        last_(NULL),
        current_page_(NULL),
        page_offset_(0) {
  }

  ~PageAllocator() {
    FreeAll();
  }

  void *Alloc(size_t bytes) {
    if (!bytes)
      return NULL;

    if (current_page_ && page_size_ - page_offset_ >= bytes) {
      uint8_t *const ret = current_page_ + page_offset_;
      page_offset_ += bytes;
      if (page_offset_ == page_size_) {
        page_offset_ = 0;
        current_page_ = NULL;
      }

      return ret;
    }

    const size_t pages =
        (bytes + sizeof(PageHeader) + page_size_ - 1) / page_size_;
    uint8_t *const ret = GetNPages(pages);
    if (!ret)
      return NULL;

    page_offset_ =
        (page_size_ - (page_size_ * pages - (bytes + sizeof(PageHeader)))) %
        page_size_;
    current_page_ = page_offset_ ? ret + page_size_ * (pages - 1) : NULL;

    return ret + sizeof(PageHeader);
  }

  bool OwnsPointer(const void* p) {
    for (PageHeader* header = last_; header; header = header->next) {
      const char* current = reinterpret_cast<char*>(header);
      if ((p >= current) && (p < current + header->num_pages * page_size_))
        return true;
    }

    return false;
  }

 private:
  uint8_t *GetNPages(size_t num_pages) {
#if defined(__x86_64__) || defined(__aarch64__) || defined(__aarch64__) || \
    ((defined(__mips__) && _MIPS_SIM == _ABI64))
    void *a = sys_mmap(NULL, page_size_ * num_pages, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
    void *a = sys_mmap2(NULL, page_size_ * num_pages, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
    if (a == MAP_FAILED)
      return NULL;

#if defined(MEMORY_SANITIZER)
    __msan_unpoison(a, page_size_ * num_pages);
#endif

    struct PageHeader *header = reinterpret_cast<PageHeader*>(a);
    header->next = last_;
    header->num_pages = num_pages;
    last_ = header;

    return reinterpret_cast<uint8_t*>(a);
  }

  void FreeAll() {
    PageHeader *next;

    for (PageHeader *cur = last_; cur; cur = next) {
      next = cur->next;
      sys_munmap(cur, cur->num_pages * page_size_);
    }
  }

  struct PageHeader {
    PageHeader *next;  // pointer to the start of the next set of pages.
    size_t num_pages;  // the number of pages in this set.
  };

  const size_t page_size_;
  PageHeader *last_;
  uint8_t *current_page_;
  size_t page_offset_;
};

template <typename T>
struct PageStdAllocator : public std::allocator<T> {
  typedef typename std::allocator<T>::pointer pointer;
  typedef typename std::allocator<T>::size_type size_type;

  explicit PageStdAllocator(PageAllocator& allocator): allocator_(allocator) {}
  template <class Other> PageStdAllocator(const PageStdAllocator<Other>& other)
      : allocator_(other.allocator_) {}

  inline pointer allocate(size_type n, const void* = 0) {
    return static_cast<pointer>(allocator_.Alloc(sizeof(T) * n));
  }

  inline void deallocate(pointer, size_type) {
  }

  template <typename U> struct rebind {
    typedef PageStdAllocator<U> other;
  };

 private:
  template<typename Other> friend struct PageStdAllocator;

  PageAllocator& allocator_;
};

template<class T>
class wasteful_vector : public std::vector<T, PageStdAllocator<T> > {
 public:
  wasteful_vector(PageAllocator* allocator, unsigned size_hint = 16)
      : std::vector<T, PageStdAllocator<T> >(PageStdAllocator<T>(*allocator)) {
    std::vector<T, PageStdAllocator<T> >::reserve(size_hint);
  }
};

}  // namespace google_breakpad

inline void* operator new(size_t nbytes,
                          google_breakpad::PageAllocator& allocator) {
  return allocator.Alloc(nbytes);
}

#endif  // GOOGLE_BREAKPAD_COMMON_MEMORY_H_
