/* SPDX-License-Identifier: (BSD-2-Clause AND libpng-2.0) */
#define SPNG__BUILD

#include "spng.h"

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define ZLIB_CONST

#ifdef __FRAMAC__
    #define SPNG_DISABLE_OPT
    #include "tests/framac_stubs.h"
#else
    #ifdef SPNG_USE_MINIZ
        #include "miniz.h"
    #else
        #include <zlib.h>
    #endif
#endif

#ifdef SPNG_MULTITHREADING
    #include <pthread.h>
#endif

#define SPNG_READ_SIZE 8192

#define SPNG_TARGET_CLONES(x)

#ifndef SPNG_DISABLE_OPT

    #if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
        #define SPNG_X86

        #if defined(__x86_64__) || defined(_M_X64)
            #define SPNG_X86_64
        #endif

    #elif defined(__aarch64__) || defined(_M_ARM64) || defined(__ARM_NEON)
        /* #define SPNG_ARM */ /* buffer overflow for rgb8 images */
        #define SPNG_DISABLE_OPT
    #else
        #pragma message "disabling SIMD optimizations for unknown target"
        #define SPNG_DISABLE_OPT
    #endif

    #if defined(SPNG_X86_64) && defined(SPNG_ENABLE_TARGET_CLONES)
        #undef SPNG_TARGET_CLONES
        #define SPNG_TARGET_CLONES(x) __attribute__((target_clones(x)))
    #else
        #define SPNG_TARGET_CLONES(x)
    #endif

    #ifndef SPNG_DISABLE_OPT
        static void defilter_sub3(size_t rowbytes, unsigned char *row);
        static void defilter_sub4(size_t rowbytes, unsigned char *row);
        static void defilter_avg3(size_t rowbytes, unsigned char *row, const unsigned char *prev);
        static void defilter_avg4(size_t rowbytes, unsigned char *row, const unsigned char *prev);
        static void defilter_paeth3(size_t rowbytes, unsigned char *row, const unsigned char *prev);
        static void defilter_paeth4(size_t rowbytes, unsigned char *row, const unsigned char *prev);
    #endif
#endif

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4244)
#endif

#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__BIG_ENDIAN__)
    #define SPNG_BIG_ENDIAN
#else
    #define SPNG_LITTLE_ENDIAN
#endif

enum spng_state
{
    SPNG_STATE_INVALID = 0,
    SPNG_STATE_INIT = 1, /* No PNG buffer/stream is set */
    SPNG_STATE_INPUT, /* Input PNG was set */
    SPNG_STATE_IHDR, /* IHDR was read */
    SPNG_STATE_FIRST_IDAT,  /* Reached first IDAT */
    SPNG_STATE_DECODE_INIT, /* Decoder is ready for progressive reads */
    SPNG_STATE_EOI, /* Reached the last scanline/row */
    SPNG_STATE_LAST_IDAT, /* Reached last IDAT, set at end of decode_image() */
    SPNG_STATE_AFTER_IDAT, /*  */
    SPNG_STATE_IEND, /* Reached IEND */
};

#define SPNG_STR(x) _SPNG_STR(x)
#define _SPNG_STR(x) #x

#define SPNG_VERSION_STRING SPNG_STR(SPNG_VERSION_MAJOR) "." \
                            SPNG_STR(SPNG_VERSION_MINOR) "." \
                            SPNG_STR(SPNG_VERSION_PATCH)

#define SPNG_GET_CHUNK_BOILERPLATE(chunk) \
    if(ctx == NULL) return 1; \
    int ret = read_chunks(ctx, 0); \
    if(ret) return ret; \
    if(!ctx->stored.chunk) return SPNG_ECHUNKAVAIL; \
    if(chunk == NULL) return 1

#define SPNG_SET_CHUNK_BOILERPLATE(chunk) \
    if(ctx == NULL || chunk == NULL) return 1; \
    if(ctx->data == NULL) ctx->encode_only = 1; \
    int ret = read_chunks(ctx, 0); \
    if(ret) return ret

struct spng_text2
{
    int type;
    char *keyword;
    char *text;

    size_t text_length;

    uint8_t compression_flag; /* iTXt only */
    char *language_tag; /* iTXt only */
    char *translated_keyword; /* iTXt only */

    size_t cache_usage;
};


/* Packed sample iterator */
struct spng__iter
{
    const uint8_t mask;
    unsigned shift_amount;
    const unsigned initial_shift, bit_depth;
    const unsigned char *samples;
};

static const uint32_t png_u32max = 2147483647;

static const uint32_t adam7_x_start[7] = { 0, 4, 0, 2, 0, 1, 0 };
static const uint32_t adam7_y_start[7] = { 0, 0, 4, 0, 2, 0, 1 };
static const uint32_t adam7_x_delta[7] = { 8, 8, 4, 4, 2, 2, 1 };
static const uint32_t adam7_y_delta[7] = { 8, 8, 8, 4, 4, 2, 2 };

static const uint8_t png_signature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

static const uint8_t type_ihdr[4] = { 73, 72, 68, 82 };
static const uint8_t type_plte[4] = { 80, 76, 84, 69 };
static const uint8_t type_idat[4] = { 73, 68, 65, 84 };
static const uint8_t type_iend[4] = { 73, 69, 78, 68 };

static const uint8_t type_trns[4] = { 116, 82, 78, 83 };
static const uint8_t type_chrm[4] = { 99,  72, 82, 77 };
static const uint8_t type_gama[4] = { 103, 65, 77, 65 };
static const uint8_t type_iccp[4] = { 105, 67, 67, 80 };
static const uint8_t type_sbit[4] = { 115, 66, 73, 84 };
static const uint8_t type_srgb[4] = { 115, 82, 71, 66 };
static const uint8_t type_text[4] = { 116, 69, 88, 116 };
static const uint8_t type_ztxt[4] = { 122, 84, 88, 116 };
static const uint8_t type_itxt[4] = { 105, 84, 88, 116 };
static const uint8_t type_bkgd[4] = { 98,  75, 71, 68 };
static const uint8_t type_hist[4] = { 104, 73, 83, 84 };
static const uint8_t type_phys[4] = { 112, 72, 89, 115 };
static const uint8_t type_splt[4] = { 115, 80, 76, 84 };
static const uint8_t type_time[4] = { 116, 73, 77, 69 };

static const uint8_t type_offs[4] = { 111, 70, 70, 115 };
static const uint8_t type_exif[4] = { 101, 88, 73, 102 };

static inline void *spng__malloc(spng_ctx *ctx,  size_t size)
{
    return ctx->alloc.malloc_fn(size);
}

static inline void *spng__calloc(spng_ctx *ctx, size_t nmemb, size_t size)
{
    return ctx->alloc.calloc_fn(nmemb, size);
}

static inline void *spng__realloc(spng_ctx *ctx, void *ptr, size_t size)
{
    return ctx->alloc.realloc_fn(ptr, size);
}

static inline void spng__free(spng_ctx *ctx, void *ptr)
{
    ctx->alloc.free_fn(ptr);
}

#if defined(SPNG_USE_MINIZ)
static void *spng__zalloc(void *opaque, size_t items, size_t size)
#else
static void *spng__zalloc(void *opaque, uInt items, uInt size)
#endif
{
    spng_ctx *ctx = opaque;

    if(size > SIZE_MAX / items) return NULL;

    size_t len = (size_t)items * size;

    return spng__malloc(ctx, len);
}

static void spng__zfree(void *opqaue, void *ptr)
{
    spng_ctx *ctx = opqaue;
    spng__free(ctx, ptr);
}

static inline uint16_t read_u16(const void *_data)
{
    const unsigned char *data = _data;

    return (data[0] & 0xFFU) << 8 | (data[1] & 0xFFU);
}

static inline uint32_t read_u32(const void *_data)
{
    const unsigned char *data = _data;

    return (data[0] & 0xFFUL) << 24 | (data[1] & 0xFFUL) << 16 |
           (data[2] & 0xFFUL) << 8  | (data[3] & 0xFFUL);
}

static inline int32_t read_s32(const void *_data)
{
    const unsigned char *data = _data;

    int32_t ret;
    uint32_t val = (data[0] & 0xFFUL) << 24 | (data[1] & 0xFFUL) << 16 |
                   (data[2] & 0xFFUL) << 8  | (data[3] & 0xFFUL);

    memcpy(&ret, &val, 4);

    return ret;
}

/* Returns an iterator for 1,2,4,8-bit samples */
static struct spng__iter spng__iter_init(unsigned bit_depth, const unsigned char *samples)
{
    struct spng__iter iter =
    {
        .mask = (uint32_t)(1 << bit_depth) - 1,
        .shift_amount = 8 - bit_depth,
        .initial_shift = 8 - bit_depth,
        .bit_depth = bit_depth,
        .samples = samples
    };

    return iter;
}

/* Returns the current sample unpacked, iterates to the next one */
static inline uint8_t get_sample(struct spng__iter *iter)
{
    uint8_t x = (iter->samples[0] >> iter->shift_amount) & iter->mask;

    iter->shift_amount -= iter->bit_depth;

    if(iter->shift_amount > 7)
    {
        iter->shift_amount = iter->initial_shift;
        iter->samples++;
    }

    return x;
}

static void u16_row_to_host(void *row, size_t size)
{
    uint16_t *px = row;
    size_t i, n = size / 2;
    for(i=0; i < n; i++)
    {
        px[i] = read_u16(&px[i]);
    }
}

static void rgb8_row_to_rgba8(const unsigned char *row, unsigned char *out, uint32_t n)
{
    uint32_t i;
    for(i=0; i < n; i++)
    {
        memcpy(out + i * 4, row + i * 3, 3);
        out[i*4+3] = 255;
    }
}

/* Calculate scanline width in bits, round up to the nearest byte */
static int calculate_scanline_width(struct spng_ctx *ctx, uint32_t width, size_t *scanline_width)
{
    if(!width) return SPNG_EINTERNAL;

    size_t res = ctx->channels * ctx->ihdr.bit_depth;

    if(res > SIZE_MAX / width) return SPNG_EOVERFLOW;
    res = res * width;

    res += 15; /* Filter byte + 7 for rounding */

    if(res < 15) return SPNG_EOVERFLOW;

    res /= 8;

    if(res > UINT32_MAX) return SPNG_EOVERFLOW;

    *scanline_width = res;

    return 0;
}

static int calculate_subimages(struct spng_ctx *ctx)
{
    if(ctx == NULL) return SPNG_EINTERNAL;

    struct spng_ihdr *ihdr = &ctx->ihdr;
    struct spng_subimage *sub = ctx->subimage;

    if(ihdr->interlace_method == 1)
    {
        sub[0].width = (ihdr->width + 7) >> 3;
        sub[0].height = (ihdr->height + 7) >> 3;
        sub[1].width = (ihdr->width + 3) >> 3;
        sub[1].height = (ihdr->height + 7) >> 3;
        sub[2].width = (ihdr->width + 3) >> 2;
        sub[2].height = (ihdr->height + 3) >> 3;
        sub[3].width = (ihdr->width + 1) >> 2;
        sub[3].height = (ihdr->height + 3) >> 2;
        sub[4].width = (ihdr->width + 1) >> 1;
        sub[4].height = (ihdr->height + 1) >> 2;
        sub[5].width = ihdr->width >> 1;
        sub[5].height = (ihdr->height + 1) >> 1;
        sub[6].width = ihdr->width;
        sub[6].height = ihdr->height >> 1;
    }
    else
    {
        sub[0].width = ihdr->width;
        sub[0].height = ihdr->height;
    }

    int i;
    for(i=0; i < 7; i++)
    {
        if(sub[i].width == 0 || sub[i].height == 0) continue;

        int ret = calculate_scanline_width(ctx, sub[i].width, &sub[i].scanline_width);
        if(ret) return ret;

        if(sub[ctx->widest_pass].scanline_width < sub[i].scanline_width) ctx->widest_pass = i;

        ctx->last_pass = i;
    }

    return 0;
}


static int increase_cache_usage(spng_ctx *ctx, size_t bytes)
{
    if(ctx == NULL || !bytes) return SPNG_EINTERNAL;

    size_t new_usage = ctx->chunk_cache_usage + bytes;

    if(new_usage < ctx->chunk_cache_usage) return SPNG_EOVERFLOW;

    if(new_usage > ctx->chunk_cache_limit) return SPNG_ECHUNK_LIMITS;

    ctx->chunk_cache_usage = new_usage;

    return 0;
}

static int decrease_cache_usage(spng_ctx *ctx, size_t usage)
{
    if(ctx == NULL || !usage) return SPNG_EINTERNAL;
    if(usage > ctx->chunk_cache_usage) return SPNG_EINTERNAL;

    ctx->chunk_cache_usage -= usage;

    return 0;
}

static int is_critical_chunk(struct spng_chunk *chunk)
{
    if(chunk == NULL) return 0;
    if((chunk->type[0] & (1 << 5)) == 0) return 1;

    return 0;
}

static inline int read_data(spng_ctx *ctx, size_t bytes)
{
    if(ctx == NULL) return SPNG_EINTERNAL;
    if(!bytes) return 0;

    if(ctx->streaming && (bytes > SPNG_READ_SIZE)) return SPNG_EINTERNAL;

    int ret = ctx->read_fn(ctx, ctx->read_user_ptr, ctx->stream_buf, bytes);

    if(ret)
    {
        if(ret > 0 || ret < SPNG_IO_ERROR) ret = SPNG_IO_ERROR;

        return ret;
    }

    ctx->bytes_read += bytes;
    if(ctx->bytes_read < bytes) return SPNG_EOVERFLOW;

    return 0;
}

/* Read and check the current chunk's crc,
   returns -SPNG_CRC_DISCARD if the chunk should be discarded */
static inline int read_and_check_crc(spng_ctx *ctx)
{
    if(ctx == NULL) return SPNG_EINTERNAL;

    int ret;
    ret = read_data(ctx, 4);
    if(ret) return ret;

    ctx->current_chunk.crc = read_u32(ctx->data);

    if(ctx->skip_crc) return 0;

    if(ctx->cur_actual_crc != ctx->current_chunk.crc)
    {
        if(is_critical_chunk(&ctx->current_chunk))
        {
            if(ctx->crc_action_critical == SPNG_CRC_USE) return 0;
        }
        else
        {
            if(ctx->crc_action_ancillary == SPNG_CRC_USE) return 0;
            if(ctx->crc_action_ancillary == SPNG_CRC_DISCARD) return -SPNG_CRC_DISCARD;
        }

        return SPNG_ECHUNK_CRC;
    }

    return 0;
}

/* Read and validate the current chunk's crc and the next chunk header */
static inline int read_header(spng_ctx *ctx)
{
    if(ctx == NULL) return SPNG_EINTERNAL;

    int ret;
    struct spng_chunk chunk = { 0 };

    ret = read_and_check_crc(ctx);
    if(ret)
    {
        if(ret == -SPNG_CRC_DISCARD)
        {
            ctx->discard = 1;
        }
        else return ret;
    }

    ret = read_data(ctx, 8);
    if(ret) return ret;

    chunk.offset = ctx->bytes_read - 8;

    chunk.length = read_u32(ctx->data);

    memcpy(&chunk.type, ctx->data + 4, 4);

    if(chunk.length > png_u32max) return SPNG_ECHUNK_STDLEN;

    ctx->cur_chunk_bytes_left = chunk.length;

    if(is_critical_chunk(&chunk) && ctx->crc_action_critical == SPNG_CRC_USE) ctx->skip_crc = 1;
    else if(ctx->crc_action_ancillary == SPNG_CRC_USE) ctx->skip_crc = 1;
    else ctx->skip_crc = 0;

    if(!ctx->skip_crc)
    {
        ctx->cur_actual_crc = crc32(0, NULL, 0);
        ctx->cur_actual_crc = crc32(ctx->cur_actual_crc, chunk.type, 4);
    }

    ctx->current_chunk = chunk;

    return 0;
}

/* Read chunk bytes and update crc */
static int read_chunk_bytes(spng_ctx *ctx, uint32_t bytes)
{
    if(ctx == NULL) return SPNG_EINTERNAL;
    if(!ctx->cur_chunk_bytes_left || !bytes) return SPNG_EINTERNAL;
    if(bytes > ctx->cur_chunk_bytes_left) return SPNG_EINTERNAL; /* XXX: more specific error? */

    int ret;

    ret = read_data(ctx, bytes);
    if(ret) return ret;

    if(!ctx->skip_crc) ctx->cur_actual_crc = crc32(ctx->cur_actual_crc, ctx->data, bytes);

    ctx->cur_chunk_bytes_left -= bytes;

    return ret;
}

/* read_chunk_bytes() + read_data() with custom output buffer */
static int read_chunk_bytes2(spng_ctx *ctx, void *out, uint32_t bytes)
{
    if(ctx == NULL) return SPNG_EINTERNAL;
    if(!ctx->cur_chunk_bytes_left || !bytes) return SPNG_EINTERNAL;
    if(bytes > ctx->cur_chunk_bytes_left) return SPNG_EINTERNAL; /* XXX: more specific error? */

    int ret;
    uint32_t len = bytes;

    if(ctx->streaming && len > SPNG_READ_SIZE) len = SPNG_READ_SIZE;

    while(bytes)
    {
        if(len > bytes) len = bytes;

        ret = ctx->read_fn(ctx, ctx->read_user_ptr, out, len);
        if(ret) return ret;

        if(!ctx->streaming) memcpy(out, ctx->data, len);

        ctx->bytes_read += len;
        if(ctx->bytes_read < len) return SPNG_EOVERFLOW;

        if(!ctx->skip_crc) ctx->cur_actual_crc = crc32(ctx->cur_actual_crc, out, len);

        ctx->cur_chunk_bytes_left -= len;

        out = (char*)out + len;
        bytes -= len;
        len = SPNG_READ_SIZE;
    }

    return 0;
}

static int discard_chunk_bytes(spng_ctx *ctx, uint32_t bytes)
{
    if(ctx == NULL) return SPNG_EINTERNAL;
    if(!bytes) return 0;

    int ret;

    if(ctx->streaming) /* Do small, consecutive reads */
    {
        while(bytes)
        {
            uint32_t len = SPNG_READ_SIZE;

            if(len > bytes) len = bytes;

            ret = read_chunk_bytes(ctx, len);
            if(ret) return ret;

            bytes -= len;
        }
    }
    else
    {
        ret = read_chunk_bytes(ctx, bytes);
        if(ret) return ret;
    }

    return 0;
}

static int spng__inflate_init(spng_ctx *ctx)
{
    if(ctx->zstream.state) inflateEnd(&ctx->zstream);

    ctx->zstream.zalloc = spng__zalloc;
    ctx->zstream.zfree = spng__zfree;
    ctx->zstream.opaque = ctx;

    if(inflateInit(&ctx->zstream) != Z_OK) return SPNG_EZLIB_INIT;

#if ZLIB_VERNUM >= 0x1290 && !defined(SPNG_USE_MINIZ)

    int validate = 1;

    if(ctx->flags & SPNG_CTX_IGNORE_ADLER32) validate = 0;

    if(is_critical_chunk(&ctx->current_chunk))
    {
        if(ctx->crc_action_critical == SPNG_CRC_USE) validate = 0;
    }
    else /* ancillary */
    {
        if(ctx->crc_action_ancillary == SPNG_CRC_USE) validate = 0;
    }

    if(inflateValidate(&ctx->zstream, validate)) return SPNG_EZLIB_INIT;

#else /* This requires zlib >= 1.2.11 */
    //#pragma message ("inflateValidate() not available, SPNG_CTX_IGNORE_ADLER32 will be ignored")
#endif

    return 0;
}

