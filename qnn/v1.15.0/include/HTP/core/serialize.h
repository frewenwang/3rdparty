//==============================================================================
//
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#ifndef SERIALIZE_H
#define SERIALIZE_H 1

#include <cstdio>
#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <array>
#include <vector>
#include <map>
#include <typeinfo>
#include <typeindex>
#include <string_view>
#include "limits.h"
#include "dtype.h"
#include "log.h"
#include "allocator.h"
#include "op_extra_info.h"

#include "forward_classes.h"
#include "minihash.h"
#include "serialize_tensors.h"

namespace hnnx {

/**
 * @brief Common base to register error in Serialize/Deserialize
 * Calling register_error sets the error string (unless there is one already).
 * any_error() should be checked after each full use of serialize/deserialize
 * Note: register_error must be called with a string constant, or other
 * persistent string, since the contents of the string are not copied.
 *
 */
class DeSerError {
    char const *errstr; // null if no error
  public:
    DeSerError() : errstr(nullptr) {}
    void reset_error() { errstr = nullptr; }
    bool any_error() const { return errstr != nullptr; }
    void register_error(char const *estr)
    { // must be a persistent string!
        if (errstr == nullptr)
            errstr = estr;
    }
    char const *error_string() const { return errstr; }
};

// We allow 4 bits of extra flag storage when storing an Op type, using
// the upper four bits of the index.
enum {
    SerializeOpFlagMask = 0xf0000000,
    SerializeOpFlagShift = 28,
};

/**
 * @brief \ref Serializer and \ref Deserializer modules that provides
 * a mechanism to flatten (serialize) and reconstruct (deserialize)
 * primitive and user-defined data types. The initial objective
 * was to create an in-memory representation of the optimized
 * \ref Graph on x86 which can then be reconstructed and executed on
 * a qdsp target, essentially, a means to Graph caching.
 *
 * Implementation Notes:
 * Serialization registration: Create maps such that given the name of a type,
 * we can know what kind of object we should return.
 * We mainly need this for deserialization
 *
 * deserialization should create a new object and return it,
 * along with how many bytes we moved in the buffer or something
 *
 * Since we're starting from the byte stream and moving towards an object,
 * we don't have the object yet and so this is more factory-like.
 *
 * Deserialization should start with finding the type.  Then it should look up
 * the deserialization table for the type and call the specific deserializer
 * for that type.
 *
 * We will often want to return a unique ptr to the object...
 * I'm not sure how we will do that.
 * We have to create a table of deserialization routines,
 * I think that table must have all the same type
 * And if they have the same type, those function pointers must
 * all return the same type....
 * And if they return the same type, ???
 * I guess we could do a void pointer that we immediately throw into
 * a unique pointer for ownership...
 * Or maybe everything we return needs to be some Serializable object
 *
 * Or... since we are always serializing a Graph,
 * And since a Graph is just a set of Ops
 * And since Ops have Tensors
 *
 * Maybe we just need a specialized serialization/deserialization interface
 * instead of a generic one.
 *
 * OK, I think that will work.
 *
 * graph.serialize():
 *   foreach op: op.serialize()
 * op.serialize():
 *   Type
 *   ID
 *   foreach input tensor: input serialize as op ID+Output Idx
 *   foreach output tensor: tensor.serialize()
 * tensor.serialize():
 *   Type
 *   Serialize memory blocks
 *   Serialize Const Values
 *
 * Deserialization
 *   Op looks up type and calls op-type-specific deserializer
 *     - should deserialize correct # of output tensors
 *     - Can do 2-pass to hook in input tensors later (?)
 *     - returns uptr_Op
 *   Tensor deserializer looks up type and calls op-type-specific deserializer
 *     - deserialize tensor type, memory blocks, const values
 *     - returns uptr_Tensor
 *
 * Ops and Tensors need separate deserializer registrations
 *
 * a serializable object has some way to serialize itself.
 * - virtual serialize method?
 * - serialize<T> implementation?
 * - registered hook?
 * - inherits serialize<T> or something?
 *
 * For now, let's do virtual serialize method... might not last though
 */

/**
 * @class Serializer
 *
 * @brief Serializer class that converts data of type T to
 * a flat buffer representation.  The Serializer object
 * needs to know before-hand the total buffer size required.
 * Incremental calls to the serializer object continually
 * appends data to the allocated buffer.
 *
 */

class __attribute__((visibility("default"))) Serializer : public DeSerError {
  private:
    using type_index_map = minihash_noerase<std::type_info const *, unsigned>;
    type_index_map optype_idx_map;
    type_index_map tensortype_idx_map;

    // the state of the 'tensor connectivity' serialize engine.
    SerTensorConn tensorconn;

