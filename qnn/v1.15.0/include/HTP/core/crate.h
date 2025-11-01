//==============================================================================
//
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

/*
 * crate.h
 *
 *  Created on: Aug 1, 2019
 *      Author: smithg
 */

#ifndef CRATE_H_
#define CRATE_H_
#include <cstddef>
#include <cstdint>
#include <utility>
#include <list>
#include <memory>
#include <vector>
#include <string.h>

#pragma GCC visibility push(default)

class Graph;
class Tensor;

/// @brief A 'Crate' allows construction of some number of different data types,
/// contiguously packed into a few large memory blocks.
///
/// Example:
///
///     Crate crt;
///     Thing  tp* = crt.emplace<Thing>( ... ctor parms for Thing ... )
///     AnotherThing tp2* = crt.emplace<AnotherThing>( ... ctor parms for AnotherThing ... )
///
/// When the crate is destroyed, all of the contained objects are destroyed in the reverse
/// order. You can also 'remove' a single entry using
///
///     crt.erase( tp );		// erase the thing
///
/// However, this is likely not going to free any memory; it will just call the dtor of the
/// object (and make sure it doesn't get called later, when the Crate is cleared or destroyed).
///
/// You can also emplace variable-sized arrays of trivially-destructable objects.
///
/// alloc_array does not initialize:
///
///     float * farr = crt.alloc_array<float>(n);
///
/// alloc_array_zero does zero-initializing:
///
///     int * farr = crt.alloc_array_zero<int>(n);
///
/// If an allocation needs space larger than CHUNKBYTES, it will get its own chunk.
///
// Each record containing an object has a non-null 'dtor' field; if the object is trivially destructible,
// this will be  (dtor_funcp)1, and the object is not on the linked-list.
// When an object is destroyed 'early' (via erase), the dtor is called, and the dtor field set to NULL.
// The object will not be removed from the dtor list (unless it's really easy, e.g. if it's the most recently
// added), so dtor==NULL  is used to ensure it doesn't get dtor'd later.
//
// note:
//  A constructor may emplace additional records in the crate recursively. Likewise,
//  it's OK if the dtors call erase() on other objects. If this happens during a 'clear',
//  the erase calls are ignored since the other objects are going to get dtor'd anyhow (if they have not
//  been already).
// Important: if object A's constructor places B into the crate, then B will very likely get destroyed
//  first when the crate is cleared. Thus, A's destructor can't look at B (it can erase B, which is ignored
//  as described above).

namespace hnnx {

class Crate {
    static constexpr size_t CHUNKBYTES = (1 << 16);
    static_assert(CHUNKBYTES % 8 == 0 && CHUNKBYTES >= 128);
    typedef void (*dtor_funcp)(void *);
    static dtor_funcp DTOR_TRIVIAL() { return (dtor_funcp)1; }
    static dtor_funcp DTOR_IN_PROCESS() { return (dtor_funcp)2; }

    //! A record in the index of a chunk
    struct index_rec {
        unsigned loc; ///< offset in bytes to the object
        dtor_funcp
                dtor; ///< pointer to dtor function (null if empty record; (DTOR_TRIVIAL if the object is trivial dtor)
    };
    //! A chunk record in the crate.
    ///
    /// Each chunk is created as an array of uint64_t, via make_unique<uint64_t[]>
    /// The memory in a chunk has a chunkhdr, which is followed by:
    ///
    ///    [Objects][Objects][Objects]--> free space   <--[Index records]
    ///
    /// 'alloc_count' is the next offset available to be allocated.
    /// index records are entered in reverse order from the end. So, the last nrec*sizeof(index_rec)
    /// bytes of the area, are the index.
    ///
    typedef std::unique_ptr<uint64_t[]> uptr_chunk_t;
    struct chunkhdr;
    static chunkhdr *hdr_of(uptr_chunk_t &p)
    {
        return reinterpret_cast<chunkhdr *>(p.get());
    }
    /// The chunkhdr is the first portion of the chunk, and is immediately followed
    /// by data_len bytes, which is a multiple of 8.
    struct alignas(8) chunkhdr {
        unsigned
                data_len; ///< length of the data area following header, bytes (>=CHUNKBYTES).
        unsigned nrec; ///< records in use (including deleted ones)
        unsigned alloc_count; ///< offset of first byte in 'free space'
        // init to a given length (header not included)
        void init(unsigned length)
        {
            data_len = length;
            nrec = 0;
            alloc_count = 0;
        }
        // reset (preserve data_len)
        void init()
        {
            nrec = 0;
            alloc_count = 0;
        }
        // pointer to 'offs ' within data area
        inline uint8_t *get_ptr(unsigned offs)
        {
            return (uint8_t *)(this + 1) + offs;
        }
        // pointer to end of  the allocation
        inline uint8_t *get_end_ptr()
        {
            return (uint8_t *)(this + 1) + data_len;
        }
        // amount of space remaining
        inline size_t space_avail() const
        {
            return data_len - alloc_count - nrec * sizeof(index_rec);
        }
        // get pointer to an index record.
        // record 0 is the last (oldest) one.
        index_rec *index_p(int idx)
        {
            return (index_rec *)get_end_ptr() - (idx + 1);
        }
        // this gives a pointer to the innermost (most recent) one, if nrec >= 1
        index_rec *index_pn() { return (index_rec *)get_end_ptr() - nrec; }
        bool contains_addr(void const *p)
        {
            uint8_t const *px = (uint8_t const *)p;
            return px >= get_ptr(0) && px < get_end_ptr();
        }
        static uptr_chunk_t allocate(unsigned len);
    };
    std::vector<uptr_chunk_t> m_chunks; /// < chunks with data
    std::vector<uptr_chunk_t> m_free; /// < chunks without
    typedef std::vector<uptr_chunk_t>::iterator chunk_iter;