/* Inflate a zlib stream starting with start_buf if non-NULL,
   continuing from the datastream till an end marker,
   allocating and writing the inflated stream to *out,
   leaving "extra" bytes at the end, final buffer length is *len.

   Takes into account the chunk size and cache limits.
*/
static int spng__inflate_stream(spng_ctx *ctx, char **out, size_t *len, size_t extra, const void *start_buf, size_t start_len)
{
    int ret = spng__inflate_init(ctx);
    if(ret) return ret;

    size_t max = ctx->chunk_cache_limit - ctx->chunk_cache_usage;

    if(ctx->max_chunk_size < max) max = ctx->max_chunk_size;

    if(extra > max) return SPNG_ECHUNK_LIMITS;
    max -= extra;

    uint32_t read_size;
    size_t size = 8 * 1024;
    void *t, *buf = spng__malloc(ctx, size);

    if(buf == NULL) return SPNG_EMEM;

    z_stream *stream = &ctx->zstream;

    if(start_buf != NULL && start_len)
    {
        stream->avail_in = (uInt)start_len;
        stream->next_in = start_buf;
    }
    else
    {
        stream->avail_in = 0;
        stream->next_in = NULL;
    }

    stream->avail_out = (uInt)size;
    stream->next_out = buf;

    while(ret != Z_STREAM_END)
    {
        ret = inflate(stream, 0);

        if(ret == Z_STREAM_END) break;

        if(ret != Z_OK && ret != Z_BUF_ERROR)
        {
            ret = SPNG_EZLIB;
            goto err;
        }

        if(!stream->avail_out) /* Resize buffer */
        {
            /* overflow or reached chunk/cache limit */
            if( (2 > SIZE_MAX / size) || (size > max / 2) )
            {
                ret = SPNG_ECHUNK_LIMITS;
                goto err;
            }

            size *= 2;

            t = spng__realloc(ctx, buf, size);
            if(t == NULL) goto mem;

            buf = t;

            stream->avail_out = (uInt)size / 2;
            stream->next_out = (unsigned char*)buf + size / 2;
        }
        else if(!stream->avail_in) /* Read more chunk bytes */
        {
            read_size = ctx->cur_chunk_bytes_left;
            if(ctx->streaming && read_size > SPNG_READ_SIZE) read_size = SPNG_READ_SIZE;

            ret = read_chunk_bytes(ctx, read_size);

            if(ret)
            {
                if(!read_size) ret = SPNG_EZLIB;

                goto err;
            }

            stream->avail_in = read_size;
            stream->next_in = ctx->data;
        }
    }

    size = stream->total_out;

    if(!size)
    {
        ret = SPNG_EZLIB;
        goto err;
    }

    size += extra;
    if(size < extra) goto mem;

    t = spng__realloc(ctx, buf, size);
    if(t == NULL) goto mem;

    buf = t;

    (void)increase_cache_usage(ctx, size);

    *out = buf;
    *len = size;

    return 0;

mem:
    ret = SPNG_EMEM;
err:
    spng__free(ctx, buf);
    return ret;
}

/* Read at least one byte from the IDAT stream */
static int read_idat_bytes(spng_ctx *ctx, uint32_t *bytes_read)
{
    if(ctx == NULL || bytes_read == NULL) return SPNG_EINTERNAL;
    if(memcmp(ctx->current_chunk.type, type_idat, 4)) return SPNG_EIDAT_TOO_SHORT;

    int ret;
    uint32_t len;

    while(!ctx->cur_chunk_bytes_left)
    {
        ret = read_header(ctx);
        if(ret) return ret;

        if(memcmp(ctx->current_chunk.type, type_idat, 4)) return SPNG_EIDAT_TOO_SHORT;
    }

    if(ctx->streaming)
    {/* TODO: estimate bytes to read for progressive reads */
        len = SPNG_READ_SIZE;
        if(len > ctx->cur_chunk_bytes_left) len = ctx->cur_chunk_bytes_left;
    }
    else len = ctx->current_chunk.length;

    ret = read_chunk_bytes(ctx, len);

    *bytes_read = len;

    return ret;
}

static int read_scanline_bytes(spng_ctx *ctx, unsigned char *dest, size_t len)
{
    if(ctx == NULL || dest == NULL) return SPNG_EINTERNAL;

    int ret = Z_OK;
    uint32_t bytes_read;

    z_stream *zstream = &ctx->zstream;

    zstream->avail_out = (uInt)len;
    zstream->next_out = dest;

    while(zstream->avail_out != 0)
    {
        ret = inflate(&ctx->zstream, 0);

        if(ret == Z_OK) continue;

        if(ret == Z_STREAM_END) /* Reached an end-marker */
        {
            if(zstream->avail_out != 0) return SPNG_EIDAT_TOO_SHORT;
        }
        else if(ret == Z_BUF_ERROR) /* Read more IDAT bytes */
        {
            ret = read_idat_bytes(ctx, &bytes_read);
            if(ret) return ret;

            zstream->avail_in = bytes_read;
            zstream->next_in = ctx->data;
        }
        else return SPNG_EIDAT_STREAM;
    }

    return 0;
}

static uint8_t paeth(uint8_t a, uint8_t b, uint8_t c)
{
    int16_t p = (int16_t)a + (int16_t)b - (int16_t)c;
    int16_t pa = abs(p - (int16_t)a);
    int16_t pb = abs(p - (int16_t)b);
    int16_t pc = abs(p - (int16_t)c);

    if(pa <= pb && pa <= pc) return a;
    else if(pb <= pc) return b;

    return c;
}

SPNG_TARGET_CLONES("default,avx2")
static void defilter_up(size_t bytes, unsigned char *row, const unsigned char *prev)
{
    size_t i;
    for(i=0; i < bytes; i++)
    {
        row[i] += prev[i];
    }
}

/* Defilter *scanline in-place.
   *prev_scanline and *scanline should point to the first pixel,
   scanline_width is the width of the scanline including the filter byte.
*/
static int defilter_scanline(const unsigned char *prev_scanline, unsigned char *scanline,
                             size_t scanline_width, unsigned bytes_per_pixel, unsigned filter)
{
    if(prev_scanline == NULL || scanline == NULL || !scanline_width) return SPNG_EINTERNAL;

    size_t i;
    scanline_width--;

    if(filter == 0) return 0;

#ifndef SPNG_DISABLE_OPT
    if(filter == SPNG_FILTER_UP) goto no_opt;

    if(bytes_per_pixel == 4)
    {
        if(filter == SPNG_FILTER_SUB)
            defilter_sub4(scanline_width, scanline);
        else if(filter == SPNG_FILTER_AVERAGE)
            defilter_avg4(scanline_width, scanline, prev_scanline);
        else if(filter == SPNG_FILTER_PAETH)
            defilter_paeth4(scanline_width, scanline, prev_scanline);
        else return SPNG_EFILTER;

        return 0;
    }
    else if(bytes_per_pixel == 3)
    {
        if(filter == SPNG_FILTER_SUB)
            defilter_sub3(scanline_width, scanline);
        else if(filter == SPNG_FILTER_AVERAGE)
            defilter_avg3(scanline_width, scanline, prev_scanline);
        else if(filter == SPNG_FILTER_PAETH)
            defilter_paeth3(scanline_width, scanline, prev_scanline);
        else return SPNG_EFILTER;

        return 0;
    }
no_opt:
#endif

    if(filter == SPNG_FILTER_UP)
    {
        defilter_up(scanline_width, scanline, prev_scanline);
        return 0;
    }

    for(i=0; i < scanline_width; i++)
    {
        uint8_t x, a, b, c;

        if(i >= bytes_per_pixel)
        {
            a = scanline[i - bytes_per_pixel];
            b = prev_scanline[i];
            c = prev_scanline[i - bytes_per_pixel];
        }
        else /* First pixel in row */
        {
            a = 0;
            b = prev_scanline[i];
            c = 0;
        }

        x = scanline[i];

        switch(filter)
        {
            case SPNG_FILTER_SUB:
            {
                x = x + a;
                break;
            }
            case SPNG_FILTER_AVERAGE:
            {
                uint16_t avg = (a + b) / 2;
                x = x + avg;
                break;
            }
            case SPNG_FILTER_PAETH:
            {
                x = x + paeth(a,b,c);
                break;
            }
        }

        scanline[i] = x;
    }

    return 0;
}

/* Scale "sbits" significant bits in "sample" from "bit_depth" to "target"

   "bit_depth" must be a valid PNG depth
   "sbits" must be less than or equal to "bit_depth"
   "target" must be between 1 and 16
*/
static uint16_t sample_to_target(uint16_t sample, unsigned bit_depth, unsigned sbits, unsigned target)
{
    if(bit_depth == sbits)
    {
        if(target == sbits) return sample; /* No scaling */
    }/* bit_depth > sbits */
    else sample = sample >> (bit_depth - sbits); /* Shift significant bits to bottom */

    /* Downscale */
    if(target < sbits) return sample >> (sbits - target);

    /* Upscale using left bit replication */
    int8_t shift_amount = target - sbits;
    uint16_t sample_bits = sample;
    sample = 0;

    while(shift_amount >= 0)
    {
        sample = sample | (sample_bits << shift_amount);
        shift_amount -= sbits;
    }

    int8_t partial = shift_amount + (int8_t)sbits;

    if(partial != 0) sample = sample | (sample_bits >> abs(shift_amount));

    return sample;
}

static inline void gamma_correct_row(unsigned char *row, uint32_t pixels, int fmt, const uint16_t *gamma_lut)
{
    uint32_t i;

    if(fmt == SPNG_FMT_RGBA8)
    {
        unsigned char *px;
        for(i=0; i < pixels; i++)
        {
            px = row + i * 4;

            px[0] = gamma_lut[px[0]];
            px[1] = gamma_lut[px[1]];
            px[2] = gamma_lut[px[2]];
        }
    }
    else if(fmt == SPNG_FMT_RGBA16)
    {
        for(i=0; i < pixels; i++)
        {
            uint16_t px[4];
            memcpy(px, row + i * 8, 8);

            px[0] = gamma_lut[px[0]];
            px[1] = gamma_lut[px[1]];
            px[2] = gamma_lut[px[2]];

            memcpy(row + i * 8, px, 8);
        }
    }
    else if(fmt == SPNG_FMT_RGB8)
    {
        unsigned char *px;
        for(i=0; i < pixels; i++)
        {
            px = row + i * 3;

            px[0] = gamma_lut[px[0]];
            px[1] = gamma_lut[px[1]];
            px[2] = gamma_lut[px[2]];
        }
    }
}

/* Apply transparency to output row */
static inline void trns_row(unsigned char *row,
                            const unsigned char *scanline,
                            const unsigned char *trns,
                            unsigned scanline_stride,
                            struct spng_ihdr *ihdr,
                            uint32_t pixels,
                            int fmt)
{
    uint32_t i;
    unsigned row_stride;
    unsigned depth = ihdr->bit_depth;

    if(fmt == SPNG_FMT_RGBA8)
    {
        if(ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE) return; /* already applied in the decoding loop */

        row_stride = 4;
        for(i=0; i < pixels; i++, scanline+=scanline_stride, row+=row_stride)
        {
            if(!memcmp(scanline, trns, scanline_stride)) row[3] = 0;
        }
    }
    else if(fmt == SPNG_FMT_RGBA16)
    {
        if(ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE) return; /* already applied in the decoding loop */

        row_stride = 8;
        for(i=0; i < pixels; i++, scanline+=scanline_stride, row+=row_stride)
        {
            if(!memcmp(scanline, trns, scanline_stride)) memset(row + 6, 0, 2);
        }
    }
    else if(fmt == SPNG_FMT_GA8)
    {
        row_stride = 2;

        if(depth == 16)
        {
            for(i=0; i < pixels; i++, scanline+=scanline_stride, row+=row_stride)
            {
                if(!memcmp(scanline, trns, scanline_stride)) memset(row + 1, 0, 1);
            }
        }
        else /* depth <= 8 */
        {
            struct spng__iter iter = spng__iter_init(depth, scanline);

            for(i=0; i < pixels; i++, row+=row_stride)
            {
                if(trns[0] == get_sample(&iter)) row[1] = 0;
            }
        }
    }
    else if(fmt == SPNG_FMT_GA16)
    {
        row_stride = 4;

        if(depth == 16)
        {
            for(i=0; i< pixels; i++, scanline+=scanline_stride, row+=row_stride)
            {
                if(!memcmp(scanline, trns, 2)) memset(row + 2, 0, 2);
            }
        }
        else
        {
            struct spng__iter iter = spng__iter_init(depth, scanline);

            for(i=0; i< pixels; i++, row+=row_stride)
            {
                if(trns[0] == get_sample(&iter)) memset(row + 2, 0, 2);
            }
        }
    }
    else return;
}

static inline void scale_row(unsigned char *row, uint32_t pixels, int fmt, unsigned depth, const struct spng_sbit *sbit)
{
    uint32_t i;

    if(fmt == SPNG_FMT_RGBA8)
    {
        unsigned char px[4];
        for(i=0; i < pixels; i++)
        {
            memcpy(px, row + i * 4, 4);

            px[0] = sample_to_target(px[0], depth, sbit->red_bits, 8);
            px[1] = sample_to_target(px[1], depth, sbit->green_bits, 8);
            px[2] = sample_to_target(px[2], depth, sbit->blue_bits, 8);
            px[3] = sample_to_target(px[3], depth, sbit->alpha_bits, 8);

            memcpy(row + i * 4, px, 4);
        }
    }
    else if(fmt == SPNG_FMT_RGBA16)
    {
        uint16_t px[4];
        for(i=0; i < pixels; i++)
        {
            memcpy(px, row + i * 8, 8);

            px[0] = sample_to_target(px[0], depth, sbit->red_bits, 16);
            px[1] = sample_to_target(px[1], depth, sbit->green_bits, 16);
            px[2] = sample_to_target(px[2], depth, sbit->blue_bits, 16);
            px[3] = sample_to_target(px[3], depth, sbit->alpha_bits, 16);

            memcpy(row + i * 8, px, 8);
        }
    }
    else if(fmt == SPNG_FMT_RGB8)
    {
        unsigned char px[4];
        for(i=0; i < pixels; i++)
        {
            memcpy(px, row + i * 3, 3);

            px[0] = sample_to_target(px[0], depth, sbit->red_bits, 8);
            px[1] = sample_to_target(px[1], depth, sbit->green_bits, 8);
            px[2] = sample_to_target(px[2], depth, sbit->blue_bits, 8);

            memcpy(row + i * 3, px, 3);
        }
    }
    else if(fmt == SPNG_FMT_G8)
    {
        for(i=0; i < pixels; i++)
        {
            row[i] = sample_to_target(row[i], depth, sbit->grayscale_bits, 8);
        }
    }
    else if(fmt == SPNG_FMT_GA8)
    {
        for(i=0; i < pixels; i++)
        {
            row[i*2] = sample_to_target(row[i*2], depth, sbit->grayscale_bits, 8);
        }
    }
}

/* Expand to *row using 8-bit palette indices from *scanline */
void expand_row(unsigned char *row, const unsigned char *scanline, const struct spng_plte_entry16 *plte, uint32_t width, int fmt)
{
    uint32_t i;
    unsigned char *px;
    unsigned char entry;
    if(fmt == SPNG_FMT_RGBA8)
    {
        for(i=0; i < width; i++)
        {
            px = row + i * 4;
            entry = scanline[i];
            px[0] = plte[entry].red;
            px[1] = plte[entry].green;
            px[2] = plte[entry].blue;
            px[3] = plte[entry].alpha;
        }
    }
    else if(fmt == SPNG_FMT_RGB8)
    {
        for(i=0; i < width; i++)
        {
            px = row + i * 3;
            entry = scanline[i];
            px[0] = plte[entry].red;
            px[1] = plte[entry].green;
            px[2] = plte[entry].blue;
        }
    }
}

/* Unpack 1/2/4/8-bit samples to G8/GA8/GA16 or G16 -> GA16 */
static void unpack_scanline(unsigned char *out, const unsigned char *scanline, uint32_t width, unsigned bit_depth, int fmt)
{
    struct spng__iter iter = spng__iter_init(bit_depth, scanline);
    uint32_t i;
    uint16_t sample, alpha = 65535;


    if(fmt == SPNG_FMT_GA8) goto ga8;
    else if(fmt == SPNG_FMT_GA16) goto ga16;

    /* 1/2/4-bit -> 8-bit */
    for(i=0; i < width; i++) out[i] = get_sample(&iter);

    return;

ga8:
    /* 1/2/4/8-bit -> GA8 */
    for(i=0; i < width; i++)
    {
        out[i*2] = get_sample(&iter);
        out[i*2 + 1] = 255;
    }

    return;

ga16:

    /* 16 -> GA16 */
    if(bit_depth == 16)
    {
        for(i=0; i < width; i++)
        {
            memcpy(out + i * 4, scanline + i * 2, 2);
            memcpy(out + i * 4 + 2, &alpha, 2);
        }
        return;
    }

     /* 1/2/4/8-bit -> GA16 */
    for(i=0; i < width; i++)
    {
        sample = get_sample(&iter);
        memcpy(out + i * 4, &sample, 2);
        memcpy(out + i * 4 + 2, &alpha, 2);
    }
}

static int check_ihdr(const struct spng_ihdr *ihdr, uint32_t max_width, uint32_t max_height)
{
    if(ihdr->width > png_u32max || ihdr->width > max_width || !ihdr->width) return SPNG_EWIDTH;
    if(ihdr->height > png_u32max || ihdr->height > max_height || !ihdr->height) return SPNG_EHEIGHT;

    switch(ihdr->color_type)
    {
        case SPNG_COLOR_TYPE_GRAYSCALE:
        {
            if( !(ihdr->bit_depth == 1 || ihdr->bit_depth == 2 ||
                  ihdr->bit_depth == 4 || ihdr->bit_depth == 8 ||
                  ihdr->bit_depth == 16) )
                  return SPNG_EBIT_DEPTH;

            break;
        }
        case SPNG_COLOR_TYPE_TRUECOLOR:
        case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA:
        case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA:
        {
            if( !(ihdr->bit_depth == 8 || ihdr->bit_depth == 16) )
                return SPNG_EBIT_DEPTH;

            break;
        }
        case SPNG_COLOR_TYPE_INDEXED:
        {
            if( !(ihdr->bit_depth == 1 || ihdr->bit_depth == 2 ||
                  ihdr->bit_depth == 4 || ihdr->bit_depth == 8) )
                return SPNG_EBIT_DEPTH;

            break;
        }
        default: return SPNG_ECOLOR_TYPE;
    }

    if(ihdr->compression_method) return SPNG_ECOMPRESSION_METHOD;
    if(ihdr->filter_method) return SPNG_EFILTER_METHOD;

    if(ihdr->interlace_method > 1) return SPNG_EINTERLACE_METHOD;

    return 0;
}

static int check_plte(const struct spng_plte *plte, const struct spng_ihdr *ihdr)
{
    if(plte == NULL || ihdr == NULL) return 1;

    if(plte->n_entries == 0) return 1;
    if(plte->n_entries > 256) return 1;

    if(ihdr->color_type == SPNG_COLOR_TYPE_INDEXED)
    {
        if(plte->n_entries > (1U << ihdr->bit_depth)) return 1;
    }

    return 0;
}

static int check_sbit(const struct spng_sbit *sbit, const struct spng_ihdr *ihdr)
{
    if(sbit == NULL || ihdr == NULL) return 1;

    if(ihdr->color_type == 0)
    {
        if(sbit->grayscale_bits == 0) return SPNG_ESBIT;
        if(sbit->grayscale_bits > ihdr->bit_depth) return SPNG_ESBIT;
    }
    else if(ihdr->color_type == 2 || ihdr->color_type == 3)
    {
        if(sbit->red_bits == 0) return SPNG_ESBIT;
        if(sbit->green_bits == 0) return SPNG_ESBIT;
        if(sbit->blue_bits == 0) return SPNG_ESBIT;

        uint8_t bit_depth;
        if(ihdr->color_type == 3) bit_depth = 8;
        else bit_depth = ihdr->bit_depth;

        if(sbit->red_bits > bit_depth) return SPNG_ESBIT;
        if(sbit->green_bits > bit_depth) return SPNG_ESBIT;
        if(sbit->blue_bits > bit_depth) return SPNG_ESBIT;
    }
    else if(ihdr->color_type == 4)
    {
        if(sbit->grayscale_bits == 0) return SPNG_ESBIT;
        if(sbit->alpha_bits == 0) return SPNG_ESBIT;

        if(sbit->grayscale_bits > ihdr->bit_depth) return SPNG_ESBIT;
        if(sbit->alpha_bits > ihdr->bit_depth) return SPNG_ESBIT;
    }
    else if(ihdr->color_type == 6)
    {
        if(sbit->red_bits == 0) return SPNG_ESBIT;
        if(sbit->green_bits == 0) return SPNG_ESBIT;
        if(sbit->blue_bits == 0) return SPNG_ESBIT;
        if(sbit->alpha_bits == 0) return SPNG_ESBIT;

        if(sbit->red_bits > ihdr->bit_depth) return SPNG_ESBIT;
        if(sbit->green_bits > ihdr->bit_depth) return SPNG_ESBIT;
        if(sbit->blue_bits > ihdr->bit_depth) return SPNG_ESBIT;
        if(sbit->alpha_bits > ihdr->bit_depth) return SPNG_ESBIT;
    }

    return 0;
}