  protected:
    const char *bufstart; // start of current buffer
    const char *bufend; // first byte we can't write
    char *bufp; // next to write
    size_t bytes_flushed; // bytes previously flushed
    bool overflowed;
    bool measuring; // true if we are only measuring;
    // this is used to serialize shape, interface object without
    // duplicating shared objects. The key is the object address; the value is
    // is an index used to encode; these are assigned 1,2,3 ...
    minihash_noerase<void const *, unsigned> objmap;

  public:
    Allocator *allocator;
    /**
	 * @brief Construct a new Serializer object
	 *
	 * @param[in] allocator_in \ref fa::FancyAllocator to get access
	 * to memory used by tensors
	 * @param[in] p allocated buffer
	 * @param[in] n size of buffer
	 */
    Serializer(Allocator *allocator_in, char *p, size_t n)
        : bufstart(p), bufend(p + n), bufp(p), bytes_flushed(0),
          overflowed(false), measuring(false), allocator(allocator_in)
    {
    }
    /**
	 * @brief Destroy the Serializer object
	 *
	 */
    virtual ~Serializer(); // please keep this as first virtual method declared.
  protected:
    /**
	 * @brief Tracks data writes to buffer for over flow cases and sets
	 * buffer pointer to start while recording \ref bytes_flushed
	 *
	 * Note: all serializing is done via serialize_fwrite, or by simple_serialize (which
	 * calls flush_buffer when needed).
	 */
    virtual void flush_buffer(); // call for overflow on short operation

    /**
	 * @brief Call for an arbitary length data that needs to be written
	 * to the serializer buffer
	 *
	 * @param[in] p data that needs to be copied into serializer buffer
	 * @param[in] len length of the data to write
	 * @param[in] align True to align data write to a boundary of 4
	 *
	 * Note: all serializing is done via serialize_fwrite or by simple_serialize
	 */
    virtual void serialize_fwrite(const void *p, size_t len, bool align);

    /**
	 * @brief Call for an arbitary length data that needs to be written
	 * to the serializer buffer. This is used only when we simply want
	 * to count the length of the buffer. see \ref NullSerializer
	 *
	 * @param[in] p data that needs to be copied into serializer buffer
	 * @param[in] len length of the data to write
	 * @param[in] align True to align data write to a boundary of 4
	 *
	 */
    void serialize_dummy_fwrite(const void *p, size_t len, bool align);
    /**
	 * @brief Tracks data writes to buffer for over flow cases and sets
	 * buffer pointer to start while recording num bytes flushed.
	 * This is used only when we simply want
	 * to count the length of the buffer. see \ref NullSerializer
	 *
	 */
    void dummy_flush_buffer();

    /**
	 * @brief Get current position of buffer at which next data will be written
	 *
	 * @return size_t offset from buffer start
	 */
    size_t buffer_offset() const { return bufp - bufstart; }

    /**
	 * @brief Available buffer size remaining for serialization
	 *
	 * @return size_t remaining bytes size
	 */
    size_t buffer_remain() const { return bufend - bufp; }

    /**
	 * @brief serialize data of type T
	 *
	 * @tparam[in] value to serialize of type T
	 */
    // note: flush_buffer must always flush in multiples of 4, and copy any extra
    // to the front of the buffer.
    template <typename T> void simple_serialize(T val)
    {
        constexpr size_t W = (sizeof(T) < 4) ? 4 : sizeof(T);
        char *buf_next = bufp + W;
        if (buf_next > bufend) { // no room
            flush_buffer();
            buf_next = bufp + W;
        }
        if (sizeof(T) < 4) {
            // shorter int, ensure it fills the whole 32 bits
            *reinterpret_cast<uint32_t *>(bufp) = val;
        } else {
            *reinterpret_cast<T *>(bufp) = val;
        }
        bufp = buf_next;
    }
    bool serialize_shared_obj_func(void const *p);

    unsigned serialize_X_type(type_index_map &tidxmap,
                              std::type_info const &tid,
                              char const *name = nullptr, uint32_t flags = 0);

  public:
    /**
	 * @brief a 'serialize' operation can call is_measuring(); if it returns true,
	 * the serialization op is only being done to measure size; in this case,
	 * you can instead call measure_bytes() and supply the data length,rather
	 * than serializing.
	 *
	 * NOTE: the parameter to measure_bytes will always be rounded up to a multiple
	 * of 4.
	 *
	 */
    bool is_measuring() const { return measuring; }
    /**
	 * @brief Called only when just 'measuring' and is used
	 * to account for new size required for the serialization data
	 * using 'len' bytes (rounded up to a multiple of 4). The call
	 * also flushes the buffer. Note that the current position
	 * must also be a multiple of 4.
	 *
	 * @param[in] len length that needs to be accounted for
	 */
    virtual void measure_bytes(size_t len);