    bool m_clearing; ///< set while clearing.
    size_t m_allrecords; ///< includes removed and 'padding' records
    size_t m_records; ///< only actual, non-erased records.

    //! Returned from add_record_slot (which is used to create a new record)
    struct recposn {
        chunkhdr *chunkp; ///< the chunk in which it was found
        void *objp; ///< pointer to the object
        int idx; ///< index within the chunk
    };
    //! Returned from find_exact_record (which used in erase())
    struct foundrec {
        chunk_iter iter; ///< iterator pointing to the chunk
        int idx; ///< index within the chunk
    };
    foundrec find_exact_record(void *) noexcept;
    recposn add_record_slot(size_t bytes, size_t align);
    void recover_ctor_throw(recposn const &) noexcept;
    void install_dtor(recposn const &, dtor_funcp dtor_func);
    void remove_empty_chunks() noexcept;
    void move_to_free(chunk_iter chunk_to_free);

  public:
    class ChunkHandle {
        friend class Crate;
        chunkhdr *chunkp;

      protected:
        ChunkHandle(chunkhdr *cp) : chunkp(cp){};

      public:
        ChunkHandle(ChunkHandle const &) = default;
        ChunkHandle &operator=(ChunkHandle const &) = default;
        std::pair<void *, size_t> get_memory_extent() const
        {
            size_t len =
                    chunkp->get_ptr(chunkp->alloc_count) - (uint8_t *)chunkp;
            return {chunkp, len};
        }
    };

    Crate(); ///< Construct a new Crate
    Crate(Crate const &) = delete;

    // get the preload handle for the most recent chunk
    ChunkHandle last_chunk_handle() const
    {
        return ChunkHandle(
                m_chunks.empty()
                        ? nullptr
                        : hdr_of(const_cast<Crate &>(*this).m_chunks.back()));
    }

    ~Crate();
    //! The number of objects in the crate.
    size_t size() const { return m_records; }
    //! The number of chunks in use
    size_t chunk_count() const { return m_chunks.size(); }
    //! Delete all objects. Does not necessarily free all storage to the
    /// system; but all retained storage is availabe for re-use in the crate.
    void clear();

    //! Construct an object of type T into the crate, using the
    /// parameters of any constructor of T. It is acceptable for the
    /// constructor to call the emplace method to add other objects to
    /// the crate.
    template <typename T, typename... Args>
    T *__attribute__((visibility("hidden"))) emplace(Args &&...args)
    {
        recposn pos = add_record_slot(sizeof(T), alignof(T));
        // construct the object
        if constexpr (std::is_nothrow_constructible<T, Args...>::value) {
            new (pos.objp) T(std::forward<Args>(args)...);
        } else {
            try {
                new (pos.objp) T(std::forward<Args>(args)...);
            } catch (const std::exception &e) {
                recover_ctor_throw(pos);
                throw;
            }
        }
        // register destructor
        if constexpr (!std::is_trivially_destructible<T>::value) {
            // Obtain a callable '~T()' function.
            // this typically generates a jump, or a small inline; lambda can
            // be cast to a function pointer since it has no state.
            auto dtor_func = [](void *obj) { static_cast<T *>(obj)->~T(); };
            install_dtor(pos, (dtor_funcp)dtor_func);
        } else {
            ++m_records; // note, install_dtor does this too.
        }
        return static_cast<T *>(pos.objp);
    }
    //! Allocate 'n' of type T in the crate.
    /// Will initially be garbage; T must be trivially destructable.
    template <typename T> T *alloc_array(size_t n)
    {
        static_assert(std::is_trivially_destructible<T>::value);
        if (n == 0)
            return nullptr;
        recposn pos = add_record_slot(sizeof(T) * n, alignof(T));
        ++m_records;
        return static_cast<T *>(pos.objp);
    }
    //! Allocate 'n' of type T in the crate.
    /// Will be zero-filled; T must be trivially destructable.
    template <typename T> T *alloc_array_zero(size_t n)
    {
        T *res = alloc_array<T>(n);
        if (n != 0)
            ::memset(res, 0, sizeof(T) * n);
        return res;
    }
    //! Allocate 'n' of type T in the crate.
    /// Will be "value constructed"; in case of things like int and pointer,
    /// this means they will be zeroed.
    ///
    /// T must be trivially destructable.
    template <typename T> T *alloc_array_value(size_t n)
    {
        T *res = alloc_array<T>(n);
        if (n != 0)
            std::uninitialized_value_construct_n(res, n);
        return res;
    }