static int check_chrm_int(const struct spng_chrm_int *chrm_int)
{
    if(chrm_int == NULL) return 1;

    if(chrm_int->white_point_x > png_u32max ||
       chrm_int->white_point_y > png_u32max ||
       chrm_int->red_x > png_u32max ||
       chrm_int->red_y > png_u32max ||
       chrm_int->green_x  > png_u32max ||
       chrm_int->green_y  > png_u32max ||
       chrm_int->blue_x > png_u32max ||
       chrm_int->blue_y > png_u32max) return SPNG_ECHRM;

    return 0;
}

static int check_phys(const struct spng_phys *phys)
{
    if(phys == NULL) return 1;

    if(phys->unit_specifier > 1) return SPNG_EPHYS;

    if(phys->ppu_x > png_u32max) return SPNG_EPHYS;
    if(phys->ppu_y > png_u32max) return SPNG_EPHYS;

    return 0;
}

static int check_time(const struct spng_time *time)
{
    if(time == NULL) return 1;

    if(time->month == 0 || time->month > 12) return 1;
    if(time->day == 0 || time->day > 31) return 1;
    if(time->hour > 23) return 1;
    if(time->minute > 59) return 1;
    if(time->second > 60) return 1;

    return 0;
}

static int check_offs(const struct spng_offs *offs)
{
    if(offs == NULL) return 1;

    if(offs->unit_specifier > 1) return 1;

    return 0;
}

static int check_exif(const struct spng_exif *exif)
{
    if(exif == NULL) return 1;
    if(exif->data == NULL) return 1;

    if(exif->length < 4) return SPNG_ECHUNK_SIZE;
    if(exif->length > png_u32max) return SPNG_ECHUNK_STDLEN;

    const uint8_t exif_le[4] = { 73, 73, 42, 0 };
    const uint8_t exif_be[4] = { 77, 77, 0, 42 };

    if(memcmp(exif->data, exif_le, 4) && memcmp(exif->data, exif_be, 4)) return 1;

    return 0;
}

/* Validate PNG keyword */
static int check_png_keyword(const char *str)
{
    if(str == NULL) return 1;
    char len = (char)strlen(str);
    const char *end = str + len;

    if(!len) return 1;
    if(str[0] == ' ') return 1; /* Leading space */
    if(end[-1] == ' ') return 1; /* Trailing space */
    if(strstr(str, "  ") != NULL) return 1; /* Consecutive spaces */

    uint8_t c;
    while(str != end)
    {
        memcpy(&c, str, 1);

        if( (c >= 32 && c <= 126) || (c >= 161) ) str++;
        else return 1; /* Invalid character */
    }

    return 0;
}

/* Validate PNG text *str up to 'len' bytes */
static int check_png_text(const char *str, size_t len)
{/* XXX: are consecutive newlines permitted? */
    if(str == NULL || len == 0) return 1;

    uint8_t c;
    size_t i = 0;
    while(i < len)
    {
        memcpy(&c, str + i, 1);

        if( (c >= 32 && c <= 126) || (c >= 161) || c == 10) i++;
        else return 1; /* Invalid character */
    }

    return 0;
}

/* Returns non-zero for standard chunks which are stored without allocating memory */
static int is_small_chunk(uint8_t type[4])
{
    if(!memcmp(type, type_plte, 4)) return 1;
    else if(!memcmp(type, type_chrm, 4)) return 1;
    else if(!memcmp(type, type_gama, 4)) return 1;
    else if(!memcmp(type, type_sbit, 4)) return 1;
    else if(!memcmp(type, type_srgb, 4)) return 1;
    else if(!memcmp(type, type_bkgd, 4)) return 1;
    else if(!memcmp(type, type_trns, 4)) return 1;
    else if(!memcmp(type, type_hist, 4)) return 1;
    else if(!memcmp(type, type_phys, 4)) return 1;
    else if(!memcmp(type, type_time, 4)) return 1;
    else if(!memcmp(type, type_offs, 4)) return 1;
    else return 0;
}

static int read_ihdr(spng_ctx *ctx)
{
    int ret;
    struct spng_chunk chunk;
    const unsigned char *data;

    chunk.offset = 8;
    chunk.length = 13;
    size_t sizeof_sig_ihdr = 29;

    ret = read_data(ctx, sizeof_sig_ihdr);
    if(ret) return ret;

    data = ctx->data;

    if(memcmp(data, png_signature, sizeof(png_signature))) return SPNG_ESIGNATURE;

    chunk.length = read_u32(data + 8);
    memcpy(&chunk.type, data + 12, 4);

    if(chunk.length != 13) return SPNG_EIHDR_SIZE;
    if(memcmp(chunk.type, type_ihdr, 4)) return SPNG_ENOIHDR;

    ctx->cur_actual_crc = crc32(0, NULL, 0);
    ctx->cur_actual_crc = crc32(ctx->cur_actual_crc, data + 12, 17);

    ctx->ihdr.width = read_u32(data + 16);
    ctx->ihdr.height = read_u32(data + 20);
    ctx->ihdr.bit_depth = data[24];
    ctx->ihdr.color_type = data[25];
    ctx->ihdr.compression_method = data[26];
    ctx->ihdr.filter_method = data[27];
    ctx->ihdr.interlace_method = data[28];

    if(!ctx->max_width) ctx->max_width = png_u32max;
    if(!ctx->max_height) ctx->max_height = png_u32max;

    ret = check_ihdr(&ctx->ihdr, ctx->max_width, ctx->max_height);
    if(ret) return ret;

    ctx->file.ihdr = 1;
    ctx->stored.ihdr = 1;

    ctx->channels = 1; /* grayscale or indexed color */

    if(ctx->ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR) ctx->channels = 3;
    else if(ctx->ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA) ctx->channels = 2;
    else if(ctx->ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA) ctx->channels = 4;

    if(ctx->ihdr.bit_depth < 8) ctx->bytes_per_pixel = 1;
    else ctx->bytes_per_pixel = ctx->channels * (ctx->ihdr.bit_depth / 8);

    ret = calculate_subimages(ctx);
    if(ret) return ret;

    return 0;
}

static void splt_undo(spng_ctx *ctx)
{
    struct spng_splt *splt = &ctx->splt_list[ctx->n_splt - 1];

    spng__free(ctx, splt->entries);

    decrease_cache_usage(ctx, sizeof(struct spng_splt));
    decrease_cache_usage(ctx, splt->n_entries * sizeof(struct spng_splt_entry));

    splt->entries = NULL;

    ctx->n_splt--;
}

static void text_undo(spng_ctx *ctx)
{
    struct spng_text2 *text = &ctx->text_list[ctx->n_text - 1];

    spng__free(ctx, text->keyword);
    if(text->compression_flag) spng__free(ctx, text->text);

    decrease_cache_usage(ctx, text->cache_usage);
    decrease_cache_usage(ctx, sizeof(struct spng_text2));

    text->keyword = NULL;
    text->text = NULL;

    ctx->n_text--;
}