    /**
	 * @brief Gets the total bytes required for the serialization data
	 *
	 * @return size_t Length of bytes required for serialization
	 */
    size_t total_bytes() const { return bytes_flushed + buffer_offset(); }

    /**
	 * @brief Get status on the overflowed bit
	 *
	 * @return true if target serialization buffer has overflowed
	 * @return false if target serialization buffer has unfilled bytes
	 */
    bool is_overflowed() const { return overflowed; }

    /**
	 * @brief Final flush of the buffer which is called by client
	 * at the end of serialization process
	 *
	 * @return size_t total bytes serialized
	 */
    virtual size_t finalize() { return total_bytes(); }

    /**
	 * @brief serialize data of type which calls simple_serialize
	 *
	 * @param val data to serialize
	 *
	 * Note: the below are the only types supported for serialize_type<T>
	 */
    void serialize_uint64(uint64_t val); // inline below
    inline void serialize_float(float val) { simple_serialize<float>(val); }
    inline void serialize_uint32(uint32_t val)
    {
        simple_serialize<uint32_t>(val);
    }
    inline void serialize_int32(NN_INT32_T val)
    {
        simple_serialize<NN_INT32_T>(val);
    }
    inline void serialize_uint16(uint16_t val)
    {
        simple_serialize<uint16_t>(val);
    }
    inline void serialize_int16(int16_t val) { simple_serialize<int16_t>(val); }
    inline void serialize_uint8(uint8_t val) { simple_serialize<uint8_t>(val); }
    inline void serialize_int8(int8_t val) { simple_serialize<int8_t>(val); }

    // serialize string
    void serialize_str(const std::string_view &val)
    {
        serialize_buf_withlen(val.data(), val.size());
    }

    // Write a tagged header with the given payload
    //   - if 'hdrtag' fits in 16 bits, it is used as the header, and the
    //     record len is found as (datalen+7)/4
    //   - if 'hdrtag' is  (tag<<16)|reclen,  and reclen > (datalen+7)/4,
    //     then 'reclen' is used and in any case the payload
    //     is padded out to the full record len with zeros.
    void serialize_hdrtag(unsigned hdrtag, void const *data, unsigned datalen);

    // namesig as alias for uint64, in case we want to change namesig_t
    inline void serialize_namesig(const uint64_t val) { serialize_uint64(val); }

    // serialize a pointer as 64 bits
    inline void serialize_ptr(void const *p) { serialize_uint64(size_t(p)); }

    // For each op serialized:
    //  - serialize all the input refs  with serialize_tensor_ref and/or serialize_tensor_refs
    //  - register addresses of all out tensors with serialize_tensor_def.
    //  Note that Tensorsubclass->serialize() will call serialize_def,
    //  so it's only needed to call it separately on special tensor subclasses
    //  which don't get serialized themselves (typically these are embedded in special Op classes).
    // NOTE: currently, each tensor must be registered with serialize_tensor_def before
    // it can be passed to serialize_tensor_ref.
    //
    inline void serialize_tensor_def(Tensor const *p)
    {
        tensorconn.tensor_def(*this, p);
    }
    inline void serialize_tensor_ref(Tensor const *p)
    {
        tensorconn.tensor_ref(*this, p);
    }
    inline void serialize_tensor_refs(Tensor const *const *p, unsigned n)
    {
        tensorconn.tensor_refs(*this, p, n);
    }
    // can call serialize_tensor_refs on pointers to subclass of Tensor.
    template <typename T>
    inline void serialize_tensor_refs(T const *const *p, unsigned n)
    {
        static_assert(std::is_base_of<Tensor, T>::value);
        tensorconn.tensor_refs(*this, (Tensor const *const *)p, n);
    }

    void serialize_finish_tensors() { tensorconn.store_pending(*this); }

    void serialize_tensor_type(std::type_info const &ty, char const *name)
    {
        serialize_X_type(tensortype_idx_map, ty, name);
    }
    void serialize_op_type(std::type_info const &ty, char const *name,
                           uint32_t flags = 0)
    {
        serialize_X_type(optype_idx_map, ty, name, flags);
    }

    // write 32-bit header and then data
    void serialize_buf_withlen(const void *buf, size_t bufsize);

    template <typename T> void serialize_type(T val);

    // serialize a shared object (shape or interface:
    // - serializes the id
    // - if not seen before, returns true, and caller must actually
    //   serialize the object;
    // - otherwise returns false, caller does nothing else.
    //
    template <typename T> bool serialize_shared_obj(T const *p)
    {
        return serialize_shared_obj_func((void const *)p);
    }