    //! Remove a specific object from the crate.
    /// The object's destructor will be invoked, but the memory
    /// may or may not be recovered at the time of this call.
    /// It is acceptable for a object's destructor to erase other
    /// entries in the crate.
    void erase(void *p);
};

/*
 * EJP: This seems silly, but I don't know how to get visibility into Graph into a templated Tensor because of include hell.
 */

Crate *graph_crate(Graph &graph_in);

//
// replacement for vector, for use in ops;

// This is currently a simulacrum, to find out what methods we
// need to implement to support the existing code, and so that the
// constructors can be modified to supply the Graph &.
//
// limited options for constructor:
//   (1) copy from vector<T> - need Graph *;
//   (2) create with a given size - need Graph *;
//   (3) create empty, and then fill in later using init( Graph* , std::vector)
//       or init( Graph *, T *ptr, size );
//       or init( Graph *, size )
// With option 3, it assumed that the 'init' is done during the constructor of
// a host object - this is needed during deserialize, for instance.
//
// In all cases, where Graph* supplied, it can be null, and then array will not
// be constructed in a crate.
//

template <typename T> class cratevec {
    T *m_ptr;
    unsigned m_len;
    bool m_need_del;
    using vec_t = std::vector<T>;

  public:
    using iterator = T *;
    using const_iterator = T const *;
    using value_type = T;
    using size_type = size_t;
    using different_type = ptrdiff_t;
    using reference = T &;
    using const_reference = T const &;

    cratevec() : m_ptr(nullptr), m_len(0), m_need_del(false) {}
    cratevec(Graph *, vec_t const &v);
    cratevec(Graph *, vec_t &&v);
    cratevec(Graph *, size_t n);
    cratevec(cratevec const &) = delete;
    cratevec(cratevec &&) = delete;
    ~cratevec();

    cratevec &operator=(cratevec const &) = delete;
    cratevec &operator=(cratevec &&) = delete;

    void init(Graph *g, T const *data, size_t n);
    void init(Graph *g, vec_t const &v) { init(g, v.data(), v.size()); }
    void init(Graph *g, size_t n);

    iterator begin() noexcept { return m_ptr; }
    iterator end() noexcept { return m_ptr + m_len; }
    const_iterator begin() const noexcept { return m_ptr; }
    const_iterator end() const noexcept { return m_ptr + m_len; }
    const_iterator cbegin() const noexcept { return m_ptr; }
    const_iterator cend() const noexcept { return m_ptr + m_len; }
    size_type size() const noexcept { return m_len; }
    T *data() noexcept { return m_ptr; }
    T const *data() const noexcept { return m_ptr; }
    bool empty() const noexcept { return m_len == 0; }
    reference operator[](size_type idx) { return m_ptr[idx]; }
    const_reference operator[](size_type idx) const { return m_ptr[idx]; }
    reference at(size_type idx)
    {
        if (idx >= m_len)
            throw std::range_error("cratevec");
        return m_ptr[idx];
    }
    const_reference at(size_type idx) const
    {
        return const_cast<cratevec &>(*this).at(idx);
    }
    reference front() { return m_ptr[0]; }
    const_reference front() const { return m_ptr[0]; }
    reference back() { return m_ptr[m_len - 1]; }
    const_reference back() const { return m_ptr[m_len - 1]; }
};
extern template class cratevec<Tensor const *>;
extern template class cratevec<void *>;

} // namespace hnnx

#pragma GCC visibility pop

#endif /* CRATE_H_ */