static int read_non_idat_chunks(spng_ctx *ctx)
{
    int ret;
    struct spng_chunk chunk;
    const unsigned char *data;

    ctx->discard = 0;
    ctx->undo = NULL;
    ctx->prev_stored = ctx->stored;

    while( !(ret = read_header(ctx)))
    {
        if(ctx->discard)
        {
            if(ctx->undo) ctx->undo(ctx);

            ctx->stored = ctx->prev_stored;
        }

        ctx->discard = 0;
        ctx->undo = NULL;

        ctx->prev_stored = ctx->stored;
        chunk = ctx->current_chunk;

        if(!memcmp(chunk.type, type_idat, 4))
        {
            if(ctx->state < SPNG_STATE_FIRST_IDAT)
            {
                if(ctx->ihdr.color_type == 3 && !ctx->stored.plte) return SPNG_ENOPLTE;

                ctx->first_idat = chunk;
                return 0;
            }

            if(ctx->prev_was_idat)
            {
                /* Ignore extra IDAT's */
                ret = discard_chunk_bytes(ctx, chunk.length);
                if(ret) return ret;

                continue;
            }
            else return SPNG_ECHUNK_POS; /* IDAT chunk not at the end of the IDAT sequence */
        }

        ctx->prev_was_idat = 0;

        if(is_small_chunk(chunk.type))
        {
            /* None of the known chunks can be zero length */
            if(!chunk.length) return SPNG_ECHUNK_SIZE;

            /* The largest of these chunks is PLTE with 256 entries */
            ret = read_chunk_bytes(ctx, chunk.length > 768 ? 768 : chunk.length);
            if(ret) return ret;
        }

        data = ctx->data;

        if(is_critical_chunk(&chunk))
        {
            if(!memcmp(chunk.type, type_plte, 4))
            {
                if(ctx->file.trns || ctx->file.hist || ctx->file.bkgd) return SPNG_ECHUNK_POS;
                if(chunk.length % 3 != 0) return SPNG_ECHUNK_SIZE;

                ctx->plte.n_entries = chunk.length / 3;

                if(check_plte(&ctx->plte, &ctx->ihdr)) return SPNG_ECHUNK_SIZE; /* XXX: EPLTE? */

                size_t i;
                for(i=0; i < ctx->plte.n_entries; i++)
                {
                    ctx->plte.entries[i].red   = data[i * 3];
                    ctx->plte.entries[i].green = data[i * 3 + 1];
                    ctx->plte.entries[i].blue  = data[i * 3 + 2];
                }

                ctx->file.plte = 1;
                ctx->stored.plte = 1;
            }
            else if(!memcmp(chunk.type, type_iend, 4))
            {
                if(ctx->state == SPNG_STATE_AFTER_IDAT)
                {
                    if(chunk.length) return SPNG_ECHUNK_SIZE;

                    ret = read_and_check_crc(ctx);
                    if(ret == -SPNG_CRC_DISCARD) ret = 0;

                    return ret;
                }
                else return SPNG_ECHUNK_POS;
            }
            else if(!memcmp(chunk.type, type_ihdr, 4)) return SPNG_ECHUNK_POS;
            else return SPNG_ECHUNK_UNKNOWN_CRITICAL;
        }
        else if(!memcmp(chunk.type, type_chrm, 4)) /* Ancillary chunks */
        {
            if(ctx->file.plte) return SPNG_ECHUNK_POS;
            if(ctx->state == SPNG_STATE_AFTER_IDAT) return SPNG_ECHUNK_POS;
            if(ctx->file.chrm) return SPNG_EDUP_CHRM;

            if(chunk.length != 32) return SPNG_ECHUNK_SIZE;

            ctx->chrm_int.white_point_x = read_u32(data);
            ctx->chrm_int.white_point_y = read_u32(data + 4);
            ctx->chrm_int.red_x = read_u32(data + 8);
            ctx->chrm_int.red_y = read_u32(data + 12);
            ctx->chrm_int.green_x = read_u32(data + 16);
            ctx->chrm_int.green_y = read_u32(data + 20);
            ctx->chrm_int.blue_x = read_u32(data + 24);
            ctx->chrm_int.blue_y = read_u32(data + 28);

            if(check_chrm_int(&ctx->chrm_int)) return SPNG_ECHRM;

            ctx->file.chrm = 1;
            ctx->stored.chrm = 1;
        }
        else if(!memcmp(chunk.type, type_gama, 4))
        {
            if(ctx->file.plte) return SPNG_ECHUNK_POS;
            if(ctx->state == SPNG_STATE_AFTER_IDAT) return SPNG_ECHUNK_POS;
            if(ctx->file.gama) return SPNG_EDUP_GAMA;

            if(chunk.length != 4) return SPNG_ECHUNK_SIZE;

            ctx->gama = read_u32(data);

            if(!ctx->gama) return SPNG_EGAMA;
            if(ctx->gama > png_u32max) return SPNG_EGAMA;

            ctx->file.gama = 1;
            ctx->stored.gama = 1;
        }
        else if(!memcmp(chunk.type, type_sbit, 4))
        {
            if(ctx->file.plte) return SPNG_ECHUNK_POS;
            if(ctx->state == SPNG_STATE_AFTER_IDAT) return SPNG_ECHUNK_POS;
            if(ctx->file.sbit) return SPNG_EDUP_SBIT;

            if(ctx->ihdr.color_type == 0)
            {
                if(chunk.length != 1) return SPNG_ECHUNK_SIZE;

                ctx->sbit.grayscale_bits = data[0];
            }
            else if(ctx->ihdr.color_type == 2 || ctx->ihdr.color_type == 3)
            {
                if(chunk.length != 3) return SPNG_ECHUNK_SIZE;

                ctx->sbit.red_bits = data[0];
                ctx->sbit.green_bits = data[1];
                ctx->sbit.blue_bits = data[2];
            }
            else if(ctx->ihdr.color_type == 4)
            {
                if(chunk.length != 2) return SPNG_ECHUNK_SIZE;

                ctx->sbit.grayscale_bits = data[0];
                ctx->sbit.alpha_bits = data[1];
            }
            else if(ctx->ihdr.color_type == 6)
            {
                if(chunk.length != 4) return SPNG_ECHUNK_SIZE;

                ctx->sbit.red_bits = data[0];
                ctx->sbit.green_bits = data[1];
                ctx->sbit.blue_bits = data[2];
                ctx->sbit.alpha_bits = data[3];
            }

            if(check_sbit(&ctx->sbit, &ctx->ihdr)) return SPNG_ESBIT;

            ctx->file.sbit = 1;
            ctx->stored.sbit = 1;
        }
        else if(!memcmp(chunk.type, type_srgb, 4))
        {
            if(ctx->file.plte) return SPNG_ECHUNK_POS;
            if(ctx->state == SPNG_STATE_AFTER_IDAT) return SPNG_ECHUNK_POS;
            if(ctx->file.srgb) return SPNG_EDUP_SRGB;

            if(chunk.length != 1) return SPNG_ECHUNK_SIZE;

            ctx->srgb_rendering_intent = data[0];

            if(ctx->srgb_rendering_intent > 3) return SPNG_ESRGB;

            ctx->file.srgb = 1;
            ctx->stored.srgb = 1;
        }
        else if(!memcmp(chunk.type, type_bkgd, 4))
        {
            if(ctx->state == SPNG_STATE_AFTER_IDAT) return SPNG_ECHUNK_POS;
            if(ctx->file.bkgd) return SPNG_EDUP_BKGD;

            uint16_t mask = ~0;
            if(ctx->ihdr.bit_depth < 16) mask = (1 << ctx->ihdr.bit_depth) - 1;

            if(ctx->ihdr.color_type == 0 || ctx->ihdr.color_type == 4)
            {
                if(chunk.length != 2) return SPNG_ECHUNK_SIZE;

                ctx->bkgd.gray = read_u16(data) & mask;
            }
            else if(ctx->ihdr.color_type == 2 || ctx->ihdr.color_type == 6)
            {
                if(chunk.length != 6) return SPNG_ECHUNK_SIZE;

                ctx->bkgd.red = read_u16(data) & mask;
                ctx->bkgd.green = read_u16(data + 2) & mask;
                ctx->bkgd.blue = read_u16(data + 4) & mask;
            }
            else if(ctx->ihdr.color_type == 3)
            {
                if(chunk.length != 1) return SPNG_ECHUNK_SIZE;
                if(!ctx->file.plte) return SPNG_EBKGD_NO_PLTE;

                ctx->bkgd.plte_index = data[0];
                if(ctx->bkgd.plte_index >= ctx->plte.n_entries) return SPNG_EBKGD_PLTE_IDX;
            }

            ctx->file.bkgd = 1;
            ctx->stored.bkgd = 1;
        }
        else if(!memcmp(chunk.type, type_trns, 4))
        {
            if(ctx->state == SPNG_STATE_AFTER_IDAT) return SPNG_ECHUNK_POS;
            if(ctx->file.trns) return SPNG_EDUP_TRNS;
            if(!chunk.length) return SPNG_ECHUNK_SIZE;

            uint16_t mask = ~0;
            if(ctx->ihdr.bit_depth < 16) mask = (1 << ctx->ihdr.bit_depth) - 1;

            if(ctx->ihdr.color_type == 0)
            {
                if(chunk.length != 2) return SPNG_ECHUNK_SIZE;

                ctx->trns.gray = read_u16(data) & mask;
            }
            else if(ctx->ihdr.color_type == 2)
            {
                if(chunk.length != 6) return SPNG_ECHUNK_SIZE;

                ctx->trns.red = read_u16(data) & mask;
                ctx->trns.green = read_u16(data + 2) & mask;
                ctx->trns.blue = read_u16(data + 4) & mask;
            }
            else if(ctx->ihdr.color_type == 3)
            {
                if(chunk.length > ctx->plte.n_entries) return SPNG_ECHUNK_SIZE;
                if(!ctx->file.plte) return SPNG_ETRNS_NO_PLTE;

                size_t k;
                for(k=0; k < chunk.length; k++)
                {
                    ctx->trns.type3_alpha[k] = data[k];
                }
                ctx->trns.n_type3_entries = chunk.length;
            }

            if(ctx->ihdr.color_type == 4 || ctx->ihdr.color_type == 6)  return SPNG_ETRNS_COLOR_TYPE;

            ctx->file.trns = 1;
            ctx->stored.trns = 1;
        }
        else if(!memcmp(chunk.type, type_hist, 4))
        {
            if(!ctx->file.plte) return SPNG_EHIST_NO_PLTE;
            if(ctx->state == SPNG_STATE_AFTER_IDAT) return SPNG_ECHUNK_POS;
            if(ctx->file.hist) return SPNG_EDUP_HIST;

            if( (chunk.length / 2) != (ctx->plte.n_entries) ) return SPNG_ECHUNK_SIZE;

            size_t k;
            for(k=0; k < (chunk.length / 2); k++)
            {
                ctx->hist.frequency[k] = read_u16(data + k*2);
            }

            ctx->file.hist = 1;
            ctx->stored.hist = 1;
        }
        else if(!memcmp(chunk.type, type_phys, 4))
        {
            if(ctx->state == SPNG_STATE_AFTER_IDAT) return SPNG_ECHUNK_POS;
            if(ctx->file.phys) return SPNG_EDUP_PHYS;

            if(chunk.length != 9) return SPNG_ECHUNK_SIZE;

            ctx->phys.ppu_x = read_u32(data);
            ctx->phys.ppu_y = read_u32(data + 4);
            ctx->phys.unit_specifier = data[8];

            if(check_phys(&ctx->phys)) return SPNG_EPHYS;

            ctx->file.phys = 1;
            ctx->stored.phys = 1;
        }
        else if(!memcmp(chunk.type, type_time, 4))
        {
            if(ctx->file.time) return SPNG_EDUP_TIME;

            if(chunk.length != 7) return SPNG_ECHUNK_SIZE;

            struct spng_time time;

            time.year = read_u16(data);
            time.month = data[2];
            time.day = data[3];
            time.hour = data[4];
            time.minute = data[5];
            time.second = data[6];

            if(check_time(&time)) return SPNG_ETIME;

            ctx->file.time = 1;

            if(!ctx->user.time) ctx->time = time;

            ctx->stored.time = 1;
        }
        else if(!memcmp(chunk.type, type_offs, 4))
        {
            if(ctx->state == SPNG_STATE_AFTER_IDAT) return SPNG_ECHUNK_POS;
            if(ctx->file.offs) return SPNG_EDUP_OFFS;

            if(chunk.length != 9) return SPNG_ECHUNK_SIZE;

            ctx->offs.x = read_s32(data);
            ctx->offs.y = read_s32(data + 4);
            ctx->offs.unit_specifier = data[8];

            if(check_offs(&ctx->offs)) return SPNG_EOFFS;

            ctx->file.offs = 1;
            ctx->stored.offs = 1;
        }
        else /* Arbitrary-length chunk */
        {

            if(!memcmp(chunk.type, type_exif, 4))
            {
                if(ctx->file.exif) return SPNG_EDUP_EXIF;

                ctx->file.exif = 1;

                if(ctx->user.exif) goto discard;

                if(increase_cache_usage(ctx, chunk.length)) return SPNG_ECHUNK_LIMITS;

                struct spng_exif exif;

                exif.length = chunk.length;

                exif.data = spng__malloc(ctx, chunk.length);
                if(exif.data == NULL) return SPNG_EMEM;

                ret = read_chunk_bytes2(ctx, exif.data, chunk.length);
                if(ret)
                {
                    spng__free(ctx, exif.data);
                    return ret;
                }

                if(check_exif(&exif))
                {
                    spng__free(ctx, exif.data);
                    return SPNG_EEXIF;
                }

                ctx->exif = exif;

                ctx->stored.exif = 1;
            }
            else if(!memcmp(chunk.type, type_iccp, 4))
            {/* TODO: add test file with color profile */
                if(ctx->file.plte) return SPNG_ECHUNK_POS;
                if(ctx->state == SPNG_STATE_AFTER_IDAT) return SPNG_ECHUNK_POS;
                if(ctx->file.iccp) return SPNG_EDUP_ICCP;
                if(!chunk.length) return SPNG_ECHUNK_SIZE;

                ctx->file.iccp = 1;

                uint32_t peek_bytes =  81 > chunk.length ? chunk.length : 81;

                ret = read_chunk_bytes(ctx, peek_bytes);
                if(ret) return ret;

                unsigned char *keyword_nul = memchr(ctx->data, '\0', peek_bytes);
                if(keyword_nul == NULL) return SPNG_EICCP_NAME;

                uint32_t keyword_len = keyword_nul - ctx->data;

                if(keyword_len > 79) return SPNG_EICCP_NAME;

                memcpy(ctx->iccp.profile_name, ctx->data, keyword_len);

                if(check_png_keyword(ctx->iccp.profile_name)) return SPNG_EICCP_NAME;

                if(chunk.length < (keyword_len + 2)) return SPNG_ECHUNK_SIZE;

                if(ctx->data[keyword_len + 1] != 0) return SPNG_EICCP_COMPRESSION_METHOD;

                ret = spng__inflate_stream(ctx, &ctx->iccp.profile, &ctx->iccp.profile_len, 0, ctx->data + keyword_len + 2, peek_bytes - (keyword_len + 2));

                if(ret) return ret;

                ctx->stored.iccp = 1;
            }
             else if(!memcmp(chunk.type, type_text, 4) ||
                     !memcmp(chunk.type, type_ztxt, 4) ||
                     !memcmp(chunk.type, type_itxt, 4))
            {
                if(!chunk.length) return SPNG_ECHUNK_SIZE;

                ctx->file.text = 1;

                if(ctx->user.text) goto discard;

                if(increase_cache_usage(ctx, sizeof(struct spng_text2))) return SPNG_ECHUNK_LIMITS;

                ctx->n_text++;
                if(ctx->n_text < 1) return SPNG_EOVERFLOW;
                if(sizeof(struct spng_text2) > SIZE_MAX / ctx->n_text) return SPNG_EOVERFLOW;

                void *buf = spng__realloc(ctx, ctx->text_list, ctx->n_text * sizeof(struct spng_text2));
                if(buf == NULL) return SPNG_EMEM;
                ctx->text_list = buf;

                struct spng_text2 *text = &ctx->text_list[ctx->n_text - 1];
                memset(text, 0, sizeof(struct spng_text2));

                ctx->undo = text_undo;

                uint32_t text_offset = 0, language_tag_offset = 0, translated_keyword_offset = 0;
                uint32_t peek_bytes = 256; /* enough for 3 80-byte keywords and some text bytes */
                uint32_t keyword_len;

                if(peek_bytes > chunk.length) peek_bytes = chunk.length;

                ret = read_chunk_bytes(ctx, peek_bytes);
                if(ret) return ret;

                data = ctx->data;

                const unsigned char *zlib_stream = NULL;
                const unsigned char *peek_end = data + peek_bytes;
                const unsigned char *keyword_nul = memchr(data, 0, chunk.length > 80 ? 80 : chunk.length);

                if(keyword_nul == NULL) return SPNG_ETEXT_KEYWORD;

                keyword_len = keyword_nul - data;

                if(!memcmp(chunk.type, type_text, 4))
                {
                    text->type = SPNG_TEXT;

                    text->text_length = chunk.length - keyword_len - 1;

                    text_offset = keyword_len;

                    /* increment past nul if there is a text field */
                    if(text->text_length) text_offset++;
                }
                else if(!memcmp(chunk.type, type_ztxt, 4))
                {
                    text->type = SPNG_ZTXT;

                    if((peek_bytes - keyword_len) <= 2) return SPNG_EZTXT;

                    if(keyword_nul[1]) return SPNG_EZTXT_COMPRESSION_METHOD;

                    text->compression_flag = 1;

                    text_offset = keyword_len + 2;
                }
                else if(!memcmp(chunk.type, type_itxt, 4))
                {
                    text->type = SPNG_ITXT;

                    /* at least two 1-byte fields, two >=0 length strings, and one byte of (compressed) text */
                    if((peek_bytes - keyword_len) < 5) return SPNG_EITXT;

                    text->compression_flag = keyword_nul[1];

                    if(text->compression_flag > 1) return SPNG_EITXT_COMPRESSION_FLAG;

                    if(keyword_nul[2]) return SPNG_EITXT_COMPRESSION_METHOD;

                    language_tag_offset = keyword_len + 3;

                    const unsigned char *term;
                    term = memchr(data + language_tag_offset, 0, peek_bytes - language_tag_offset);
                    if(term == NULL) return SPNG_EITXT_LANG_TAG;

                    if((peek_end - term) < 2) return SPNG_EITXT;

                    translated_keyword_offset = term - data + 1;

                    const unsigned char *zlib_stream = memchr(data + translated_keyword_offset, 0, peek_bytes - translated_keyword_offset);
                    if(zlib_stream == NULL) return SPNG_EITXT;
                    if(zlib_stream == peek_end) return SPNG_EITXT;

                    text_offset = zlib_stream - data + 1;
                    text->text_length = chunk.length - text_offset;
                }
                else return SPNG_EINTERNAL;


                if(text->compression_flag)
                {
                    /* cache usage = peek_bytes + decompressed text size + nul */
                    if(increase_cache_usage(ctx, peek_bytes)) return SPNG_ECHUNK_LIMITS;

                    text->keyword = spng__calloc(ctx, 1, peek_bytes);
                    if(text->keyword == NULL) return SPNG_EMEM;

                    memcpy(text->keyword, data, peek_bytes);

                    zlib_stream = ctx->data + text_offset;

                    ret = spng__inflate_stream(ctx, &text->text, &text->text_length, 1, zlib_stream, peek_bytes - text_offset);

                    if(ret) return ret;

                    text->text[text->text_length - 1] = '\0';
                    text->cache_usage = text->text_length + peek_bytes;
                }
                else
                {
                    if(increase_cache_usage(ctx, chunk.length + 1)) return SPNG_ECHUNK_LIMITS;

                    text->keyword = spng__malloc(ctx, chunk.length + 1);
                    if(text->keyword == NULL) return SPNG_EMEM;

                    memcpy(text->keyword, data, peek_bytes);

                    if(chunk.length > peek_bytes)
                    {
                        ret = read_chunk_bytes2(ctx, text->keyword + peek_bytes, chunk.length - peek_bytes);
                        if(ret) return ret;
                    }

                    text->text = text->keyword + text_offset;

                    text->text_length = chunk.length - text_offset;

                    text->text[text->text_length] = '\0';
                    text->cache_usage = chunk.length + 1;
                }

                if(check_png_keyword(text->keyword)) return SPNG_ETEXT_KEYWORD;

                text->text_length = strlen(text->text);

                if(text->type != SPNG_ITXT)
                {
                    language_tag_offset = keyword_len;
                    translated_keyword_offset = keyword_len;

                    if(ctx->strict && check_png_text(text->text, text->text_length))
                    {
                        if(text->type == SPNG_ZTXT) return SPNG_EZTXT;
                        else return SPNG_ETEXT;
                    }
                }

                text->language_tag = text->keyword + language_tag_offset;
                text->translated_keyword = text->keyword + translated_keyword_offset;

                ctx->stored.text = 1;
            }
            else if(!memcmp(chunk.type, type_splt, 4))
            {
                if(ctx->state == SPNG_STATE_AFTER_IDAT) return SPNG_ECHUNK_POS;
                if(ctx->user.splt) goto discard; /* XXX: could check profile names for uniqueness */
                if(!chunk.length) return SPNG_ECHUNK_SIZE;

                ctx->file.splt = 1;

                /* chunk.length + sizeof(struct spng_splt) + splt->n_entries * sizeof(struct spng_splt_entry) */
                if(increase_cache_usage(ctx, chunk.length + sizeof(struct spng_splt))) return SPNG_ECHUNK_LIMITS;

                ctx->n_splt++;
                if(ctx->n_splt < 1) return SPNG_EOVERFLOW;
                if(sizeof(struct spng_splt) > SIZE_MAX / ctx->n_splt) return SPNG_EOVERFLOW;

                void *buf = spng__realloc(ctx, ctx->splt_list, ctx->n_splt * sizeof(struct spng_splt));
                if(buf == NULL) return SPNG_EMEM;
                ctx->splt_list = buf;

                struct spng_splt *splt = &ctx->splt_list[ctx->n_splt - 1];

                memset(splt, 0, sizeof(struct spng_splt));

                ctx->undo = splt_undo;

                void *t = spng__malloc(ctx, chunk.length);
                if(t == NULL) return SPNG_EMEM;

                splt->entries = t; /* simplifies error handling */
                data = t;

                ret = read_chunk_bytes2(ctx, t, chunk.length);
                if(ret) return ret;

                uint32_t keyword_len = chunk.length < 80 ? chunk.length : 80;

                const unsigned char *keyword_nul = memchr(data, 0, keyword_len);
                if(keyword_nul == NULL) return SPNG_ESPLT_NAME;

                keyword_len = keyword_nul - data;

                memcpy(splt->name, data, keyword_len);

                if(check_png_keyword(splt->name)) return SPNG_ESPLT_NAME;

                uint32_t j;
                for(j=0; j < (ctx->n_splt - 1); j++)
                {
                    if(!strcmp(ctx->splt_list[j].name, splt->name)) return SPNG_ESPLT_DUP_NAME;
                }

                if( (chunk.length - keyword_len) <= 2) return SPNG_ECHUNK_SIZE;

                splt->sample_depth = data[keyword_len + 1];

                uint32_t entries_len = chunk.length - keyword_len - 2;
                if(!entries_len) return SPNG_ECHUNK_SIZE;

                if(splt->sample_depth == 16)
                {
                    if(entries_len % 10 != 0) return SPNG_ECHUNK_SIZE;
                    splt->n_entries = entries_len / 10;
                }
                else if(splt->sample_depth == 8)
                {
                    if(entries_len % 6 != 0) return SPNG_ECHUNK_SIZE;
                    splt->n_entries = entries_len / 6;
                }
                else return SPNG_ESPLT_DEPTH;

                if(!splt->n_entries) return SPNG_ECHUNK_SIZE;

                size_t list_size = splt->n_entries;

                if(list_size > SIZE_MAX / sizeof(struct spng_splt_entry)) return SPNG_EOVERFLOW;

                list_size *= sizeof(struct spng_splt_entry);

                if(increase_cache_usage(ctx, list_size)) return SPNG_ECHUNK_LIMITS;

                splt->entries = spng__malloc(ctx, list_size);
                if(splt->entries == NULL)
                {
                    spng__free(ctx, t);
                    return SPNG_EMEM;
                }

                data = (unsigned char*)t + keyword_len + 2;

                uint32_t k;
                if(splt->sample_depth == 16)
                {
                    for(k=0; k < splt->n_entries; k++)
                    {
                        splt->entries[k].red =   read_u16(data + k * 10);
                        splt->entries[k].green = read_u16(data + k * 10 + 2);
                        splt->entries[k].blue =  read_u16(data + k * 10 + 4);
                        splt->entries[k].alpha = read_u16(data + k * 10 + 6);
                        splt->entries[k].frequency = read_u16(data + k * 10 + 8);
                    }
                }
                else if(splt->sample_depth == 8)
                {
                    for(k=0; k < splt->n_entries; k++)
                    {
                        splt->entries[k].red =   data[k * 6];
                        splt->entries[k].green = data[k * 6 + 1];
                        splt->entries[k].blue =  data[k * 6 + 2];
                        splt->entries[k].alpha = data[k * 6 + 3];
                        splt->entries[k].frequency = read_u16(data + k * 6 + 4);
                    }
                }

                spng__free(ctx, t);
                decrease_cache_usage(ctx, chunk.length);

                ctx->stored.splt = 1;
            }

discard:
            ret = discard_chunk_bytes(ctx, ctx->cur_chunk_bytes_left);
            if(ret) return ret;
        }

    }

    return ret;
}

static int decode_err(spng_ctx *ctx, int err)
{
    ctx->state = SPNG_STATE_INVALID;

    return err;
}

/* Read chunks before or after the IDAT chunks depending on state */
static int read_chunks(spng_ctx *ctx, int only_ihdr)
{
    if(ctx == NULL) return SPNG_EINTERNAL;
    if(!ctx->state) return SPNG_EBADSTATE;
    if(ctx->data == NULL)
    {
        if(ctx->encode_only) return 0;
        else return SPNG_EINTERNAL;
    }

    int ret = 0;

    if(ctx->state == SPNG_STATE_INPUT)
    {
        ret = read_ihdr(ctx);

        if(ret) return decode_err(ctx, ret);

        ctx->state = SPNG_STATE_IHDR;
    }

    if(only_ihdr) return 0;

    if(ctx->state == SPNG_STATE_EOI)
    {
        ctx->state = SPNG_STATE_AFTER_IDAT;
        ctx->prev_was_idat = 1;
    }

    while(ctx->state < SPNG_STATE_FIRST_IDAT || ctx->state == SPNG_STATE_AFTER_IDAT)
    {
        ret = read_non_idat_chunks(ctx);

        if(!ret)
        {
            if(ctx->state < SPNG_STATE_FIRST_IDAT) ctx->state = SPNG_STATE_FIRST_IDAT;
            else if(ctx->state == SPNG_STATE_AFTER_IDAT) ctx->state = SPNG_STATE_IEND;
        }
        else
        {
            switch(ret)
            {
                case SPNG_ECHUNK_POS:
                case SPNG_ECHUNK_SIZE: /* size != expected size, SPNG_ECHUNK_STDLEN = invalid size */
                case SPNG_EDUP_PLTE:
                case SPNG_EDUP_CHRM:
                case SPNG_EDUP_GAMA:
                case SPNG_EDUP_ICCP:
                case SPNG_EDUP_SBIT:
                case SPNG_EDUP_SRGB:
                case SPNG_EDUP_BKGD:
                case SPNG_EDUP_HIST:
                case SPNG_EDUP_TRNS:
                case SPNG_EDUP_PHYS:
                case SPNG_EDUP_TIME:
                case SPNG_EDUP_OFFS:
                case SPNG_EDUP_EXIF:
                case SPNG_ECHRM:
                case SPNG_ETRNS_COLOR_TYPE:
                case SPNG_ETRNS_NO_PLTE:
                case SPNG_EGAMA:
                case SPNG_EICCP_NAME:
                case SPNG_EICCP_COMPRESSION_METHOD:
                case SPNG_ESBIT:
                case SPNG_ESRGB:
                case SPNG_ETEXT:
                case SPNG_ETEXT_KEYWORD:
                case SPNG_EZTXT:
                case SPNG_EZTXT_COMPRESSION_METHOD:
                case SPNG_EITXT:
                case SPNG_EITXT_COMPRESSION_FLAG:
                case SPNG_EITXT_COMPRESSION_METHOD:
                case SPNG_EITXT_LANG_TAG:
                case SPNG_EITXT_TRANSLATED_KEY:
                case SPNG_EBKGD_NO_PLTE:
                case SPNG_EBKGD_PLTE_IDX:
                case SPNG_EHIST_NO_PLTE:
                case SPNG_EPHYS:
                case SPNG_ESPLT_NAME:
                case SPNG_ESPLT_DUP_NAME:
                case SPNG_ESPLT_DEPTH:
                case SPNG_ETIME:
                case SPNG_EOFFS:
                case SPNG_EEXIF:
                case SPNG_EZLIB:
                {
                    if(!ctx->strict && !is_critical_chunk(&ctx->current_chunk))
                    {
                        ret = discard_chunk_bytes(ctx, ctx->cur_chunk_bytes_left);
                        if(ret) return decode_err(ctx, ret);

                        if(ctx->undo) ctx->undo(ctx);

                        ctx->stored = ctx->prev_stored;

                        ctx->discard = 0;
                        ctx->undo = NULL;

                        continue;
                    }
                    else return decode_err(ctx, ret);

                    break;
                }
                default: return decode_err(ctx, ret);
            }
        }
    }

    return ret;
}