    /**
	 * @brief convernience wrappers for serialize fuctions that
	 * take in different number of arguments of uint32_t type
	 *
	 * @param[in] x0 first uint32_t data to serialize
	 * @param[in] x1 second uint32_t data to serialize
	 */
    // convenience wrappers (to reduce inlined code size w/o much loss of speed)
    void serialize_uint32(uint32_t x0, uint32_t x1);
    void serialize_uint32(uint32_t x0, uint32_t x1, uint32_t x2);
    void serialize_uint32(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3);

    // to reduce code size in the templates, we can serialize arrays of
    // N sizet to uint32
    void serialize_uint32_arr_sizet(size_t const *p, size_t N);
    void serialize_uint32_arr(uint32_t const *p, size_t N);
    inline void serialize_unsigned_arr(unsigned const *p, size_t N)
    {
        static_assert(sizeof(uint32_t) == sizeof(unsigned));
        serialize_uint32_arr((uint32_t const *)p, N);
    }
    /**
	 * @brief serialize array of type std::array
	 *
	 * @tparam N num items in array
	 * @param arr actual array to serialize
	 */
    template <size_t N>
    inline void serialize_uint32_arr(std::array<size_t, N> const &arr)
    {
        serialize_uint32_arr_sizet(&arr[0], N);
    }
    /**
	 * @brief serialize vector
	 *
	 * @param[in] arr vector containing data of type uint32_t to serialize
	 */
    void serialize_uint32_arr(std::vector<size_t> const &arr)
    {
        serialize_uint32_arr_sizet(&arr[0], arr.size());
    }
};

// unaligned store of 64-bits (two 32-bit aligned store)
template <> inline void Serializer::simple_serialize<uint64_t>(uint64_t val)
{
    constexpr size_t W = sizeof(uint64_t);
    char *buf_next = bufp + W;
    if (buf_next > bufend) { // no room
        flush_buffer();
        buf_next = bufp + W;
    }
    uint32_t *p = reinterpret_cast<uint32_t *>(bufp);
    p[0] = (uint32_t)val;
    p[1] = (uint32_t)(val >> 32);
    bufp += W;
}

inline void Serializer::serialize_uint64(uint64_t val)
{
    simple_serialize<uint64_t>(val);
}

template <> inline void Serializer::serialize_type<uint64_t>(uint64_t val)
{
    serialize_uint64(val);
}
template <> inline void Serializer::serialize_type<float>(float val)
{
    serialize_float(val);
}
// sometimes uint32_t is unsigned long, sometimes it's unsigned
// sometimes unsigned long is uint64. Hopefully this should cover it all.
#if ULONG_MAX == UINT_MAX
template <>
inline void Serializer::serialize_type<unsigned long>(unsigned long val)
{
    serialize_uint32(val);
}
template <> inline void Serializer::serialize_type<long>(long val)
{
    serialize_int32(val);
}
#endif
template <> inline void Serializer::serialize_type<int>(int val)
{
    serialize_int32(val);
}
template <> inline void Serializer::serialize_type<unsigned>(unsigned val)
{
    serialize_uint32(val);
}
template <> inline void Serializer::serialize_type<uint16_t>(uint16_t val)
{
    serialize_uint16(val);
}
template <> inline void Serializer::serialize_type<int16_t>(int16_t val)
{
    serialize_int16(val);
}
template <> inline void Serializer::serialize_type<uint8_t>(uint8_t val)
{
    serialize_uint8(val);
}
template <> inline void Serializer::serialize_type<int8_t>(int8_t val)
{
    serialize_int8(val);
}

/**
 * @brief NullSerializer class is a 'fake' serializer, to measure the len of
 *  an operation. It has a 64-byte dummy buffer built in.
 *
 */
class __attribute__((visibility("default"))) NullSerializer
    : public Serializer {
    union {
        uint32_t for_align;
        char tbuf[64];
    } uu;

  public:
    /**
	 * @brief Construct a new Null Serializer object
	 *
	 * @param[in] allocator_in \ref fa::FancyAllocator to get access
	 * to memory used by tensors
	 */
    NullSerializer(Allocator *allocator_in)
        : Serializer(allocator_in, uu.tbuf, sizeof(uu.tbuf))
    {
        measuring = true;
    }
    virtual ~NullSerializer();

  private:
    /**
	 * @brief Tracks data writes to buffer for over flow cases and sets
	 * buffer pointer to start while recording num bytes flushed
	 *
	 * Note: all serializing is done via serialize_fwrite, or by simple_serialize (which
	 * calls flush_buffer when needed).
	 */
    virtual void flush_buffer() override
    {
        // call for overflow on short operation
        dummy_flush_buffer();
    }
    /**
	 * @brief Call for an arbitary length data that needs to be written
	 * to the serializer buffer. Dummys out the actual write but sets
	 * up the internal len members to later provide the actual length
	 * of bytes required for serialization
	 *
	 * @param[in] p data that needs to be copied into serializer buffer
	 * @param[in] len length of the data to write
	 * @param[in] align True to align data write to a boundary of 4
	 *
	 */
    virtual void serialize_fwrite(const void *p, size_t len,
                                  bool align) override
    {
        serialize_dummy_fwrite(p, len, align);
    }
};

// TODO: remove 'Graph &' param from all these, since dctx.graph() can be used instead.
using op_deserializer_fn = uptr_Op (*)(Deserializer &, Graph &graph_in);
using tensor_deserializer_fn = uptr_Tensor (*)(const Op *producer,
                                               Deserializer &, Graph &graph_in);

// here's a quick and dirty way to make these maps go faster: compare string_view starting with len;
// and if the len is the same, then compare the middle character, and if that's the same,
// use memcmp. This avoids getting slowed down by a lot of long common prefixes in the type names.
// and we don't care about the weird ordering it generates.
//
struct trick_stringview_lt {
    bool operator()(std::string_view const &a, std::string_view const &b) const
    {
        unsigned na = a.size();
        unsigned nb = b.size();
        if (na != nb)
            return na < nb;
        char const *pa = a.data();
        char const *pb = b.data();
        if (pa == pb || na == 0)
            return false; // pa==pb is a  common case.
        unsigned char_a = pa[na >> 1];
        unsigned char_b = pb[na >> 1];
        if (char_a != char_b)
            return char_a < char_b;
        return ::memcmp(pa, pb, na) < 0;
    }
};

using op_deserializer_map_t =
        std::map<std::string_view, std::pair<op_deserializer_fn, bool>,
                 trick_stringview_lt>;
using tensor_deserializer_map_t =
        std::map<std::string_view, tensor_deserializer_fn, trick_stringview_lt>;

/**
 * @brief Deserializer class to reverse the serialization
 * process and reconstruct the data for specific types
 *
 */
class __attribute__((visibility("default"))) Deserializer : public DeSerError {
  public:
    /**
	 * @brief Construct a new Deserializer object
	 *
	 * @param[in] p buffer that needs to be deserialized
	 * @param[in] n length of the buffer
	 * @param[in] g pointer Graph object to deserialize (usually null, since object
	 *              is being passed to the Graph::Graph ctor to deserialize; that ctor
	 *              must immediately call dctx.set_graph(*this) )
	 */
    Deserializer(char const *p, size_t n, Graph *g = nullptr);
    /**
	 * @brief Destroy the Deserializer object
	 *
	 */
    virtual ~Deserializer(); // please keep this as first virtual method declared.
    Allocator *allocator;
    typedef uint32_t object_identity_type;

    std::vector<op_deserializer_fn> op_deserialize_fn_list;
    std::vector<tensor_deserializer_fn> tensor_deserialize_fn_list;

    // when deserializing an op:
    //  - call deserialize_tensor_ref (or _refs) on all the input tensor pointers
    //  - pass all output tensor addresses to deserialize_tensor_def
    //  Sequence must match serialization; note that the deserialize-ctor of Tensor
    //  calls deserialize_tensor_def on itself; so there is no need to call it elsewhere,
    //   except for specialized types which are constructed otherwise during depickle (e.g.,
    //   types embedded in the Op).
    //
    // Some ops have multiple copies of some input tensor pointers; for these, it's possible
    // serialize just one reference, and the deserialize it using
    //     auto id = deserialize_object_identity()		// <- corresponds to serialize_tensor_ref
    //     need_tensor_fixup( id, &first_tensor_pointer);
    //      (other deserialize activity can happen here)
    //     need_tensor_fixup( id, &second_tensor_pointer);

    inline void deserialize_tensor_def(Tensor const *tensor_ptr)
    {
        tensorconn.tensor_def(*this, tensor_ptr);
    }
    inline void deserialize_tensor_ref(Tensor const *&where)
    {
        tensorconn.tensor_ref(*this, where);
    }
    inline void deserialize_tensor_refs(Tensor const **ptrs, unsigned n)
    {
        tensorconn.tensor_refs(*this, ptrs, n);
    }
    template <typename T> inline void deserialize_tensor_ref(T const *&where)
    {
        static_assert(std::is_base_of<Tensor, T>::value);
        tensorconn.tensor_ref(*this, *(Tensor const **)&where);
    }
    template <typename T>
    void deserialize_tensor_refs(T const **ptrs, unsigned n)
    {
        static_assert(std::is_base_of<Tensor, T>::value);
        tensorconn.tensor_refs(*this, (Tensor const **)ptrs, n);
    }
    inline object_identity_type deserialize_object_identity()
    {
        return tensorconn.read_identity(*this);
    }

    inline void need_tensor_fixup(object_identity_type oid,
                                  Tensor const **where)
    {
        tensorconn.need_fixup(oid, where);
    }
    inline void resolve_fixups() { tensorconn.read_pending(*this); }