int spng_decode_scanline(spng_ctx *ctx, void *out, size_t len)
{
    if(ctx == NULL || out == NULL) return 1;

    if(ctx->state >= SPNG_STATE_EOI) return SPNG_EOI;

    struct decode_flags f = ctx->decode_flags;

    int ret;
    int fmt = ctx->fmt;

    struct spng_row_info *ri = &ctx->row_info;
    const struct spng_subimage *sub = ctx->subimage;

    const struct spng_ihdr *ihdr = &ctx->ihdr;
    const uint16_t *gamma_lut = ctx->gamma_lut;
    unsigned char *trns_px = ctx->trns_px;
    const struct spng_sbit *sb = &ctx->decode_sb;
    const struct spng_plte_entry16 *plte = ctx->decode_plte;
    struct spng__iter iter = (ihdr->bit_depth < 16) ? spng__iter_init(ihdr->bit_depth, ctx->scanline) : (struct spng__iter){0};

    const unsigned char *scanline = ctx->scanline;

    int pass = ri->pass;
    uint8_t next_filter = 0;
    size_t scanline_width = sub[pass].scanline_width;
    uint32_t k;
    uint32_t scanline_idx = ri->scanline_idx;
    uint32_t width = sub[pass].width;
    uint8_t r_8, g_8, b_8, a_8, gray_8;
    uint16_t r_16, g_16, b_16, a_16, gray_16;
    r_8=0; g_8=0; b_8=0; a_8=0; gray_8=0;
    r_16=0; g_16=0; b_16=0; a_16=0; gray_16=0;
    size_t pixel_size = 4; /* SPNG_FMT_RGBA8 */
    size_t pixel_offset = 0;
    unsigned char *pixel;
    unsigned processing_depth = ihdr->bit_depth;

    if(f.indexed) processing_depth = 8;

    if(fmt == SPNG_FMT_RGBA16) pixel_size = 8;
    else if(fmt == SPNG_FMT_RGB8) pixel_size = 3;

    if(len < sub[pass].out_width) return SPNG_EBUFSIZ;

    if(scanline_idx == (sub[pass].height - 1) && ri->pass == ctx->last_pass)
    {
        ret = read_scanline_bytes(ctx, ctx->scanline, scanline_width - 1);
    }
    else
    {
        ret = read_scanline_bytes(ctx, ctx->scanline, scanline_width);
        if(ret) return decode_err(ctx, ret);

        next_filter = ctx->scanline[scanline_width - 1];
        if(next_filter > 4) ret = SPNG_EFILTER;
    }

    if(ret) return decode_err(ctx, ret);

    if(!scanline_idx && ri->filter > 1)
    {
        /* prev_scanline is all zeros for the first scanline */
        memset(ctx->prev_scanline, 0, scanline_width);
    }

    if(ihdr->bit_depth == 16 && fmt != SPNG_FMT_RAW) u16_row_to_host(ctx->scanline, scanline_width - 1);

    ret = defilter_scanline(ctx->prev_scanline, ctx->scanline, scanline_width, ctx->bytes_per_pixel, ri->filter);
    if(ret) return decode_err(ctx, ret);

    ri->filter = next_filter;

    for(k=0; k < width; k++)
    {
        pixel = (unsigned char*)out + pixel_offset;
        pixel_offset += pixel_size;

        if(f.same_layout)
        {
            if(f.zerocopy) break;

            memcpy(out, scanline, scanline_width - 1);
            break;
        }

        if(f.unpack)
        {
            unpack_scanline(out, scanline, width, ihdr->bit_depth, fmt);
            break;
        }

        if(ihdr->color_type == SPNG_COLOR_TYPE_TRUECOLOR)
        {
            if(ihdr->bit_depth == 16)
            {
                memcpy(&r_16, scanline + (k * 6), 2);
                memcpy(&g_16, scanline + (k * 6) + 2, 2);
                memcpy(&b_16, scanline + (k * 6) + 4, 2);

                a_16 = 65535;
            }
            else /* == 8 */
            {
                if(fmt == SPNG_FMT_RGBA8)
                {
                    rgb8_row_to_rgba8(scanline, out, width);
                    break;
                }

                r_8 = scanline[k * 3];
                g_8 = scanline[k * 3 + 1];
                b_8 = scanline[k * 3 + 2];

                a_8 = 255;
            }
        }
        else if(ihdr->color_type == SPNG_COLOR_TYPE_INDEXED)
        {
            uint8_t entry = 0;

            if(ihdr->bit_depth == 8)
            {
                if(fmt & (SPNG_FMT_RGBA8 | SPNG_FMT_RGB8))
                {
                    expand_row(out, scanline, plte, width, fmt);
                    break;
                }

                entry = scanline[k];
            }
            else /* < 8 */
            {
                entry = get_sample(&iter);
            }

            if(fmt & (SPNG_FMT_RGBA8 | SPNG_FMT_RGB8))
            {
                pixel[0] = plte[entry].red;
                pixel[1] = plte[entry].green;
                pixel[2] = plte[entry].blue;
                if(fmt == SPNG_FMT_RGBA8) pixel[3] = plte[entry].alpha;

                continue;
            }
            else /* RGBA16 */
            {
                r_16 = plte[entry].red;
                g_16 = plte[entry].green;
                b_16 = plte[entry].blue;
                a_16 = plte[entry].alpha;

                memcpy(pixel, &r_16, 2);
                memcpy(pixel + 2, &g_16, 2);
                memcpy(pixel + 4, &b_16, 2);
                memcpy(pixel + 6, &a_16, 2);

                continue;
            }
        }
        else if(ihdr->color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA)
        {
            if(ihdr->bit_depth == 16)
            {
                memcpy(&r_16, scanline + (k * 8), 2);
                memcpy(&g_16, scanline + (k * 8) + 2, 2);
                memcpy(&b_16, scanline + (k * 8) + 4, 2);
                memcpy(&a_16, scanline + (k * 8) + 6, 2);
            }
            else /* == 8 */
            {
                r_8 = scanline[k * 4];
                g_8 = scanline[k * 4 + 1];
                b_8 = scanline[k * 4 + 2];
                a_8 = scanline[k * 4 + 3];
            }
        }
        else if(ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE)
        {
            if(ihdr->bit_depth == 16)
            {
                memcpy(&gray_16, scanline + k * 2, 2);

                if(f.apply_trns && ctx->trns.gray == gray_16) a_16 = 0;
                else a_16 = 65535;

                r_16 = gray_16;
                g_16 = gray_16;
                b_16 = gray_16;
            }
            else /* <= 8 */
            {
                gray_8 = get_sample(&iter);

                if(f.apply_trns && ctx->trns.gray == gray_8) a_8 = 0;
                else a_8 = 255;

                r_8 = gray_8; g_8 = gray_8; b_8 = gray_8;
            }
        }
        else if(ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA)
        {
            if(ihdr->bit_depth == 16)
            {
                memcpy(&gray_16, scanline + (k * 4), 2);
                memcpy(&a_16, scanline + (k * 4) + 2, 2);

                r_16 = gray_16;
                g_16 = gray_16;
                b_16 = gray_16;
            }
            else /* == 8 */
            {
                gray_8 = scanline[k * 2];
                a_8 = scanline[k * 2 + 1];

                r_8 = gray_8;
                g_8 = gray_8;
                b_8 = gray_8;
            }
        }


        if(fmt & (SPNG_FMT_RGBA8 | SPNG_FMT_RGB8))
        {
            if(ihdr->bit_depth == 16)
            {
                r_8 = r_16 >> 8;
                g_8 = g_16 >> 8;
                b_8 = b_16 >> 8;
                a_8 = a_16 >> 8;
            }

            pixel[0] = r_8;
            pixel[1] = g_8;
            pixel[2] = b_8;

            if(fmt == SPNG_FMT_RGBA8) pixel[3] = a_8;
        }
        else if(fmt == SPNG_FMT_RGBA16)
        {
            if(ihdr->bit_depth != 16)
            {
                r_16 = r_8;
                g_16 = g_8;
                b_16 = b_8;
                a_16 = a_8;
            }

            memcpy(pixel, &r_16, 2);
            memcpy(pixel + 2, &g_16, 2);
            memcpy(pixel + 4, &b_16, 2);
            memcpy(pixel + 6, &a_16, 2);
        }
    }/* for(k=0; k < width; k++) */

    if(f.apply_trns) trns_row(out, scanline, trns_px, ctx->bytes_per_pixel, &ctx->ihdr, width, fmt);

    if(f.do_scaling) scale_row(out, width, fmt, processing_depth, sb);

    if(f.apply_gamma) gamma_correct_row(out, width, fmt, gamma_lut);

    /* The previous scanline is always defiltered */
    void *t = ctx->prev_scanline;
    ctx->prev_scanline = ctx->scanline;
    ctx->scanline = t;

    if(ri->scanline_idx == (sub[pass].height - 1)) /* Last scanline */
    {
        if(ri->pass == ctx->last_pass)
        {
            ctx->state = SPNG_STATE_EOI;

            if(ctx->cur_chunk_bytes_left) /* zlib stream ended before an IDAT chunk boundary */
            {/* Discard the rest of the chunk */
                int ret = discard_chunk_bytes(ctx, ctx->cur_chunk_bytes_left);
                if(ret) return decode_err(ctx, ret);
            }

            ctx->last_idat = ctx->current_chunk;

            return SPNG_EOI;
        }

        ri->scanline_idx = 0;
        ri->pass++;

        /* Skip empty passes */
        while( (!sub[ri->pass].width || !sub[ri->pass].height) && (ri->pass < ctx->last_pass)) ri->pass++;
    }
    else
    {
        ri->row_num++;
        ri->scanline_idx++;
    }

    if(f.interlaced) ri->row_num = adam7_y_start[ri->pass] + ri->scanline_idx * adam7_y_delta[ri->pass];

    return 0;
}

int spng_decode_row(spng_ctx *ctx, void *out, size_t len)
{
    if(ctx == NULL || out == NULL) return 1;
    if(ctx->state >= SPNG_STATE_EOI) return SPNG_EOI;
    if(len < ctx->out_width) return SPNG_EBUFSIZ;

    const struct spng_ihdr *ihdr = &ctx->ihdr;
    int ret, pass = ctx->row_info.pass;
    unsigned char *outptr = out;

    if(!ihdr->interlace_method || pass == 6) return spng_decode_scanline(ctx, out, len);

    ret = spng_decode_scanline(ctx, ctx->row, ctx->out_width);
    if(ret && ret != SPNG_EOI) return ret;

    uint32_t k;
    unsigned pixel_size = 4; /* RGBA8 */
    if(ctx->fmt == SPNG_FMT_RGBA16) pixel_size = 8;
    else if(ctx->fmt == SPNG_FMT_RGB8) pixel_size = 3;
    else if(ctx->fmt == SPNG_FMT_G8) pixel_size = 1;
    else if(ctx->fmt == SPNG_FMT_GA8) pixel_size = 2;
    else if(ctx->fmt & (SPNG_FMT_PNG | SPNG_FMT_RAW))
    {
        if(ihdr->bit_depth < 8)
        {
            struct spng__iter iter = spng__iter_init(ihdr->bit_depth, ctx->row);
            const uint8_t samples_per_byte = 8 / ihdr->bit_depth;
            uint8_t sample;

            for(k=0; k < ctx->subimage[pass].width; k++)
            {
                sample = get_sample(&iter);

                size_t ioffset = adam7_x_start[pass] + k * adam7_x_delta[pass];

                sample = sample << (iter.initial_shift - ioffset * ihdr->bit_depth % 8);

                ioffset /= samples_per_byte;

                outptr[ioffset] |= sample;
            }

            return 0;
        }
        else pixel_size = ctx->bytes_per_pixel;
    }

    for(k=0; k < ctx->subimage[pass].width; k++)
    {
        size_t ioffset = (adam7_x_start[pass] + (size_t) k * adam7_x_delta[pass]) * pixel_size;

        memcpy(outptr + ioffset, ctx->row + k * pixel_size, pixel_size);
    }

    return 0;
}

int spng_decode_image(spng_ctx *ctx, void *out, size_t len, int fmt, int flags)
{
    if(ctx == NULL) return 1;
    if(ctx->state >= SPNG_STATE_EOI) return SPNG_EOI;

    int ret = spng_decoded_image_size(ctx, fmt, &ctx->total_out_size);
    if(ret) return decode_err(ctx, ret);

    ret = read_chunks(ctx, 0);
    if(ret) return ret;

    if( !(flags & SPNG_DECODE_PROGRESSIVE) )
    {
        if(out == NULL) return 1;
        if(len < ctx->total_out_size) return SPNG_EBUFSIZ;
    }

    struct spng_ihdr *ihdr = &ctx->ihdr;

    ctx->out_width = ctx->total_out_size / ihdr->height;

    ret = spng__inflate_init(ctx);
    if(ret) return decode_err(ctx, ret);

    ctx->zstream.avail_in = 0;
    ctx->zstream.next_in = ctx->data;

    ctx->scanline_buf = spng__malloc(ctx, ctx->subimage[ctx->widest_pass].scanline_width);
    ctx->prev_scanline_buf = spng__malloc(ctx, ctx->subimage[ctx->widest_pass].scanline_width);
    ctx->scanline = ctx->scanline_buf;
    ctx->prev_scanline = ctx->prev_scanline_buf;

    struct decode_flags f = {0};

    ctx->fmt = fmt;

    if(ihdr->color_type == SPNG_COLOR_TYPE_INDEXED) f.indexed = 1;

    unsigned processing_depth = ihdr->bit_depth;

    if(f.indexed) processing_depth = 8;

    if(ihdr->interlace_method)
    {
        f.interlaced = 1;
        ctx->row_buf = spng__malloc(ctx, ctx->out_width);
        ctx->row = ctx->row_buf;

        if(ctx->row == NULL) return decode_err(ctx, SPNG_EMEM);
    }

    if(ctx->scanline == NULL || ctx->prev_scanline == NULL)
    {
        return decode_err(ctx, SPNG_EMEM);
    }

    f.do_scaling = 1;
    if(f.indexed) f.do_scaling = 0;

    unsigned depth_target = 8; /* FMT_RGBA8, G8 */
    if(fmt == SPNG_FMT_RGBA16) depth_target = 16;

    if(flags & SPNG_DECODE_TRNS && ctx->stored.trns) f.apply_trns = 1;
    else flags &= ~SPNG_DECODE_TRNS;

    if(ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA ||
       ihdr->color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA) flags &= ~SPNG_DECODE_TRNS;

    if(flags & SPNG_DECODE_GAMMA && ctx->stored.gama) f.apply_gamma = 1;
    else flags &= ~SPNG_DECODE_GAMMA;

    if(flags & SPNG_DECODE_USE_SBIT && ctx->stored.sbit) f.use_sbit = 1;
    else flags &= ~SPNG_DECODE_USE_SBIT;

    if(fmt & (SPNG_FMT_RGBA8 | SPNG_FMT_RGBA16))
    {
        if(ihdr->color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA &&
           ihdr->bit_depth == depth_target) f.same_layout = 1;
    }
    else if(fmt == SPNG_FMT_RGB8)
    {
        if(ihdr->color_type == SPNG_COLOR_TYPE_TRUECOLOR &&
           ihdr->bit_depth == depth_target) f.same_layout = 1;

        f.apply_trns = 0; /* not applicable */
    }
    else if(fmt & (SPNG_FMT_PNG | SPNG_FMT_RAW))
    {
        f.same_layout = 1;
        f.do_scaling = 0;
        f.apply_gamma = 0; /* for now */
        f.apply_trns = 0;
    }
    else if(fmt == SPNG_FMT_G8 && ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE && ihdr->bit_depth <= 8)
    {
        if(ihdr->bit_depth == depth_target) f.same_layout = 1;
        else if(ihdr->bit_depth < 8) f.unpack = 1;

        f.apply_trns = 0;
    }
    else if(fmt == SPNG_FMT_GA8 && ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE && ihdr->bit_depth <= 8)
    {
        if(ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA &&
           ihdr->bit_depth == depth_target) f.same_layout = 1;
        else if(ihdr->bit_depth <= 8) f.unpack = 1;
    }
    else if(fmt == SPNG_FMT_GA16 && ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE && ihdr->bit_depth == 16)
    {
        if(ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA &&
           ihdr->bit_depth == depth_target) f.same_layout = 1;
        else if(ihdr->bit_depth == 16) f.unpack = 1;
    }

    /*if(f.same_layout && !flags && !f.interlaced) f.zerocopy = 1;*/

    uint16_t *gamma_lut = NULL;

    if(f.apply_gamma)
    {
        float file_gamma = (float)ctx->gama / 100000.0f;
        float max;

        unsigned lut_entries;

        if(fmt & (SPNG_FMT_RGBA8 | SPNG_FMT_RGB8))
        {
            lut_entries = 256;
            max = 255.0f;

            gamma_lut = ctx->gamma_lut8;
            ctx->gamma_lut = ctx->gamma_lut8;
        }
        else /* SPNG_FMT_RGBA16 */
        {
            lut_entries = 65536;
            max = 65535.0f;

            ctx->gamma_lut16 = spng__malloc(ctx, lut_entries * sizeof(uint16_t));
            if(ctx->gamma_lut16 == NULL) return decode_err(ctx, SPNG_EMEM);

            gamma_lut = ctx->gamma_lut16;
            ctx->gamma_lut = ctx->gamma_lut16;
        }

        float screen_gamma = 2.2f;
        float exponent = file_gamma * screen_gamma;

        if(FP_ZERO == fpclassify(exponent)) return decode_err(ctx, SPNG_EGAMA);

        exponent = 1.0f / exponent;

        unsigned i;
        for(i=0; i < lut_entries; i++)
        {
            float c = pow((float)i / max, exponent) * max;
            if(c > max) c = max;

            gamma_lut[i] = (uint16_t)c;
        }
    }

    struct spng_sbit *sb = &ctx->decode_sb;

    sb->red_bits = processing_depth;
    sb->green_bits = processing_depth;
    sb->blue_bits = processing_depth;
    sb->alpha_bits = processing_depth;
    sb->grayscale_bits = processing_depth;

    if(f.use_sbit)
    {
        if(ihdr->color_type == 0)
        {
            sb->grayscale_bits = ctx->sbit.grayscale_bits;
            sb->alpha_bits = ihdr->bit_depth;
        }
        else if(ihdr->color_type == 2 || ihdr->color_type == 3)
        {
            sb->red_bits = ctx->sbit.red_bits;
            sb->green_bits = ctx->sbit.green_bits;
            sb->blue_bits = ctx->sbit.blue_bits;
            sb->alpha_bits = ihdr->bit_depth;
        }
        else if(ihdr->color_type == 4)
        {
            sb->grayscale_bits = ctx->sbit.grayscale_bits;
            sb->alpha_bits = ctx->sbit.alpha_bits;
        }
        else /* == 6 */
        {
            sb->red_bits = ctx->sbit.red_bits;
            sb->green_bits = ctx->sbit.green_bits;
            sb->blue_bits = ctx->sbit.blue_bits;
            sb->alpha_bits = ctx->sbit.alpha_bits;
        }
    }

    if(ihdr->bit_depth == 16 && fmt & (SPNG_FMT_RGBA8 | SPNG_FMT_RGB8))
    {/* samples are scaled down by 8 bits in the decode loop */
        sb->red_bits -= 8;
        sb->green_bits -= 8;
        sb->blue_bits -= 8;
        sb->alpha_bits -= 8;
        sb->grayscale_bits -= 8;

        processing_depth = 8;
    }

    /* Prevent infinite loops in sample_to_target() */
    if(!depth_target || depth_target > 16 ||
       !processing_depth || processing_depth > 16 ||
       !sb->grayscale_bits || sb->grayscale_bits > processing_depth ||
       !sb->alpha_bits || sb->alpha_bits > processing_depth ||
       !sb->red_bits || sb->red_bits > processing_depth ||
       !sb->green_bits || sb->green_bits > processing_depth ||
       !sb->blue_bits || sb->blue_bits > processing_depth)
    {
        return decode_err(ctx, SPNG_ESBIT);
    }

    if(sb->red_bits == sb->green_bits &&
       sb->green_bits == sb->blue_bits &&
       sb->blue_bits == sb->alpha_bits &&
       sb->alpha_bits == processing_depth &&
       processing_depth == depth_target) f.do_scaling = 0;

    struct spng_plte_entry16 *plte = ctx->decode_plte;

    /* Pre-process palette entries */
    if(f.indexed)
    {
        uint32_t i;
        for(i=0; i < 256; i++)
        {
            if(f.apply_trns && i < ctx->trns.n_type3_entries)
                ctx->plte.entries[i].alpha = ctx->trns.type3_alpha[i];
            else
                ctx->plte.entries[i].alpha = 255;

            plte[i].red = sample_to_target(ctx->plte.entries[i].red, 8, sb->red_bits, depth_target);
            plte[i].green = sample_to_target(ctx->plte.entries[i].green, 8, sb->green_bits, depth_target);
            plte[i].blue = sample_to_target(ctx->plte.entries[i].blue, 8, sb->blue_bits, depth_target);
            plte[i].alpha = sample_to_target(ctx->plte.entries[i].alpha, 8, sb->alpha_bits, depth_target);

            if(f.apply_gamma)
            {
                plte[i].red = gamma_lut[plte[i].red];
                plte[i].green = gamma_lut[plte[i].green];
                plte[i].blue = gamma_lut[plte[i].blue];
            }
        }

        f.apply_trns = 0;
        f.apply_gamma = 0;
    }

    unsigned char *trns_px = ctx->trns_px;

    if(f.apply_trns)
    {
        if(fmt & (SPNG_FMT_RGBA8 | SPNG_FMT_RGBA16))
        {
            if(ihdr->color_type == SPNG_COLOR_TYPE_TRUECOLOR)
            {
                if(ihdr->bit_depth == 16)
                {
                    memcpy(trns_px, &ctx->trns.red, 2);
                    memcpy(trns_px + 2, &ctx->trns.green, 2);
                    memcpy(trns_px + 4, &ctx->trns.blue, 2);
                }
                else
                {
                    trns_px[0] = ctx->trns.red;
                    trns_px[1] = ctx->trns.green;
                    trns_px[2] = ctx->trns.blue;
                }
            }
        }
        else if(ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE) // fmt == SPNG_FMT_GA8 &&
        {
            if(ihdr->bit_depth == 16)
            {
                memcpy(trns_px, &ctx->trns.gray, 2);
            }
            else
            {
                trns_px[0] = ctx->trns.gray;
            }
        }
    }

    ctx->decode_flags = f;

    ctx->state = SPNG_STATE_DECODE_INIT;

    struct spng_row_info *ri = &ctx->row_info;
    struct spng_subimage *sub = ctx->subimage;

    while(!sub[ri->pass].width || !sub[ri->pass].height) ri->pass++;

    if(f.interlaced) ri->row_num = adam7_y_start[ri->pass];

    unsigned pixel_size = 4; /* SPNG_FMT_RGBA8 */

    if(fmt == SPNG_FMT_RGBA16) pixel_size = 8;
    else if(fmt == SPNG_FMT_RGB8) pixel_size = 3;
    else if(fmt == SPNG_FMT_G8) pixel_size = 1;
    else if(fmt == SPNG_FMT_GA8) pixel_size = 2;

    int i;
    for(i=ri->pass; i <= ctx->last_pass; i++)
    {
        if(!sub[i].scanline_width) continue;

        if(fmt & (SPNG_FMT_PNG | SPNG_FMT_RAW)) sub[i].out_width = sub[i].scanline_width - 1;
        else sub[i].out_width = (size_t)sub[i].width * pixel_size;

        if(sub[i].out_width > UINT32_MAX) return decode_err(ctx, SPNG_EOVERFLOW);
    }

    /* Read the first filter byte, offsetting all reads by 1 byte.
    The scanlines will be aligned with the start of the array with
    the next scanline's filter byte at the end,
    the last scanline will end up being 1 byte "shorter". */
    ret = read_scanline_bytes(ctx, &ri->filter, 1);
    if(ret) return decode_err(ctx, ret);

    if(ri->filter > 4) return decode_err(ctx, SPNG_EFILTER);

    if(flags & SPNG_DECODE_PROGRESSIVE)
    {
        return 0;
    }

    do
    {
        size_t ioffset = ri->row_num * ctx->out_width;

        ret = spng_decode_row(ctx, (unsigned char*)out + ioffset, ctx->out_width);
    }while(!ret);

    if(ret != SPNG_EOI) return decode_err(ctx, ret);

    return 0;
}