    Graph &graph() const { return *graph_ptr; }
    void set_graph(Graph &g) { graph_ptr = &g; }

    op_deserializer_map_t const &get_op_deser_map() const
    {
        return *op_deserializer_map;
    }

  protected:
    // hoist pointers to these maps into Deserializer to avoid static lock overhead
    op_deserializer_map_t const *op_deserializer_map;
    tensor_deserializer_map_t const *tensor_deserializer_map;
    Graph *graph_ptr;
    std::vector<void const *> objindex; // index of pointers to shape, etc.
    // the state of the 'tensor connectivity' deserialize engine.
    DeserTensorConn tensorconn;

    char const *bufstart; // start of current buffer
    char const *bufend; // first byte we can't read
    char const *bufp; // next to read
    size_t bytes_filled; // bytes previously filled
    char name_buf[4096]; // used for string view

    uint32_t op_flags;
    OpExtraInfo op_extra_info;

    /**
	 * @brief throws an error since deserializer detected
	 * deserialization on insufficient bytes i.e. an underflow
	 *
	 */
    virtual void fill_buffer(); // called for underflow on short operation

    /**
	 * @brief Deserialize data of specified length and write into
	 * buffer provided by caller
	 *
	 * @param[out] p buffer to write to
	 * @param[in] len length of the \ref bufp to read from
	 * @param[in] align if true, skip input bytes to a boundary of 4
	 */
    virtual void deserialize_fread(void *p, size_t len, bool align);

    /**
	 * @brief Get current position of buffer from which next data will be read
	 *
	 * @return size_t offset from buffer start
	 */
    size_t buffer_offset() const { return bufp - bufstart; }
    /**
	 * @brief Available buffer size remaining for deserialization
	 *
	 * @return size_t remaining bytes size
	 */
    size_t buffer_remain() const { return bufend - bufp; }

    /**
	 * @brief deserialize buffer for type T
	 *
	 * @retval T returs the deserialized value of type T
	 *
	 * Note: This is the templated API called by deserialize_T() functions
	 *
	 * Note: buffer size must be a (large) multiple of 8, and fill_buffer will always fill in
	 * a multiple of 8; so if the next uint64 we want to read is split over the end of the buffer,
	 * the fill_buffer() will copy the first part down to the front of the buffer and fill after it,
	 * this avoids the need to read it in  two parts.
	 */
    template <typename T> T simple_deserialize()
    {
        constexpr size_t W = (sizeof(T) < 4) ? 4 : sizeof(T);
        char const *p_next = bufp + W;
        if (p_next > bufend) {
            errlog("deserialize error: data pos %p overbounds buffer end %p",
                   p_next, bufend);
            fill_buffer();
            p_next = bufp + W;
        }
        T val = *reinterpret_cast<T const *>(bufp);
        bufp = p_next;
        return val;
    }
    void const **deserialize_shared_obj_func();

  public:
    /**
	 * @brief deserialize data of type which calls simple_deserialize
	 *
	 * @param val data to deserialize
	 *
	 * Note: the below are the only types supported for deserialize_type<T>
	 */
    uint64_t deserialize_uint64(); // inline later
    inline float deserialize_float() { return simple_deserialize<float>(); }
    inline uint32_t deserialize_uint32()
    {
        return simple_deserialize<uint32_t>();
    }
    inline NN_INT32_T deserialize_int32()
    {
        return simple_deserialize<NN_INT32_T>();
    }
    inline int16_t deserialize_int16() { return simple_deserialize<int16_t>(); }
    inline uint16_t deserialize_uint16()
    {
        return simple_deserialize<uint16_t>();
    }
    inline int8_t deserialize_int8() { return simple_deserialize<int8_t>(); }
    inline uint8_t deserialize_uint8() { return simple_deserialize<uint8_t>(); }

    inline uint64_t deserialize_namesig() { return deserialize_uint64(); }

    tensor_deserializer_fn deserialize_tensor_identification();

    // deserialize string
    std::string_view deserialize_str();

    uint32_t get_op_flags() const { return op_flags; };
    void clear_op_flags() { op_flags = 0; };
    void set_op_flags(uint32_t f) { op_flags = f; };

    const OpExtraInfo &get_op_extra_info() const { return op_extra_info; };
    OpExtraInfo &get_op_extra_info() { return op_extra_info; };
    void clear_extra_info() { op_extra_info.clear(); };

    /**
	 * @brief deserialize buffer for specified size
	 *
	 * @param[in] alloc_size number of bytes to read from \ref bufp
	 * @param[out] ptr destination buffer for the read bytes
	 * @return size_t number of bytes actually read
	 */
    size_t deserialize_buf(size_t alloc_size, void *ptr);
    /**
	 * @brief similar to deserialize_buf but first deserialize a
	 * uint32_t size of bytes that should match the alloc_size
	 *
	 * @param[in] alloc_size number of bytes to read from \ref bufp
	 * @param[out] ptr destination buffer for the read bytes
	 * @return size_t number of bytes actually read
	 */
    size_t deserialize_buf_withlen(size_t alloc_size, void *ptr);
    // deserialize a pointer as 64 bits
    inline void *deserialize_ptr()
    {
        return (void *)size_t(deserialize_uint64());
    }

    template <typename T> T deserialize_type();

    template <typename RetT, size_t N, typename SerialT>
    std::array<RetT, N> deserialize_array();

    /**
	 * @brief convernience wrappers for deserialize fuctions that
	 * take in different number of arguments of uint32_t type
	 *
	 * @return std::tuple<uint32_t,uint32_t> (first, second) uint32_t data deserialized
	 */
    // convenience wrappers (to reduce inlined code size w/o much loss of speed)
    std::tuple<uint32_t, uint32_t> deserialize_uint32_x2();
    std::tuple<uint32_t, uint32_t, uint32_t> deserialize_uint32_x3();
    std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> deserialize_uint32_x4();

    void deserialize_uint32_arr(uint32_t *p, size_t N);

    // to reduce code size in the templates, we can deserialize arrays of
    // N uint32 to sizet
    void deserialize_uint32_arr_sizet(size_t *p, size_t N);

    /**
	 * @brief deserialize array containing uint32_t type date
	 *
	 * @tparam N size of the array
	 * @return std::array<size_t,N> array containing the deserialized values
	 */
    template <size_t N> std::array<size_t, N> deserialize_uint32_array_sizet()
    {
        std::array<size_t, N> res;
        deserialize_uint32_arr_sizet(&res[0], N);
        return res;
    }
    /**
	 * @brief deserialize vector containing uint32_t type date
	 *
	 * @param[in] size of vector; if N is less than 0, deserialize
	 * a uint32_t sized data first to get N
	 * @return std::vector<size_t> vector containing the deserialized values
	 */
    // this will read N elements; if N <0, read N first.
    std::vector<size_t> deserialize_uint32_vec_sizet(int N = -1);

    //
    // this is used for shared objects like Shape and Interface.
    // it deserializes the index, and then returns a pointer p to a location
    // in objindex. If that location *p is not null, the caller uses it directly;
    // if it is null, the caller must deserialize the object, put it in the crate,
    // and store its address at *p.
    template <typename T> T const **deserialize_shared_obj()
    {
        return (T const **)deserialize_shared_obj_func();
    }