int spng_get_row_info(spng_ctx *ctx, struct spng_row_info *row_info)
{
    if(ctx == NULL || row_info == NULL || ctx->state < SPNG_STATE_DECODE_INIT) return 1;

    if(ctx->state >= SPNG_STATE_EOI) return SPNG_EOI;

    *row_info = ctx->row_info;

    return 0;
}

spng_ctx *spng_ctx_new(int flags)
{
    struct spng_alloc alloc = {0};

    alloc.malloc_fn = malloc;
    alloc.realloc_fn = realloc;
    alloc.calloc_fn = calloc;
    alloc.free_fn = free;

    return spng_ctx_new2(&alloc, flags);
}

spng_ctx *spng_ctx_new2(struct spng_alloc *alloc, int flags)
{
    if(alloc == NULL) return NULL;
    if(flags != (flags & SPNG_CTX_IGNORE_ADLER32)) return NULL;

    if(alloc->malloc_fn == NULL) return NULL;
    if(alloc->realloc_fn == NULL) return NULL;
    if(alloc->calloc_fn == NULL) return NULL;
    if(alloc->free_fn == NULL) return NULL;

    spng_ctx *ctx = alloc->calloc_fn(1, sizeof(spng_ctx));
    if(ctx == NULL) return NULL;

    ctx->alloc = *alloc;

    ctx->max_chunk_size = png_u32max;
    ctx->chunk_cache_limit = SIZE_MAX;

    ctx->state = SPNG_STATE_INIT;

    ctx->crc_action_critical = SPNG_CRC_ERROR;
    ctx->crc_action_ancillary = SPNG_CRC_DISCARD;

    ctx->flags = flags;

    return ctx;
}

void spng_ctx_free(spng_ctx *ctx)
{
    if(ctx == NULL) return;

    if(ctx->streaming && ctx->stream_buf != NULL) spng__free(ctx, ctx->stream_buf);

    if(!ctx->user.exif) spng__free(ctx, ctx->exif.data);

    if(!ctx->user.iccp) spng__free(ctx, ctx->iccp.profile);

    uint32_t i;

    if(ctx->splt_list != NULL && !ctx->user.splt)
    {
        for(i=0; i < ctx->n_splt; i++)
        {
            spng__free(ctx, ctx->splt_list[i].entries);
        }
        spng__free(ctx, ctx->splt_list);
    }

    if(ctx->text_list != NULL && !ctx->user.text)
    {
        for(i=0; i< ctx->n_text; i++)
        {
            spng__free(ctx, ctx->text_list[i].keyword);
            if(ctx->text_list[i].compression_flag) spng__free(ctx, ctx->text_list[i].text);
        }
        spng__free(ctx, ctx->text_list);
    }

    inflateEnd(&ctx->zstream);

    spng__free(ctx, ctx->gamma_lut16);

    spng__free(ctx, ctx->row_buf);
    spng__free(ctx, ctx->scanline_buf);
    spng__free(ctx, ctx->prev_scanline_buf);

    spng_free_fn *free_func = ctx->alloc.free_fn;

    memset(ctx, 0, sizeof(spng_ctx));

    free_func(ctx);
}

static int buffer_read_fn(spng_ctx *ctx, void *user, void *data, size_t n)
{
    if(n > ctx->bytes_left) return SPNG_IO_EOF;

    (void)user;
    (void)data;
    ctx->data = ctx->data + ctx->last_read_size;

    ctx->last_read_size = n;
    ctx->bytes_left -= n;

    return 0;
}

static int file_read_fn(spng_ctx *ctx, void *user, void *data, size_t n)
{
    FILE *file = user;
    (void)ctx;

    if(fread(data, n, 1, file) != 1)
    {
        if(feof(file)) return SPNG_IO_EOF;
        else return SPNG_IO_ERROR;
    }

    return 0;
}

int spng_set_png_buffer(spng_ctx *ctx, const void *buf, size_t size)
{
    if(ctx == NULL || buf == NULL) return 1;
    if(!ctx->state) return SPNG_EBADSTATE;
    if(ctx->encode_only) return SPNG_ENCODE_ONLY;

    if(ctx->data != NULL) return SPNG_EBUF_SET;

    ctx->data = buf;
    ctx->png_buf = buf;
    ctx->data_size = size;
    ctx->bytes_left = size;

    ctx->read_fn = buffer_read_fn;

    ctx->state = SPNG_STATE_INPUT;

    return 0;
}

int spng_set_png_stream(spng_ctx *ctx, spng_read_fn *read_func, void *user)
{
    if(ctx == NULL || read_func == NULL) return 1;
    if(!ctx->state) return SPNG_EBADSTATE;
    if(ctx->encode_only) return SPNG_ENCODE_ONLY;

    if(ctx->stream_buf != NULL) return SPNG_EBUF_SET;

    ctx->stream_buf = spng__malloc(ctx, SPNG_READ_SIZE);
    if(ctx->stream_buf == NULL) return SPNG_EMEM;

    ctx->data = ctx->stream_buf;
    ctx->data_size = SPNG_READ_SIZE;

    ctx->read_fn = read_func;
    ctx->read_user_ptr = user;

    ctx->streaming = 1;

    ctx->state = SPNG_STATE_INPUT;

    return 0;
}

int spng_set_png_file(spng_ctx *ctx, FILE *file)
{
    if(file == NULL) return 1;

    return spng_set_png_stream(ctx, file_read_fn, file);
}

int spng_set_image_limits(spng_ctx *ctx, uint32_t width, uint32_t height)
{
    if(ctx == NULL) return 1;

    if(width > png_u32max || height > png_u32max) return 1;

    ctx->max_width = width;
    ctx->max_height = height;

    return 0;
}

int spng_get_image_limits(spng_ctx *ctx, uint32_t *width, uint32_t *height)
{
    if(ctx == NULL || width == NULL || height == NULL) return 1;

    *width = ctx->max_width;
    *height = ctx->max_height;

    return 0;
}

int spng_set_chunk_limits(spng_ctx *ctx, size_t chunk_size, size_t cache_limit)
{
    if(ctx == NULL || chunk_size > png_u32max || chunk_size > cache_limit) return 1;

    ctx->max_chunk_size = chunk_size;

    ctx->chunk_cache_limit = cache_limit;

    return 0;
}

int spng_get_chunk_limits(spng_ctx *ctx, size_t *chunk_size, size_t *cache_limit)
{
    if(ctx == NULL || chunk_size == NULL || cache_limit == NULL) return 1;

    *chunk_size = ctx->max_chunk_size;

    *cache_limit = ctx->chunk_cache_limit;

    return 0;
}

int spng_set_crc_action(spng_ctx *ctx, int critical, int ancillary)
{
    if(ctx == NULL) return 1;

    if(critical > 2 || critical < 0) return 1;
    if(ancillary > 2 || ancillary < 0) return 1;

    if(critical == SPNG_CRC_DISCARD) return 1;

    ctx->crc_action_critical = critical;
    ctx->crc_action_ancillary = ancillary;

    return 0;
}

int spng_decoded_image_size(spng_ctx *ctx, int fmt, size_t *len)
{
    if(ctx == NULL || len == NULL) return 1;

    int ret = read_chunks(ctx, 1);
    if(ret) return ret;

    struct spng_ihdr *ihdr = &ctx->ihdr;
    size_t res = ihdr->width;
    unsigned bytes_per_pixel;

    /* Currently all enums are single-bit values */
    if(fmt & ((unsigned)fmt - 1)) return SPNG_EFMT; /* More than one bit is set */

    if(fmt == SPNG_FMT_RGBA8)
    {
        bytes_per_pixel = 4;
    }
    else if(fmt == SPNG_FMT_RGBA16)
    {
        bytes_per_pixel = 8;
    }
    else if(fmt == SPNG_FMT_RGB8)
    {
        bytes_per_pixel = 3;
    }
    else if(fmt == SPNG_FMT_PNG || fmt == SPNG_FMT_RAW)
    {
        ret = calculate_scanline_width(ctx, ihdr->width, &res);
        if(ret) return ret;

        res -= 1; /* exclude filter byte */
        bytes_per_pixel = 1;
    }
    else if(fmt == SPNG_FMT_G8 && ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE && ihdr->bit_depth <= 8)
    {
        bytes_per_pixel = 1;
    }
    else if(fmt == SPNG_FMT_GA8 && ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE && ihdr->bit_depth <= 8)
    {
        bytes_per_pixel = 2;
    }
    else if(fmt == SPNG_FMT_GA16 && ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE && ihdr->bit_depth == 16)
    {
        bytes_per_pixel = 4;
    }
    else return SPNG_EFMT;

    if(res > SIZE_MAX / bytes_per_pixel) return SPNG_EOVERFLOW;
    res = res * bytes_per_pixel;

    if(res > SIZE_MAX / ihdr->height) return SPNG_EOVERFLOW;
    res = res * ihdr->height;

    *len = res;

    return 0;
}

int spng_get_ihdr(spng_ctx *ctx, struct spng_ihdr *ihdr)
{
    if(ctx == NULL) return 1;
    int ret = read_chunks(ctx, 1);
    if(ret) return ret;
    if(ihdr == NULL) return 1;

    *ihdr = ctx->ihdr;

    return 0;
}

int spng_get_plte(spng_ctx *ctx, struct spng_plte *plte)
{
    SPNG_GET_CHUNK_BOILERPLATE(plte);

    *plte = ctx->plte;

    return 0;
}

int spng_get_trns(spng_ctx *ctx, struct spng_trns *trns)
{
    SPNG_GET_CHUNK_BOILERPLATE(trns);

    *trns = ctx->trns;

    return 0;
}

int spng_get_chrm(spng_ctx *ctx, struct spng_chrm *chrm)
{
    SPNG_GET_CHUNK_BOILERPLATE(chrm);

    chrm->white_point_x = (double)ctx->chrm_int.white_point_x / 100000.0;
    chrm->white_point_y = (double)ctx->chrm_int.white_point_y / 100000.0;
    chrm->red_x = (double)ctx->chrm_int.red_x / 100000.0;
    chrm->red_y = (double)ctx->chrm_int.red_y / 100000.0;
    chrm->blue_y = (double)ctx->chrm_int.blue_y / 100000.0;
    chrm->blue_x = (double)ctx->chrm_int.blue_x / 100000.0;
    chrm->green_x = (double)ctx->chrm_int.green_x / 100000.0;
    chrm->green_y = (double)ctx->chrm_int.green_y / 100000.0;

    return 0;
}

int spng_get_chrm_int(spng_ctx *ctx, struct spng_chrm_int *chrm)
{
    SPNG_GET_CHUNK_BOILERPLATE(chrm);

    *chrm = ctx->chrm_int;

    return 0;
}

int spng_get_gama(spng_ctx *ctx, double *gamma)
{
    double *gama = gamma;
    SPNG_GET_CHUNK_BOILERPLATE(gama);

    *gama = (double)ctx->gama / 100000.0;

    return 0;
}

int spng_get_iccp(spng_ctx *ctx, struct spng_iccp *iccp)
{
    SPNG_GET_CHUNK_BOILERPLATE(iccp);

    *iccp = ctx->iccp;

    return 0;
}

int spng_get_sbit(spng_ctx *ctx, struct spng_sbit *sbit)
{
    SPNG_GET_CHUNK_BOILERPLATE(sbit);

    *sbit = ctx->sbit;

    return 0;
}

int spng_get_srgb(spng_ctx *ctx, uint8_t *rendering_intent)
{
    uint8_t *srgb = rendering_intent;
    SPNG_GET_CHUNK_BOILERPLATE(srgb);

    *srgb = ctx->srgb_rendering_intent;

    return 0;
}

int spng_get_text(spng_ctx *ctx, struct spng_text *text, uint32_t *n_text)
{
    if(ctx == NULL) return 1;
    int ret = read_chunks(ctx, 0);
    if(ret) return ret;
    if(!ctx->stored.text) return SPNG_ECHUNKAVAIL;
    if(n_text == NULL) return 1;

    if(text == NULL)
    {
        *n_text = ctx->n_text;
        return 0;
    }

    if(*n_text < ctx->n_text) return 1;

    uint32_t i;
    for(i=0; i< ctx->n_text; i++)
    {
        text[i].type = ctx->text_list[i].type;
        memcpy(&text[i].keyword, ctx->text_list[i].keyword, strlen(ctx->text_list[i].keyword) + 1);
        text[i].compression_method = 0;
        text[i].compression_flag = ctx->text_list[i].compression_flag;
        text[i].language_tag = ctx->text_list[i].language_tag;
        text[i].translated_keyword = ctx->text_list[i].translated_keyword;
        text[i].length = ctx->text_list[i].text_length;
        text[i].text = ctx->text_list[i].text;
    }

    return ret;
}

int spng_get_bkgd(spng_ctx *ctx, struct spng_bkgd *bkgd)
{
    SPNG_GET_CHUNK_BOILERPLATE(bkgd);

    *bkgd = ctx->bkgd;

    return 0;
}

int spng_get_hist(spng_ctx *ctx, struct spng_hist *hist)
{
    SPNG_GET_CHUNK_BOILERPLATE(hist);

    *hist = ctx->hist;

    return 0;
}

int spng_get_phys(spng_ctx *ctx, struct spng_phys *phys)
{
    SPNG_GET_CHUNK_BOILERPLATE(phys);

    *phys = ctx->phys;

    return 0;
}

int spng_get_splt(spng_ctx *ctx, struct spng_splt *splt, uint32_t *n_splt)
{
    if(ctx == NULL) return 1;
    int ret = read_chunks(ctx, 0);
    if(ret) return ret;
    if(!ctx->stored.splt) return SPNG_ECHUNKAVAIL;
    if(n_splt == NULL) return 1;

    if(splt == NULL)
    {
        *n_splt = ctx->n_splt;
        return 0;
    }

    if(*n_splt < ctx->n_splt) return 1;

    memcpy(splt, &ctx->splt_list, ctx->n_splt * sizeof(struct spng_splt));

    return 0;
}

int spng_get_time(spng_ctx *ctx, struct spng_time *time)
{
    SPNG_GET_CHUNK_BOILERPLATE(time);

    *time = ctx->time;

    return 0;
}

int spng_get_offs(spng_ctx *ctx, struct spng_offs *offs)
{
    SPNG_GET_CHUNK_BOILERPLATE(offs);

    *offs = ctx->offs;

    return 0;
}

int spng_get_exif(spng_ctx *ctx, struct spng_exif *exif)
{
    SPNG_GET_CHUNK_BOILERPLATE(exif);

    *exif = ctx->exif;

    return 0;
}

int spng_set_ihdr(spng_ctx *ctx, struct spng_ihdr *ihdr)
{
    SPNG_SET_CHUNK_BOILERPLATE(ihdr);

    if(ctx->stored.ihdr) return 1;

    ret = check_ihdr(ihdr, ctx->max_width, ctx->max_height);
    if(ret) return ret;

    ctx->ihdr = *ihdr;

    ctx->stored.ihdr = 1;
    ctx->user.ihdr = 1;

    return 0;
}

int spng_set_plte(spng_ctx *ctx, struct spng_plte *plte)
{
    SPNG_SET_CHUNK_BOILERPLATE(plte);

    if(!ctx->stored.ihdr) return 1;

    if(check_plte(plte, &ctx->ihdr)) return 1;

    ctx->plte = *plte;

    ctx->stored.plte = 1;
    ctx->user.plte = 1;

    return 0;
}

int spng_set_trns(spng_ctx *ctx, struct spng_trns *trns)
{
    SPNG_SET_CHUNK_BOILERPLATE(trns);

    if(!ctx->stored.ihdr) return 1;

    uint16_t mask = ~0;
    if(ctx->ihdr.bit_depth < 16) mask = (1 << ctx->ihdr.bit_depth) - 1;

    if(ctx->ihdr.color_type == 0)
    {
        trns->gray &= mask;
    }
    else if(ctx->ihdr.color_type == 2)
    {
        trns->red &= mask;
        trns->green &= mask;
        trns->blue &= mask;
    }
    else if(ctx->ihdr.color_type == 3)
    {
        if(!ctx->stored.plte) return SPNG_ETRNS_NO_PLTE;
    }
    else return SPNG_ETRNS_COLOR_TYPE;

    ctx->trns = *trns;

    ctx->stored.trns = 1;
    ctx->user.trns = 1;

    return 0;
}

int spng_set_chrm(spng_ctx *ctx, struct spng_chrm *chrm)
{
    SPNG_SET_CHUNK_BOILERPLATE(chrm);

    struct spng_chrm_int chrm_int;

    chrm_int.white_point_x = (uint32_t)(chrm->white_point_x * 100000.0);
    chrm_int.white_point_y = (uint32_t)(chrm->white_point_y * 100000.0);
    chrm_int.red_x = (uint32_t)(chrm->red_x * 100000.0);
    chrm_int.red_y = (uint32_t)(chrm->red_y * 100000.0);
    chrm_int.green_x = (uint32_t)(chrm->green_x * 100000.0);
    chrm_int.green_y = (uint32_t)(chrm->green_y * 100000.0);
    chrm_int.blue_x = (uint32_t)(chrm->blue_x * 100000.0);
    chrm_int.blue_y = (uint32_t)(chrm->blue_y * 100000.0);

    if(check_chrm_int(&chrm_int)) return SPNG_ECHRM;

    ctx->chrm_int = chrm_int;

    ctx->stored.chrm = 1;
    ctx->user.chrm = 1;

    return 0;
}

int spng_set_chrm_int(spng_ctx *ctx, struct spng_chrm_int *chrm_int)
{
    SPNG_SET_CHUNK_BOILERPLATE(chrm_int);

    if(check_chrm_int(chrm_int)) return SPNG_ECHRM;

    ctx->chrm_int = *chrm_int;

    ctx->stored.chrm = 1;
    ctx->user.chrm = 1;

    return 0;
}

int spng_set_gama(spng_ctx *ctx, double gamma)
{
    SPNG_SET_CHUNK_BOILERPLATE(ctx);

    uint32_t gama = gamma * 100000.0;

    if(!gama) return 1;
    if(gama > png_u32max) return 1;

    ctx->gama = gama;

    ctx->stored.gama = 1;
    ctx->user.gama = 1;

    return 0;
}

int spng_set_iccp(spng_ctx *ctx, struct spng_iccp *iccp)
{
    SPNG_SET_CHUNK_BOILERPLATE(iccp);

    if(check_png_keyword(iccp->profile_name)) return SPNG_EICCP_NAME;
    if(!iccp->profile_len) return 1;

    if(ctx->iccp.profile && !ctx->user.iccp) spng__free(ctx, ctx->iccp.profile);

    ctx->iccp = *iccp;

    ctx->stored.iccp = 1;
    ctx->user.iccp = 1;

    return 0;
}

int spng_set_sbit(spng_ctx *ctx, struct spng_sbit *sbit)
{
    SPNG_SET_CHUNK_BOILERPLATE(sbit);

    if(check_sbit(sbit, &ctx->ihdr)) return 1;

    if(!ctx->stored.ihdr) return 1;

    ctx->sbit = *sbit;

    ctx->stored.sbit = 1;
    ctx->user.sbit = 1;

    return 0;
}

int spng_set_srgb(spng_ctx *ctx, uint8_t rendering_intent)
{
    SPNG_SET_CHUNK_BOILERPLATE(ctx);

    if(rendering_intent > 3) return 1;

    ctx->srgb_rendering_intent = rendering_intent;

    ctx->stored.srgb = 1;
    ctx->user.srgb = 1;

    return 0;
}

int spng_set_text(spng_ctx *ctx, struct spng_text *text, uint32_t n_text)
{
    if(!n_text) return 1;
    SPNG_SET_CHUNK_BOILERPLATE(text);

    return 0; /* XXX: fix this for encode support */
/*
    uint32_t i;
    for(i=0; i < n_text; i++)
    {
        if(check_png_keyword(text[i].keyword)) return SPNG_ETEXT_KEYWORD;
        if(!text[i].length) return 1;
        if(text[i].text == NULL) return 1;

        if(text[i].type == SPNG_TEXT)
        {
            if(check_png_text(text[i].text, text[i].length)) return 1;
        }
        else if(text[i].type == SPNG_ZTXT)
        {
            if(check_png_text(text[i].text, text[i].length)) return 1;

            if(text[i].compression_method != 0) return 1;
        }
        else if(text[i].type == SPNG_ITXT)
        {
            if(text[i].compression_flag > 1) return 1;
            if(text[i].compression_method != 0) return 1;
            if(text[i].language_tag == NULL) return SPNG_EITXT_LANG_TAG;
            if(text[i].translated_keyword == NULL) return SPNG_EITXT_TRANSLATED_KEY;

        }
        else return 1;

    }

    if(ctx->text_list != NULL && !ctx->user.text)
    {
        for(i=0; i < ctx->n_text; i++)
        {
            spng__free(ctx, ctx->text_list[i].keyword);
        }
        spng__free(ctx, ctx->text_list);
    }

    ctx->text_list = text;
    ctx->n_text = n_text;

    ctx->stored.text = 1;
    ctx->user.text = 1;

    return 0;*/
}

int spng_set_bkgd(spng_ctx *ctx, struct spng_bkgd *bkgd)
{
    SPNG_SET_CHUNK_BOILERPLATE(bkgd);

    if(!ctx->stored.ihdr)  return 1;

    uint16_t mask = ~0;

    if(ctx->ihdr.bit_depth < 16) mask = (1 << ctx->ihdr.bit_depth) - 1;

    if(ctx->ihdr.color_type == 0 || ctx->ihdr.color_type == 4)
    {
        bkgd->gray &= mask;
    }
    else if(ctx->ihdr.color_type == 2 || ctx->ihdr.color_type == 6)
    {
        bkgd->red &= mask;
        bkgd->green &= mask;
        bkgd->blue &= mask;
    }
    else if(ctx->ihdr.color_type == 3)
    {
        if(!ctx->stored.bkgd) return SPNG_EBKGD_NO_PLTE;
        if(bkgd->plte_index >= ctx->plte.n_entries) return SPNG_EBKGD_PLTE_IDX;
    }

    ctx->bkgd = *bkgd;

    ctx->stored.bkgd = 1;
    ctx->user.bkgd = 1;

    return 0;
}

int spng_set_hist(spng_ctx *ctx, struct spng_hist *hist)
{
    SPNG_SET_CHUNK_BOILERPLATE(hist);

    if(!ctx->stored.plte) return SPNG_EHIST_NO_PLTE;

    ctx->hist = *hist;

    ctx->stored.hist = 1;
    ctx->user.hist = 1;

    return 0;
}

int spng_set_phys(spng_ctx *ctx, struct spng_phys *phys)
{
    SPNG_SET_CHUNK_BOILERPLATE(phys);

    if(check_phys(phys)) return SPNG_EPHYS;

    ctx->phys = *phys;

    ctx->stored.phys = 1;
    ctx->user.phys = 1;

    return 0;
}

int spng_set_splt(spng_ctx *ctx, struct spng_splt *splt, uint32_t n_splt)
{
    if(!n_splt) return 1;
    SPNG_SET_CHUNK_BOILERPLATE(splt);

    uint32_t i;
    for(i=0; i < n_splt; i++)
    {
        if(check_png_keyword(splt[i].name)) return SPNG_ESPLT_NAME;
        if( !(splt[i].sample_depth == 8 || splt[i].sample_depth == 16) ) return SPNG_ESPLT_DEPTH;
    }

    if(ctx->stored.splt && !ctx->user.splt)
    {
        for(i=0; i < ctx->n_splt; i++)
        {
            if(ctx->splt_list[i].entries != NULL) spng__free(ctx, ctx->splt_list[i].entries);
        }
        spng__free(ctx, ctx->splt_list);
    }

    ctx->splt_list = splt;
    ctx->n_splt = n_splt;

    ctx->stored.splt = 1;
    ctx->user.splt = 1;

    return 0;
}

int spng_set_time(spng_ctx *ctx, struct spng_time *time)
{
    SPNG_SET_CHUNK_BOILERPLATE(time);

    if(check_time(time)) return SPNG_ETIME;

    ctx->time = *time;

    ctx->stored.time = 1;
    ctx->user.time = 1;

    return 0;
}

int spng_set_offs(spng_ctx *ctx, struct spng_offs *offs)
{
    SPNG_SET_CHUNK_BOILERPLATE(offs);

    if(check_offs(offs)) return SPNG_EOFFS;

    ctx->offs = *offs;

    ctx->stored.offs = 1;
    ctx->user.offs = 1;

    return 0;
}

int spng_set_exif(spng_ctx *ctx, struct spng_exif *exif)
{
    SPNG_SET_CHUNK_BOILERPLATE(exif);

    if(check_exif(exif)) return SPNG_EEXIF;

    if(ctx->exif.data != NULL && !ctx->user.exif) spng__free(ctx, ctx->exif.data);

    ctx->exif = *exif;

    ctx->stored.exif = 1;
    ctx->user.exif = 1;

    return 0;
}

const char *spng_strerror(int err)
{
    switch(err)
    {
        case SPNG_IO_EOF: return "end of stream";
        case SPNG_IO_ERROR: return "stream error";
        case SPNG_OK: return "success";
        case SPNG_EINVAL: return "invalid argument";
        case SPNG_EMEM: return "out of memory";
        case SPNG_EOVERFLOW: return "arithmetic overflow";
        case SPNG_ESIGNATURE: return "invalid signature";
        case SPNG_EWIDTH: return "invalid image width";
        case SPNG_EHEIGHT: return "invalid image height";
        case SPNG_EUSER_WIDTH: return "image width exceeds user limit";
        case SPNG_EUSER_HEIGHT: return "image height exceeds user limit";
        case SPNG_EBIT_DEPTH: return "invalid bit depth";
        case SPNG_ECOLOR_TYPE: return "invalid color type";
        case SPNG_ECOMPRESSION_METHOD: return "invalid compression method";
        case SPNG_EFILTER_METHOD: return "invalid filter method";
        case SPNG_EINTERLACE_METHOD: return "invalid interlace method";
        case SPNG_EIHDR_SIZE: return "invalid IHDR chunk size";
        case SPNG_ENOIHDR: return "missing IHDR chunk";
        case SPNG_ECHUNK_POS: return "invalid chunk position";
        case SPNG_ECHUNK_SIZE: return "invalid chunk length";
        case SPNG_ECHUNK_CRC: return "invalid chunk checksum";
        case SPNG_ECHUNK_TYPE: return "invalid chunk type";
        case SPNG_ECHUNK_UNKNOWN_CRITICAL: return "unknown critical chunk";
        case SPNG_EDUP_PLTE: return "duplicate PLTE chunk";
        case SPNG_EDUP_CHRM: return "duplicate cHRM chunk";
        case SPNG_EDUP_GAMA: return "duplicate gAMA chunk";
        case SPNG_EDUP_ICCP: return "duplicate iCCP chunk";
        case SPNG_EDUP_SBIT: return "duplicate sBIT chunk";
        case SPNG_EDUP_SRGB: return "duplicate sRGB chunk";
        case SPNG_EDUP_BKGD: return "duplicate bKGD chunk";
        case SPNG_EDUP_HIST: return "duplicate hIST chunk";
        case SPNG_EDUP_TRNS: return "duplicate tRNS chunk";
        case SPNG_EDUP_PHYS: return "duplicate pHYs chunk";
        case SPNG_EDUP_TIME: return "duplicate tIME chunk";
        case SPNG_EDUP_OFFS: return "duplicate oFFs chunk";
        case SPNG_EDUP_EXIF: return "duplicate eXIf chunk";
        case SPNG_ECHRM: return "invalid cHRM chunk";
        case SPNG_EPLTE_IDX: return "invalid palette (PLTE) index";
        case SPNG_ETRNS_COLOR_TYPE: return "tRNS chunk with incompatible color type";
        case SPNG_ETRNS_NO_PLTE: return "missing palette (PLTE) for tRNS chunk";
        case SPNG_EGAMA: return "invalid gAMA chunk";
        case SPNG_EICCP_NAME: return "invalid iCCP profile name";
        case SPNG_EICCP_COMPRESSION_METHOD: return "invalid iCCP compression method";
        case SPNG_ESBIT: return "invalid sBIT chunk";
        case SPNG_ESRGB: return "invalid sRGB chunk";
        case SPNG_ETEXT: return "invalid tEXt chunk";
        case SPNG_ETEXT_KEYWORD: return "invalid tEXt keyword";
        case SPNG_EZTXT: return "invalid zTXt chunk";
        case SPNG_EZTXT_COMPRESSION_METHOD: return "invalid zTXt compression method";
        case SPNG_EITXT: return "invalid iTXt chunk";
        case SPNG_EITXT_COMPRESSION_FLAG: return "invalid iTXt compression flag";
        case SPNG_EITXT_COMPRESSION_METHOD: return "invalid iTXt compression method";
        case SPNG_EITXT_LANG_TAG: return "invalid iTXt language tag";
        case SPNG_EITXT_TRANSLATED_KEY: return "invalid iTXt translated key";
        case SPNG_EBKGD_NO_PLTE: return "missing palette for bKGD chunk";
        case SPNG_EBKGD_PLTE_IDX: return "invalid palette index for bKGD chunk";
        case SPNG_EHIST_NO_PLTE: return "missing palette for hIST chunk";
        case SPNG_EPHYS: return "invalid pHYs chunk";
        case SPNG_ESPLT_NAME: return "invalid suggested palette name";
        case SPNG_ESPLT_DUP_NAME: return "duplicate suggested palette (sPLT) name";
        case SPNG_ESPLT_DEPTH: return "invalid suggested palette (sPLT) sample depth";
        case SPNG_ETIME: return "invalid tIME chunk";
        case SPNG_EOFFS: return "invalid oFFs chunk";
        case SPNG_EEXIF: return "invalid eXIf chunk";
        case SPNG_EIDAT_TOO_SHORT: return "IDAT stream too short";
        case SPNG_EIDAT_STREAM: return "IDAT stream error";
        case SPNG_EZLIB: return "zlib error";
        case SPNG_EFILTER: return "invalid scanline filter";
        case SPNG_EBUFSIZ: return "output buffer too small";
        case SPNG_EIO: return "i/o error";
        case SPNG_EOF: return "end of file";
        case SPNG_EBUF_SET: return "buffer already set";
        case SPNG_EBADSTATE: return "non-recoverable state";
        case SPNG_EFMT: return "invalid format";
        case SPNG_EFLAGS: return "invalid flags";
        case SPNG_ECHUNKAVAIL: return "chunk not available";
        case SPNG_ENCODE_ONLY: return "encode only context";
        case SPNG_EOI: return "reached end-of-image state";
        case SPNG_ENOPLTE: return "missing PLTE for indexed image";
        case SPNG_ECHUNK_LIMITS: return "reached chunk/cache limits";
        case SPNG_EZLIB_INIT: return "zlib init error";
        case SPNG_ECHUNK_STDLEN: return "chunk exceeds maximum standard length";
        case SPNG_EINTERNAL: return "internal error";
        default: return "unknown error";
    }
}

const char *spng_version_string(void)
{
    return SPNG_VERSION_STRING;
}

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

/* The following SIMD optimizations are derived from libpng source code. */

/*
* PNG Reference Library License version 2
*
* Copyright (c) 1995-2019 The PNG Reference Library Authors.
* Copyright (c) 2018-2019 Cosmin Truta.
* Copyright (c) 2000-2002, 2004, 2006-2018 Glenn Randers-Pehrson.
* Copyright (c) 1996-1997 Andreas Dilger.
* Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
*
* The software is supplied "as is", without warranty of any kind,
* express or implied, including, without limitation, the warranties
* of merchantability, fitness for a particular purpose, title, and
* non-infringement.  In no event shall the Copyright owners, or
* anyone distributing the software, be liable for any damages or
* other liability, whether in contract, tort or otherwise, arising
* from, out of, or in connection with the software, or the use or
* other dealings in the software, even if advised of the possibility
* of such damage.
*
* Permission is hereby granted to use, copy, modify, and distribute
* this software, or portions hereof, for any purpose, without fee,
* subject to the following restrictions:
*
*  1. The origin of this software must not be misrepresented; you
*     must not claim that you wrote the original software.  If you
*     use this software in a product, an acknowledgment in the product
*     documentation would be appreciated, but is not required.
*
*  2. Altered source versions must be plainly marked as such, and must
*     not be misrepresented as being the original software.
*
*  3. This Copyright notice may not be removed or altered from any
*     source or altered source distribution.
*/

#if defined(SPNG_X86)

#ifndef SPNG_SSE
    #define SPNG_SSE 1
#endif

#if defined(__GNUC__) && !defined(__clang__)
    #if SPNG_SSE == 3
        #pragma GCC target("ssse3")
    #elif SPNG_SSE == 4
        #pragma GCC target("sse4.1")
    #else
        #pragma GCC target("sse2")
    #endif
#endif

/* SSE2 optimised filter functions
 * Derived from filter_neon_intrinsics.c
 *
 * Copyright (c) 2018 Cosmin Truta
 * Copyright (c) 2016-2017 Glenn Randers-Pehrson
 * Written by Mike Klein and Matt Sarett
 * Derived from arm/filter_neon_intrinsics.c
 *
 * This code is derived from libpng source code.
 * For conditions of distribution and use, see the disclaimer
 * and license above.
 */

#include <immintrin.h>
#include <inttypes.h>
#include <string.h>

/* Functions in this file look at most 3 pixels (a,b,c) to predict the 4th (d).
 * They're positioned like this:
 *    prev:  c b
 *    row:   a d
 * The Sub filter predicts d=a, Avg d=(a+b)/2, and Paeth predicts d to be
 * whichever of a, b, or c is closest to p=a+b-c.
 */

static __m128i load4(const void* p)
{
   int tmp;
   memcpy(&tmp, p, sizeof(tmp));
   return _mm_cvtsi32_si128(tmp);
}

static void store4(void* p, __m128i v)
{
   int tmp = _mm_cvtsi128_si32(v);
   memcpy(p, &tmp, sizeof(int));
}

static __m128i load3(const void* p)
{
   uint32_t tmp = 0;
   memcpy(&tmp, p, 3);
   return _mm_cvtsi32_si128(tmp);
}

static void store3(void* p, __m128i v)
{
   int tmp = _mm_cvtsi128_si32(v);
   memcpy(p, &tmp, 3);
}

static void defilter_sub3(size_t rowbytes, unsigned char *row)
{
   /* The Sub filter predicts each pixel as the previous pixel, a.
    * There is no pixel to the left of the first pixel.  It's encoded directly.
    * That works with our main loop if we just say that left pixel was zero.
    */
   size_t rb = rowbytes;

   __m128i a, d = _mm_setzero_si128();

   while (rb >= 4) {
      a = d; d = load4(row);
      d = _mm_add_epi8(d, a);
      store3(row, d);

      row += 3;
      rb  -= 3;
   }
   if (rb > 0) {
      a = d; d = load3(row);
      d = _mm_add_epi8(d, a);
      store3(row, d);
   }
}

static void defilter_sub4(size_t rowbytes, unsigned char *row)
{
   /* The Sub filter predicts each pixel as the previous pixel, a.
    * There is no pixel to the left of the first pixel.  It's encoded directly.
    * That works with our main loop if we just say that left pixel was zero.
    */
   size_t rb = rowbytes+4;

   __m128i a, d = _mm_setzero_si128();

   while (rb > 4) {
      a = d; d = load4(row);
      d = _mm_add_epi8(d, a);
      store4(row, d);

      row += 4;
      rb  -= 4;
   }
}

static void defilter_avg3(size_t rowbytes, unsigned char *row, const unsigned char *prev)
{
   /* The Avg filter predicts each pixel as the (truncated) average of a and b.
    * There's no pixel to the left of the first pixel.  Luckily, it's
    * predicted to be half of the pixel above it.  So again, this works
    * perfectly with our loop if we make sure a starts at zero.
    */

   size_t rb = rowbytes;

   const __m128i zero = _mm_setzero_si128();

   __m128i    b;
   __m128i a, d = zero;

   while (rb >= 4)
   {
      __m128i avg;
             b = load4(prev);
      a = d; d = load4(row );

      /* PNG requires a truncating average, so we can't just use _mm_avg_epu8 */
      avg = _mm_avg_epu8(a,b);
      /* ...but we can fix it up by subtracting off 1 if it rounded up. */
      avg = _mm_sub_epi8(avg, _mm_and_si128(_mm_xor_si128(a, b),
                                            _mm_set1_epi8(1)));
      d = _mm_add_epi8(d, avg);
      store3(row, d);

      prev += 3;
      row  += 3;
      rb   -= 3;
   }

   if (rb > 0)
   {
      __m128i avg;
             b = load3(prev);
      a = d; d = load3(row );

      /* PNG requires a truncating average, so we can't just use _mm_avg_epu8 */
      avg = _mm_avg_epu8(a, b);
      /* ...but we can fix it up by subtracting off 1 if it rounded up. */
      avg = _mm_sub_epi8(avg, _mm_and_si128(_mm_xor_si128(a, b),
                                            _mm_set1_epi8(1)));

      d = _mm_add_epi8(d, avg);
      store3(row, d);
   }
}

static void defilter_avg4(size_t rowbytes, unsigned char *row, const unsigned char *prev)
{
   /* The Avg filter predicts each pixel as the (truncated) average of a and b.
    * There's no pixel to the left of the first pixel.  Luckily, it's
    * predicted to be half of the pixel above it.  So again, this works
    * perfectly with our loop if we make sure a starts at zero.
    */
   size_t rb = rowbytes+4;

   const __m128i zero = _mm_setzero_si128();
   __m128i    b;
   __m128i a, d = zero;

   while (rb > 4)
   {
      __m128i avg;
             b = load4(prev);
      a = d; d = load4(row );

      /* PNG requires a truncating average, so we can't just use _mm_avg_epu8 */
      avg = _mm_avg_epu8(a,b);
      /* ...but we can fix it up by subtracting off 1 if it rounded up. */
      avg = _mm_sub_epi8(avg, _mm_and_si128(_mm_xor_si128(a, b),
                                            _mm_set1_epi8(1)));

      d = _mm_add_epi8(d, avg);
      store4(row, d);

      prev += 4;
      row  += 4;
      rb   -= 4;
   }
}

/* Returns |x| for 16-bit lanes. */
#if (SPNG_SSE >= 3) && !defined(_MSC_VER)
__attribute__((target("ssse3")))
#endif
static __m128i abs_i16(__m128i x)
{
#if SPNG_SSE >= 3
   return _mm_abs_epi16(x);
#else
   /* Read this all as, return x<0 ? -x : x.
   * To negate two's complement, you flip all the bits then add 1.
    */
   __m128i is_negative = _mm_cmplt_epi16(x, _mm_setzero_si128());

   /* Flip negative lanes. */
   x = _mm_xor_si128(x, is_negative);

   /* +1 to negative lanes, else +0. */
   x = _mm_sub_epi16(x, is_negative);
   return x;
#endif
}