    void *uncached_ptr;
    uint32_t uncached_len;
};
// unaligned read of 64-bits (two 32-bit aligned reads)
template <> inline uint64_t Deserializer::simple_deserialize<uint64_t>()
{
    constexpr size_t W = sizeof(uint64_t);
    char const *p_next = bufp + W;
    if (p_next > bufend) {
        errlog("deserialize 64-bits error: data pos %p overbounds buffer end %p",
               p_next, bufend);
        fill_buffer();
        p_next = bufp + W;
    }
    uint32_t const *p = reinterpret_cast<uint32_t const *>(bufp);
    bufp = p_next;
    return p[0] + ((uint64_t)p[1] << 32);
}
inline uint64_t Deserializer::deserialize_uint64()
{
    return simple_deserialize<uint64_t>();
}

template <> inline uint64_t Deserializer::deserialize_type<uint64_t>()
{
    return deserialize_uint64();
}
template <> inline float Deserializer::deserialize_type<float>()
{
    return deserialize_float();
}
// sometimes uint32_t is unsigned long, sometimes it's unsigned
// sometimes unsigned long is uint64. Hopefully this should cover it all.
#if ULONG_MAX == UINT_MAX
template <> inline unsigned long Deserializer::deserialize_type<unsigned long>()
{
    return deserialize_uint32();
}
template <> inline long Deserializer::deserialize_type<long>()
{
    return deserialize_int32();
}
#endif
template <> inline unsigned Deserializer::deserialize_type<unsigned>()
{
    return deserialize_uint32();
}
template <> inline int Deserializer::deserialize_type<int>()
{
    return deserialize_int32();
}
template <> inline int16_t Deserializer::deserialize_type<int16_t>()
{
    return deserialize_int16();
}
template <> inline uint16_t Deserializer::deserialize_type<uint16_t>()
{
    return deserialize_uint16();
}
template <> inline int8_t Deserializer::deserialize_type<int8_t>()
{
    return deserialize_int8();
}
template <> inline uint8_t Deserializer::deserialize_type<uint8_t>()
{
    return deserialize_uint8();
}

// assert( dctx.deserialize_uint32() == SOME_CONST );
// is not safe, since if you turn off asserts, it will no longer read the 4 bytes. This is to allow that to work
#define DESERIALIZE_ASSERT_UINT32(DCTX, VAL)                                   \
    do {                                                                       \
        uint32_t tmp [[gnu::unused]] = (DCTX).deserialize_uint32();            \
        assert(tmp == (VAL));                                                  \
    } while (0)

#pragma GCC visibility push(default)

/**
 * @brief register the deserialization function for each \ref Op
 * TypicalOp and VariadicOp derived classes are instantiated via
 * template and hence the need to create a map of deserialize functions
 * for each Op when they are generated at library initialization
 *
 * @param[in] tinf Op type_info that is used to key the map
 * @param[in] fn Deserialize function
 */
void deserialize_op_register(std::type_info const &tinf, const char *type_tag,
                             op_deserializer_fn fn, bool is_external = false);
/**
 * @brief register the deserialization function for each \ref Tensor
 * Since \ref Tensor derived classes are instantiated via templates, there
 * is a need to create a map of deserialize function for each Tensor at runtime
 *
 * @param[in] type_tag Tensor type tag that is used to key the map
 * @param[in] fn Deserialize function
 */
void deserialize_tensor_register(std::type_info const &tinf,
                                 const char *type_tag,
                                 tensor_deserializer_fn fn);

#pragma GCC visibility pop

// this is fully defined in serialize_register.h
template <typename T> struct deserialize_tensor_using_constructor;

// this is fully defined in serialize_register.h
template <typename T> struct deserialize_op_using_constructor;

//////////////////////
// Forward decls of things defined in template_help.h
//
// contains_type< tuple<a,b,c>, x >::value: true if x is in a,b,c ...
// no 'remove ref' etc is done.
template <typename TUPLET, typename T> struct contains_type;
template <typename TUPLET, typename T> struct not_contains_type;
template <template <typename> typename Pred, typename...> struct TupFilter;

#pragma GCC visibility push(default)
/**
 * Deserialize some index into some table.
 * If you need to throw away an op header, this does that.
 */

size_t deserialize_op_idx(Deserializer &dctx, Graph &graph_in);

/**
 * @brief deserialize an \ref Op. The implementation makes uses of the map
 * created during \ref deserialize_op_register to construct the Op.
 *
 * @param[in] dctx \ref Deserializer context that has the buffer to read from
 * @param[in] graph_in \ref Graph context where this Op lives. See constructors
 * of Op with \ref Deserializer input
 * @return uptr_Op unique_ptr of \ref Op type
 */
uptr_Op deserialize_op(Deserializer &dctx, Graph &graph_in, bool *is_vec_op);
/**
 * @brief deserialize a \ref Tensor. The implementation makes use of the map
 * created during \ref deserialize_tensor_register to construct the Tensor.
 *
 * @param[in] producer \ref Op that will produce this tensor
 * @param[in] dctx \ref Deserializer context that has the buffer to read from
 * @param[in] graph_in \ref Graph context where this Tensor lives.
 * @return uptr_Tensor unique_ptr of \ref Tensor type
 */
uptr_Tensor deserialize_tensor(const Op *producer, Deserializer &dctx,
                               Graph &graph_in);

#pragma GCC visibility pop

/*
 * EJP: FIXME: should probably change from FILE * to memory buffer
 * Especially for the reading side.
 * But this should work for now.
 */
#if 0

template <typename T> T serialize_archive_read(FILE *);

template <typename T> void serialize_archive_write(FILE *, T const &val);
template <> std::string serialize_archive_read<std::string>(FILE *);
template <> int32_t serialize_archive_read<int32_t>(FILE *);
template <> uint32_t serialize_archive_read<uint32_t>(FILE *f)
{
    return (uint32_t)serialize_archive_read<int32_t>(f);
}
template <> int64_t serialize_archive_read<int64_t>(FILE *);
template <> uint64_t serialize_archive_read<uint64_t>(FILE *f)
{
    return (uint64_t)serialize_archive_read<int64_t>(f);
}
template <> uptr_Op serialize_archive_read<uptr_Op>(FILE *);
template <>
std::unique_ptr<Tensor> serialize_archive_read<std::unique_ptr<Tensor>>(FILE *);

template <>
void serialize_archive_write<std::string>(FILE *, std::string const &val);
template <> void serialize_archive_write<int32_t>(FILE *, int32_t const &val);
template <> void serialize_archive_write<int64_t>(FILE *, int64_t const &val);
template <> void serialize_archive_write<Op>(FILE *f, Op const &val)
{
    val.serialize(f);
}
template <> void serialize_archive_write<Tensor>(FILE *f, Tensor const &val)
{
    val.serialize(f);
}
#endif

} // namespace hnnx

#endif