/* Bytewise c ? t : e. */
static __m128i if_then_else(__m128i c, __m128i t, __m128i e)
{
#if SPNG_SSE >= 4
   return _mm_blendv_epi8(e, t, c);
#else
   return _mm_or_si128(_mm_and_si128(c, t), _mm_andnot_si128(c, e));
#endif
}

static void defilter_paeth3(size_t rowbytes, unsigned char *row, const unsigned char *prev)
{
   /* Paeth tries to predict pixel d using the pixel to the left of it, a,
    * and two pixels from the previous row, b and c:
    *   prev: c b
    *   row:  a d
    * The Paeth function predicts d to be whichever of a, b, or c is nearest to
    * p=a+b-c.
    *
    * The first pixel has no left context, and so uses an Up filter, p = b.
    * This works naturally with our main loop's p = a+b-c if we force a and c
    * to zero.
    * Here we zero b and d, which become c and a respectively at the start of
    * the loop.
    */
   size_t rb = rowbytes;
   const __m128i zero = _mm_setzero_si128();
   __m128i c, b = zero,
           a, d = zero;

   while (rb >= 4)
   {
      /* It's easiest to do this math (particularly, deal with pc) with 16-bit
       * intermediates.
       */
      __m128i pa,pb,pc,smallest,nearest;
      c = b; b = _mm_unpacklo_epi8(load4(prev), zero);
      a = d; d = _mm_unpacklo_epi8(load4(row ), zero);

      /* (p-a) == (a+b-c - a) == (b-c) */

      pa = _mm_sub_epi16(b, c);

      /* (p-b) == (a+b-c - b) == (a-c) */
      pb = _mm_sub_epi16(a, c);

      /* (p-c) == (a+b-c - c) == (a+b-c-c) == (b-c)+(a-c) */
      pc = _mm_add_epi16(pa, pb);

      pa = abs_i16(pa);  /* |p-a| */
      pb = abs_i16(pb);  /* |p-b| */
      pc = abs_i16(pc);  /* |p-c| */

      smallest = _mm_min_epi16(pc, _mm_min_epi16(pa, pb));

      /* Paeth breaks ties favoring a over b over c. */
      nearest  = if_then_else(_mm_cmpeq_epi16(smallest, pa), a,
                         if_then_else(_mm_cmpeq_epi16(smallest, pb), b, c));

      /* Note `_epi8`: we need addition to wrap modulo 255. */
      d = _mm_add_epi8(d, nearest);
      store3(row, _mm_packus_epi16(d, d));

      prev += 3;
      row  += 3;
      rb   -= 3;
   }

   if (rb > 0)
   {
      /* It's easiest to do this math (particularly, deal with pc) with 16-bit
       * intermediates.
       */
      __m128i pa,pb,pc,smallest,nearest;
      c = b; b = _mm_unpacklo_epi8(load3(prev), zero);
      a = d; d = _mm_unpacklo_epi8(load3(row ), zero);

      /* (p-a) == (a+b-c - a) == (b-c) */
      pa = _mm_sub_epi16(b, c);

      /* (p-b) == (a+b-c - b) == (a-c) */
      pb = _mm_sub_epi16(a, c);

      /* (p-c) == (a+b-c - c) == (a+b-c-c) == (b-c)+(a-c) */
      pc = _mm_add_epi16(pa, pb);

      pa = abs_i16(pa);  /* |p-a| */
      pb = abs_i16(pb);  /* |p-b| */
      pc = abs_i16(pc);  /* |p-c| */

      smallest = _mm_min_epi16(pc, _mm_min_epi16(pa, pb));

      /* Paeth breaks ties favoring a over b over c. */
      nearest  = if_then_else(_mm_cmpeq_epi16(smallest, pa), a,
                         if_then_else(_mm_cmpeq_epi16(smallest, pb), b, c));

      /* Note `_epi8`: we need addition to wrap modulo 255. */
      d = _mm_add_epi8(d, nearest);
      store3(row, _mm_packus_epi16(d, d));
   }
}

static void defilter_paeth4(size_t rowbytes, unsigned char *row, const unsigned char *prev)
{
   /* Paeth tries to predict pixel d using the pixel to the left of it, a,
    * and two pixels from the previous row, b and c:
    *   prev: c b
    *   row:  a d
    * The Paeth function predicts d to be whichever of a, b, or c is nearest to
    * p=a+b-c.
    *
    * The first pixel has no left context, and so uses an Up filter, p = b.
    * This works naturally with our main loop's p = a+b-c if we force a and c
    * to zero.
    * Here we zero b and d, which become c and a respectively at the start of
    * the loop.
    */
   size_t rb = rowbytes+4;

   const __m128i zero = _mm_setzero_si128();
   __m128i pa,pb,pc,smallest,nearest;
   __m128i c, b = zero,
           a, d = zero;

   while (rb > 4)
   {
      /* It's easiest to do this math (particularly, deal with pc) with 16-bit
       * intermediates.
       */
      c = b; b = _mm_unpacklo_epi8(load4(prev), zero);
      a = d; d = _mm_unpacklo_epi8(load4(row ), zero);

      /* (p-a) == (a+b-c - a) == (b-c) */
      pa = _mm_sub_epi16(b, c);

      /* (p-b) == (a+b-c - b) == (a-c) */
      pb = _mm_sub_epi16(a, c);

      /* (p-c) == (a+b-c - c) == (a+b-c-c) == (b-c)+(a-c) */
      pc = _mm_add_epi16(pa, pb);

      pa = abs_i16(pa);  /* |p-a| */
      pb = abs_i16(pb);  /* |p-b| */
      pc = abs_i16(pc);  /* |p-c| */

      smallest = _mm_min_epi16(pc, _mm_min_epi16(pa, pb));

      /* Paeth breaks ties favoring a over b over c. */
      nearest  = if_then_else(_mm_cmpeq_epi16(smallest, pa), a,
                         if_then_else(_mm_cmpeq_epi16(smallest, pb), b, c));

      /* Note `_epi8`: we need addition to wrap modulo 255. */
      d = _mm_add_epi8(d, nearest);
      store4(row, _mm_packus_epi16(d, d));

      prev += 4;
      row  += 4;
      rb   -= 4;
   }
}

#endif /* SPNG_X86 */


#if defined(SPNG_ARM)

/* NEON optimised filter functions
 * Derived from filter_neon_intrinsics.c
 *
 * Copyright (c) 2018 Cosmin Truta
 * Copyright (c) 2014,2016 Glenn Randers-Pehrson
 * Written by James Yu <james.yu at linaro.org>, October 2013.
 * Based on filter_neon.S, written by Mans Rullgard, 2011.
 *
 * This code is derived from libpng source code.
 * For conditions of distribution and use, see the disclaimer
 * and license in this file.
 */

#define png_aligncast(type, value) ((void*)(value))
#define png_aligncastconst(type, value) ((const void*)(value))

/* libpng row pointers are not necessarily aligned to any particular boundary,
 * however this code will only work with appropriate alignment. mips/mips_init.c
 * checks for this (and will not compile unless it is done). This code uses
 * variants of png_aligncast to avoid compiler warnings.
 */
#define png_ptr(type,pointer) png_aligncast(type *,pointer)
#define png_ptrc(type,pointer) png_aligncastconst(const type *,pointer)

/* The following relies on a variable 'temp_pointer' being declared with type
 * 'type'.  This is written this way just to hide the GCC strict aliasing
 * warning; note that the code is safe because there never is an alias between
 * the input and output pointers.
 */
#define png_ldr(type,pointer)\
   (temp_pointer = png_ptr(type,pointer), *temp_pointer)


#if defined(_MSC_VER) && !defined(__clang__) && defined(_M_ARM64)
    #include <arm64_neon.h>
#else
    #include <arm_neon.h>
#endif

static void defilter_sub3(size_t rowbytes, unsigned char *row)
{
   unsigned char *rp = row;
   unsigned char *rp_stop = row + rowbytes;

   uint8x16_t vtmp = vld1q_u8(rp);
   uint8x8x2_t *vrpt = png_ptr(uint8x8x2_t, &vtmp);
   uint8x8x2_t vrp = *vrpt;

   uint8x8x4_t vdest;
   vdest.val[3] = vdup_n_u8(0);

   for (; rp < rp_stop;)
   {
      uint8x8_t vtmp1, vtmp2;
      uint32x2_t *temp_pointer;

      vtmp1 = vext_u8(vrp.val[0], vrp.val[1], 3);
      vdest.val[0] = vadd_u8(vdest.val[3], vrp.val[0]);
      vtmp2 = vext_u8(vrp.val[0], vrp.val[1], 6);
      vdest.val[1] = vadd_u8(vdest.val[0], vtmp1);

      vtmp1 = vext_u8(vrp.val[1], vrp.val[1], 1);
      vdest.val[2] = vadd_u8(vdest.val[1], vtmp2);
      vdest.val[3] = vadd_u8(vdest.val[2], vtmp1);

      vtmp = vld1q_u8(rp + 12);
      vrpt = png_ptr(uint8x8x2_t, &vtmp);
      vrp = *vrpt;

      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[0]), 0);
      rp += 3;
      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[1]), 0);
      rp += 3;
      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[2]), 0);
      rp += 3;
      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[3]), 0);
      rp += 3;
   }
}

static void defilter_sub4(size_t rowbytes, unsigned char *row)
{
   unsigned char *rp = row;
   unsigned char *rp_stop = row + rowbytes;

   uint8x8x4_t vdest;
   vdest.val[3] = vdup_n_u8(0);

   for (; rp < rp_stop; rp += 16)
   {
      uint32x2x4_t vtmp = vld4_u32(png_ptr(uint32_t,rp));
      uint8x8x4_t *vrpt = png_ptr(uint8x8x4_t,&vtmp);
      uint8x8x4_t vrp = *vrpt;
      uint32x2x4_t *temp_pointer;
      uint32x2x4_t vdest_val;

      vdest.val[0] = vadd_u8(vdest.val[3], vrp.val[0]);
      vdest.val[1] = vadd_u8(vdest.val[0], vrp.val[1]);
      vdest.val[2] = vadd_u8(vdest.val[1], vrp.val[2]);
      vdest.val[3] = vadd_u8(vdest.val[2], vrp.val[3]);

      vdest_val = png_ldr(uint32x2x4_t, &vdest);
      vst4_lane_u32(png_ptr(uint32_t,rp), vdest_val, 0);
   }
}

static void defilter_avg3(size_t rowbytes, unsigned char *row, const unsigned char *prev_row)
{
   unsigned char *rp = row;
   const unsigned char *pp = prev_row;
   unsigned char *rp_stop = row + rowbytes;

   uint8x16_t vtmp;
   uint8x8x2_t *vrpt;
   uint8x8x2_t vrp;
   uint8x8x4_t vdest;
   vdest.val[3] = vdup_n_u8(0);

   vtmp = vld1q_u8(rp);
   vrpt = png_ptr(uint8x8x2_t,&vtmp);
   vrp = *vrpt;

   for (; rp < rp_stop; pp += 12)
   {
      uint8x8_t vtmp1, vtmp2, vtmp3;

      uint8x8x2_t *vppt;
      uint8x8x2_t vpp;

      uint32x2_t *temp_pointer;

      vtmp = vld1q_u8(pp);
      vppt = png_ptr(uint8x8x2_t,&vtmp);
      vpp = *vppt;

      vtmp1 = vext_u8(vrp.val[0], vrp.val[1], 3);
      vdest.val[0] = vhadd_u8(vdest.val[3], vpp.val[0]);
      vdest.val[0] = vadd_u8(vdest.val[0], vrp.val[0]);

      vtmp2 = vext_u8(vpp.val[0], vpp.val[1], 3);
      vtmp3 = vext_u8(vrp.val[0], vrp.val[1], 6);
      vdest.val[1] = vhadd_u8(vdest.val[0], vtmp2);
      vdest.val[1] = vadd_u8(vdest.val[1], vtmp1);

      vtmp2 = vext_u8(vpp.val[0], vpp.val[1], 6);
      vtmp1 = vext_u8(vrp.val[1], vrp.val[1], 1);

      vtmp = vld1q_u8(rp + 12);
      vrpt = png_ptr(uint8x8x2_t,&vtmp);
      vrp = *vrpt;

      vdest.val[2] = vhadd_u8(vdest.val[1], vtmp2);
      vdest.val[2] = vadd_u8(vdest.val[2], vtmp3);

      vtmp2 = vext_u8(vpp.val[1], vpp.val[1], 1);

      vdest.val[3] = vhadd_u8(vdest.val[2], vtmp2);
      vdest.val[3] = vadd_u8(vdest.val[3], vtmp1);

      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[0]), 0);
      rp += 3;
      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[1]), 0);
      rp += 3;
      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[2]), 0);
      rp += 3;
      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[3]), 0);
      rp += 3;
   }
}

static void defilter_avg4(size_t rowbytes, unsigned char *row, const unsigned char *prev_row)
{
   unsigned char *rp = row;
   unsigned char *rp_stop = row + rowbytes;
   const unsigned char *pp = prev_row;

   uint8x8x4_t vdest;
   vdest.val[3] = vdup_n_u8(0);

   for (; rp < rp_stop; rp += 16, pp += 16)
   {
      uint32x2x4_t vtmp;
      uint8x8x4_t *vrpt, *vppt;
      uint8x8x4_t vrp, vpp;
      uint32x2x4_t *temp_pointer;
      uint32x2x4_t vdest_val;

      vtmp = vld4_u32(png_ptr(uint32_t,rp));
      vrpt = png_ptr(uint8x8x4_t,&vtmp);
      vrp = *vrpt;
      vtmp = vld4_u32(png_ptrc(uint32_t,pp));
      vppt = png_ptr(uint8x8x4_t,&vtmp);
      vpp = *vppt;

      vdest.val[0] = vhadd_u8(vdest.val[3], vpp.val[0]);
      vdest.val[0] = vadd_u8(vdest.val[0], vrp.val[0]);
      vdest.val[1] = vhadd_u8(vdest.val[0], vpp.val[1]);
      vdest.val[1] = vadd_u8(vdest.val[1], vrp.val[1]);
      vdest.val[2] = vhadd_u8(vdest.val[1], vpp.val[2]);
      vdest.val[2] = vadd_u8(vdest.val[2], vrp.val[2]);
      vdest.val[3] = vhadd_u8(vdest.val[2], vpp.val[3]);
      vdest.val[3] = vadd_u8(vdest.val[3], vrp.val[3]);

      vdest_val = png_ldr(uint32x2x4_t, &vdest);
      vst4_lane_u32(png_ptr(uint32_t,rp), vdest_val, 0);
   }
}

static uint8x8_t paeth_arm(uint8x8_t a, uint8x8_t b, uint8x8_t c)
{
   uint8x8_t d, e;
   uint16x8_t p1, pa, pb, pc;

   p1 = vaddl_u8(a, b); /* a + b */
   pc = vaddl_u8(c, c); /* c * 2 */
   pa = vabdl_u8(b, c); /* pa */
   pb = vabdl_u8(a, c); /* pb */
   pc = vabdq_u16(p1, pc); /* pc */

   p1 = vcleq_u16(pa, pb); /* pa <= pb */
   pa = vcleq_u16(pa, pc); /* pa <= pc */
   pb = vcleq_u16(pb, pc); /* pb <= pc */

   p1 = vandq_u16(p1, pa); /* pa <= pb && pa <= pc */

   d = vmovn_u16(pb);
   e = vmovn_u16(p1);

   d = vbsl_u8(d, b, c);
   e = vbsl_u8(e, a, d);

   return e;
}

static void defilter_paeth3(size_t rowbytes, unsigned char *row, const unsigned char *prev_row)
{
   unsigned char *rp = row;
   const unsigned char *pp = prev_row;
   unsigned char *rp_stop = row + rowbytes;

   uint8x16_t vtmp;
   uint8x8x2_t *vrpt;
   uint8x8x2_t vrp;
   uint8x8_t vlast = vdup_n_u8(0);
   uint8x8x4_t vdest;
   vdest.val[3] = vdup_n_u8(0);

   vtmp = vld1q_u8(rp);
   vrpt = png_ptr(uint8x8x2_t,&vtmp);
   vrp = *vrpt;

   for (; rp < rp_stop; pp += 12)
   {
      uint8x8x2_t *vppt;
      uint8x8x2_t vpp;
      uint8x8_t vtmp1, vtmp2, vtmp3;
      uint32x2_t *temp_pointer;

      vtmp = vld1q_u8(pp);
      vppt = png_ptr(uint8x8x2_t,&vtmp);
      vpp = *vppt;

      vdest.val[0] = paeth_arm(vdest.val[3], vpp.val[0], vlast);
      vdest.val[0] = vadd_u8(vdest.val[0], vrp.val[0]);

      vtmp1 = vext_u8(vrp.val[0], vrp.val[1], 3);
      vtmp2 = vext_u8(vpp.val[0], vpp.val[1], 3);
      vdest.val[1] = paeth_arm(vdest.val[0], vtmp2, vpp.val[0]);
      vdest.val[1] = vadd_u8(vdest.val[1], vtmp1);

      vtmp1 = vext_u8(vrp.val[0], vrp.val[1], 6);
      vtmp3 = vext_u8(vpp.val[0], vpp.val[1], 6);
      vdest.val[2] = paeth_arm(vdest.val[1], vtmp3, vtmp2);
      vdest.val[2] = vadd_u8(vdest.val[2], vtmp1);

      vtmp1 = vext_u8(vrp.val[1], vrp.val[1], 1);
      vtmp2 = vext_u8(vpp.val[1], vpp.val[1], 1);

      vtmp = vld1q_u8(rp + 12);
      vrpt = png_ptr(uint8x8x2_t,&vtmp);
      vrp = *vrpt;

      vdest.val[3] = paeth_arm(vdest.val[2], vtmp2, vtmp3);
      vdest.val[3] = vadd_u8(vdest.val[3], vtmp1);

      vlast = vtmp2;

      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[0]), 0);
      rp += 3;
      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[1]), 0);
      rp += 3;
      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[2]), 0);
      rp += 3;
      vst1_lane_u32(png_ptr(uint32_t,rp), png_ldr(uint32x2_t,&vdest.val[3]), 0);
      rp += 3;
   }
}

static void defilter_paeth4(size_t rowbytes, unsigned char *row, const unsigned char *prev_row)
{
   unsigned char *rp = row;
   unsigned char *rp_stop = row + rowbytes;
   const unsigned char *pp = prev_row;

   uint8x8_t vlast = vdup_n_u8(0);
   uint8x8x4_t vdest;
   vdest.val[3] = vdup_n_u8(0);

   for (; rp < rp_stop; rp += 16, pp += 16)
   {
      uint32x2x4_t vtmp;
      uint8x8x4_t *vrpt, *vppt;
      uint8x8x4_t vrp, vpp;
      uint32x2x4_t *temp_pointer;
      uint32x2x4_t vdest_val;

      vtmp = vld4_u32(png_ptr(uint32_t,rp));
      vrpt = png_ptr(uint8x8x4_t,&vtmp);
      vrp = *vrpt;
      vtmp = vld4_u32(png_ptrc(uint32_t,pp));
      vppt = png_ptr(uint8x8x4_t,&vtmp);
      vpp = *vppt;

      vdest.val[0] = paeth_arm(vdest.val[3], vpp.val[0], vlast);
      vdest.val[0] = vadd_u8(vdest.val[0], vrp.val[0]);
      vdest.val[1] = paeth_arm(vdest.val[0], vpp.val[1], vpp.val[0]);
      vdest.val[1] = vadd_u8(vdest.val[1], vrp.val[1]);
      vdest.val[2] = paeth_arm(vdest.val[1], vpp.val[2], vpp.val[1]);
      vdest.val[2] = vadd_u8(vdest.val[2], vrp.val[2]);
      vdest.val[3] = paeth_arm(vdest.val[2], vpp.val[3], vpp.val[2]);
      vdest.val[3] = vadd_u8(vdest.val[3], vrp.val[3]);

      vlast = vpp.val[3];

      vdest_val = png_ldr(uint32x2x4_t, &vdest);
      vst4_lane_u32(png_ptr(uint32_t,rp), vdest_val, 0);
   }
}

#endif /* SPNG_ARM */
