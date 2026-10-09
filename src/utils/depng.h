// My PNG Decoder: https://github.com/KirolosBoshra/DePNG

#ifndef DEPNG_H
#define DEPNG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"

typedef enum {
  DEPNG_OK = 0,
  DEPNG_ERR_IO,
  DEPNG_ERR_OOM,
  DEPNG_ERR_NOT_PNG,
  DEPNG_ERR_TRUNCATED,
  DEPNG_ERR_BAD_IHDR,
  DEPNG_ERR_UNSUPPORTED,
  DEPNG_ERR_BAD_CRC,
  DEPNG_ERR_BAD_DATA,
} DEPNG_ERROR;

static const char *depng_error_str(DEPNG_ERROR err) {
  switch (err) {
  case DEPNG_OK:
    return "ok";
  case DEPNG_ERR_IO:
    return "i/o error";
  case DEPNG_ERR_OOM:
    return "out of memory";
  case DEPNG_ERR_NOT_PNG:
    return "not a png file";
  case DEPNG_ERR_TRUNCATED:
    return "truncated file";
  case DEPNG_ERR_BAD_IHDR:
    return "bad IHDR";
  case DEPNG_ERR_UNSUPPORTED:
    return "unsupported png format";
  case DEPNG_ERR_BAD_CRC:
    return "bad chunk crc";
  case DEPNG_ERR_BAD_DATA:
    return "bad image data";
  default:
    return "unknown error";
  }
}

static const u8 PNG_SIGNATUR[8] = {0x89, 0x50, 0x4E, 0x47,
                                   0x0D, 0x0A, 0x1A, 0x0A};

static bool is_png(const u8 *buf, usize size) {
  if (size < 8)
    return false;

  return memcmp(buf, PNG_SIGNATUR, 8) == 0;
}

typedef struct {
  u32 w, h;
  u8 bit_depth, color_type, comp, filter, interlace;
} DepngIHDR;

static u32 depng_u32be(const u8 *p) {
  return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | (u32)p[3];
}

static u32 depng_crc32(const u8 *data, usize len) {
  static u32 tab[256];
  static bool ready = false;
  if (!ready) {
    for (u32 i = 0; i < 256; i++) {
      u32 c = i;
      for (i32 k = 0; k < 8; k++)
        c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
      tab[i] = c;
    }
    ready = true;
  }
  u32 c = 0xFFFFFFFFu;
  for (usize i = 0; i < len; i++)
    c = tab[(c ^ data[i]) & 0xFF] ^ (c >> 8);
  return c ^ 0xFFFFFFFFu;
}

// Walks chunks from buf, validates IHDR (v1: 8-bit, types 0/2/6,
// non-interlaced) and concatenates all IDAT payloads.
static DEPNG_ERROR depng_parse_png(const u8 *buf, usize size,
                                   DepngIHDR *out_ihdr, u8 **out_idat,
                                   usize *out_idat_len) {
  if (!is_png(buf, size))
    return DEPNG_ERR_NOT_PNG;

  usize off = 8;
  bool seen_ihdr = false;
  u8 *idat = NULL;
  usize idat_len = 0;

  while (off + 8 <= size) {
    u32 len = depng_u32be(buf + off);
    const u8 *type = buf + off + 4;
    if (off + 12 + (usize)len > size || off + 12 + (usize)len < off) {
      free(idat);
      return DEPNG_ERR_TRUNCATED;
    }
    const u8 *data = buf + off + 8;

    if (depng_crc32(buf + off + 4, 4 + (usize)len) != depng_u32be(data + len)) {
      free(idat);
      return DEPNG_ERR_BAD_CRC;
    }

    if (memcmp(type, "IHDR", 4) == 0) {
      if (off != 8 || seen_ihdr || len != 13) {
        free(idat);
        return DEPNG_ERR_BAD_IHDR;
      }
      out_ihdr->w = depng_u32be(data);
      out_ihdr->h = depng_u32be(data + 4);
      out_ihdr->bit_depth = data[8];
      out_ihdr->color_type = data[9];
      out_ihdr->comp = data[10];
      out_ihdr->filter = data[11];
      out_ihdr->interlace = data[12];
      if (out_ihdr->w == 0 || out_ihdr->h == 0) {
        free(idat);
        return DEPNG_ERR_BAD_IHDR;
      }
      if (out_ihdr->comp != 0 || out_ihdr->filter != 0) {
        free(idat);
        return DEPNG_ERR_UNSUPPORTED;
      }
      if (out_ihdr->bit_depth != 8 ||
          (out_ihdr->color_type != 0 && out_ihdr->color_type != 2 &&
           out_ihdr->color_type != 6)) {
        free(idat);
        return DEPNG_ERR_UNSUPPORTED;
      }
      if (out_ihdr->interlace != 0) {
        free(idat);
        return DEPNG_ERR_UNSUPPORTED;
      }
      seen_ihdr = true;
    } else if (memcmp(type, "IDAT", 4) == 0) {
      if (!seen_ihdr) {
        free(idat);
        return DEPNG_ERR_BAD_IHDR;
      }
      if (len > 0) {
        u8 *grown = (u8 *)realloc(idat, idat_len + len);
        if (!grown) {
          free(idat);
          return DEPNG_ERR_OOM;
        }
        idat = grown;
        memcpy(idat + idat_len, data, len);
        idat_len += len;
      }
    } else if (memcmp(type, "IEND", 4) == 0) {
      if (!seen_ihdr) {
        free(idat);
        return DEPNG_ERR_BAD_IHDR;
      }
      *out_idat = idat;
      *out_idat_len = idat_len;
      return idat_len > 0 ? DEPNG_OK : DEPNG_ERR_TRUNCATED;
    }
    // else: skip ancillary chunk
    off += 12 + (usize)len;
  }

  free(idat);
  return DEPNG_ERR_TRUNCATED; // no IEND
}

static u32 depng_adler32(const u8 *data, usize len) {
  u32 a = 1, b = 0;
  for (usize i = 0; i < len; i++) {
    a = (a + data[i]) % 65521;
    b = (b + a) % 65521;
  }
  return (b << 16) | a;
}

// LSB-first bit reader over the DEFLATE stream.
typedef struct {
  const u8 *p, *end;
  u32 buf;
  i32 nbits;
} DepngBits;

static bool depng_need(DepngBits *b, i32 n) {
  while (b->nbits < n) {
    if (b->p == b->end)
      return false;
    b->buf |= (u32)*b->p++ << b->nbits;
    b->nbits += 8;
  }
  return true;
}

static u32 depng_take(DepngBits *b, i32 n) {
  u32 v = b->buf & ((1u << n) - 1);
  b->buf >>= n;
  b->nbits -= n;
  return v;
}

typedef struct {
  u16 count[16]; // count[len] = codes of that bit length
  u16 sym[288];  // symbols sorted by (length, symbol)
} DepngHuff;

static bool depng_huff_build(DepngHuff *h, const u8 *lens, i32 n) {
  i32 offs[16];
  memset(h->count, 0, sizeof h->count);
  for (i32 i = 0; i < n; i++) {
    if (lens[i] >= 16)
      return false;
    h->count[lens[i]]++;
  }
  i32 left = 1, sum = 0;
  for (i32 len = 1; len < 16; len++) {
    left <<= 1;
    left -= h->count[len];
    if (left < 0)
      return false; // over-subscribed
    offs[len] = sum;
    sum += h->count[len];
  }
  for (i32 i = 0; i < n; i++)
    if (lens[i] != 0)
      h->sym[offs[lens[i]]++] = (u16)i;
  return true;
}

// Decodes one symbol, or -1 on bad/ truncated stream.
static i32 depng_huff_decode(DepngHuff *h, DepngBits *b) {
  i32 code = 0, first = 0, index = 0;
  for (i32 len = 1; len < 16; len++) {
    if (!depng_need(b, 1))
      return -1;
    code |= (i32)depng_take(b, 1);
    if (code - first < h->count[len])
      return h->sym[index + (code - first)];
    index += h->count[len];
    first = (first + h->count[len]) << 1;
    code <<= 1;
  }
  return -1;
}

// RFC 1951 length/distance base + extra-bit tables.
static const u16 depng_len_base[29] = {
    3,  4,  5,  6,  7,  8,  9,  10, 11,  13,  15,  17,  19,  23, 27,
    31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static const u8 depng_len_extra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                                       1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
                                       4, 4, 4, 4, 5, 5, 5, 5, 0};
static const u16 depng_dist_base[30] = {
    1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
    33,   49,   65,   97,   129,  193,  257,  385,   513,   769,
    1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
static const u8 depng_dist_extra[30] = {0, 0, 0,  0,  1,  1,  2,  2,  3,  3,
                                        4, 4, 5,  5,  6,  6,  7,  7,  8,  8,
                                        9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

#define DEPNG_ENSURE(n)                                                        \
  do {                                                                         \
    if ((n) > cap) {                                                           \
      if ((n) > exact) {                                                       \
        free(buf);                                                             \
        free(lens_dyn);                                                        \
        return DEPNG_ERR_BAD_DATA;                                             \
      }                                                                        \
      usize nc = cap ? cap * 2 : 256;                                          \
      while (nc < (n))                                                         \
        nc *= 2;                                                               \
      if (nc > exact)                                                          \
        nc = exact;                                                            \
      u8 *nb = (u8 *)realloc(buf, nc ? nc : 1);                                \
      if (!nb) {                                                               \
        free(buf);                                                             \
        free(lens_dyn);                                                        \
        return DEPNG_ERR_OOM;                                                  \
      }                                                                        \
      buf = nb;                                                                \
      cap = nc;                                                                \
    }                                                                          \
  } while (0)

// Raw DEFLATE inflate. Output is capped at `exact` bytes: a valid PNG
// decompresses to exactly the filtered image size, so anything larger is
// corrupt (also bounds allocation on hostile input).
static DEPNG_ERROR depng_inflate(const u8 *in, usize in_len, u8 **out,
                                 usize *out_len, usize exact) {
  u8 *lens_dyn = NULL; // dynamic-block working buffer (NULL for fixed/stored)
  DepngBits b = {in, in + in_len, 0, 0};
  usize cap = exact < 16384 ? exact : 16384;
  u8 *buf = (u8 *)malloc(cap ? cap : 1);
  if (!buf)
    return DEPNG_ERR_OOM;
  usize len = 0;
  bool last = false;

  while (!last) {
    if (!depng_need(&b, 3)) {
      free(buf);
      free(lens_dyn);
      return DEPNG_ERR_BAD_DATA;
    }
    last = depng_take(&b, 1) != 0;
    i32 type = (int)depng_take(&b, 2);

    if (type == 0) { // stored: byte-align, LEN/NLEN, copy
      b.buf = 0;
      b.nbits = 0; // p already sits on the next byte boundary here
      if ((usize)(b.end - b.p) < 4) {
        free(buf);
        free(lens_dyn);
        return DEPNG_ERR_BAD_DATA;
      }
      u16 slen = (u16)(b.p[0] | (b.p[1] << 8));
      u16 nlen = (u16)(b.p[2] | (b.p[3] << 8));
      b.p += 4;
      if (slen != (u16)~nlen || (usize)(b.end - b.p) < slen) {
        free(buf);
        free(lens_dyn);
        return DEPNG_ERR_BAD_DATA;
      }
      DEPNG_ENSURE(len + slen);
      memcpy(buf + len, b.p, slen);
      b.p += slen;
      len += slen;
    } else if (type == 1 || type == 2) {
      DepngHuff lit, dist;
      if (type == 1) { // fixed Huffman codes
        u8 lens[288];
        for (i32 i = 0; i < 144; i++)
          lens[i] = 8;
        for (i32 i = 144; i < 256; i++)
          lens[i] = 9;
        for (i32 i = 256; i < 280; i++)
          lens[i] = 7;
        for (i32 i = 280; i < 288; i++)
          lens[i] = 8;
        u8 dlens[32];
        memset(dlens, 5, sizeof dlens);
        if (!depng_huff_build(&lit, lens, 288) ||
            !depng_huff_build(&dist, dlens, 32)) {
          free(buf);
          free(lens_dyn);
          return DEPNG_ERR_BAD_DATA;
        }
      } else { // dynamic Huffman codes
        static const u8 order[19] = {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                                     11, 4,  12, 3, 13, 2, 14, 1, 15};
        if (!depng_need(&b, 14)) {
          free(buf);
          free(lens_dyn);
          return DEPNG_ERR_BAD_DATA;
        }
        i32 hlit = (int)depng_take(&b, 5) + 257;
        i32 hdist = (int)depng_take(&b, 5) + 1;
        i32 hclen = (int)depng_take(&b, 4) + 4;
        u8 cl_lens[19] = {0};
        for (i32 i = 0; i < hclen; i++) {
          if (!depng_need(&b, 3)) {
            free(buf);
            free(lens_dyn);
            return DEPNG_ERR_BAD_DATA;
          }
          cl_lens[order[i]] = (u8)depng_take(&b, 3);
        }
        DepngHuff cl;
        if (!depng_huff_build(&cl, cl_lens, 19)) {
          free(buf);
          free(lens_dyn);
          return DEPNG_ERR_BAD_DATA;
        }
        free(lens_dyn);
        lens_dyn = (u8 *)malloc((usize)hlit + (usize)hdist);
        if (!lens_dyn) {
          free(buf);
          return DEPNG_ERR_OOM;
        }
        i32 nl = 0;
        while (nl < hlit + hdist) {
          i32 sym = depng_huff_decode(&cl, &b);
          if (sym < 0) {
            free(buf);
            free(lens_dyn);
            return DEPNG_ERR_BAD_DATA;
          }
          if (sym < 16) {
            lens_dyn[nl++] = (u8)sym;
          } else {
            i32 rep = 0, val = 0;
            if (sym == 16) {
              if (nl == 0 || !depng_need(&b, 2)) {
                free(buf);
                free(lens_dyn);
                return DEPNG_ERR_BAD_DATA;
              }
              rep = (int)depng_take(&b, 2) + 3;
              val = lens_dyn[nl - 1];
            } else if (sym == 17) {
              if (!depng_need(&b, 3)) {
                free(buf);
                free(lens_dyn);
                return DEPNG_ERR_BAD_DATA;
              }
              rep = (int)depng_take(&b, 3) + 3;
            } else { // 18
              if (!depng_need(&b, 7)) {
                free(buf);
                free(lens_dyn);
                return DEPNG_ERR_BAD_DATA;
              }
              rep = (int)depng_take(&b, 7) + 11;
            }
            if (nl + rep > hlit + hdist) {
              free(buf);
              free(lens_dyn);
              return DEPNG_ERR_BAD_DATA;
            }
            memset(lens_dyn + nl, val, (usize)rep);
            nl += rep;
          }
        }
        if (lens_dyn[256] == 0) { // end-of-block code required
          free(buf);
          free(lens_dyn);
          return DEPNG_ERR_BAD_DATA;
        }
        if (!depng_huff_build(&lit, lens_dyn, hlit) ||
            !depng_huff_build(&dist, lens_dyn + hlit, hdist)) {
          free(buf);
          free(lens_dyn);
          return DEPNG_ERR_BAD_DATA;
        }
      }

      for (;;) {
        i32 sym = depng_huff_decode(&lit, &b);
        if (sym < 0) {
          free(buf);
          free(lens_dyn);
          return DEPNG_ERR_BAD_DATA;
        }
        if (sym < 256) {
          DEPNG_ENSURE(len + 1);
          buf[len++] = (u8)sym;
        } else if (sym == 256) {
          break;
        } else {
          i32 li = sym - 257;
          if (li >= 29) {
            free(buf);
            free(lens_dyn);
            return DEPNG_ERR_BAD_DATA;
          }
          if (!depng_need(&b, depng_len_extra[li])) {
            free(buf);
            free(lens_dyn);
            return DEPNG_ERR_BAD_DATA;
          }
          usize length =
              depng_len_base[li] + depng_take(&b, depng_len_extra[li]);
          i32 dsym = depng_huff_decode(&dist, &b);
          if (dsym < 0 || dsym >= 30) {
            free(buf);
            free(lens_dyn);
            return DEPNG_ERR_BAD_DATA;
          }
          if (!depng_need(&b, depng_dist_extra[dsym])) {
            free(buf);
            free(lens_dyn);
            return DEPNG_ERR_BAD_DATA;
          }
          usize ds =
              depng_dist_base[dsym] + depng_take(&b, depng_dist_extra[dsym]);
          if (ds == 0 || ds > len) {
            free(buf);
            free(lens_dyn);
            return DEPNG_ERR_BAD_DATA;
          }
          DEPNG_ENSURE(len + length);
          for (usize i = 0; i < length; i++, len++)
            buf[len] = buf[len - ds]; // byte-wise: handles overlap
        }
      }
    } else {
      free(buf);
      free(lens_dyn);
      return DEPNG_ERR_BAD_DATA; // reserved BTYPE=3
    }
  }

  free(lens_dyn);
  if (len != exact) { // valid PNG inflates to exactly the filtered size
    free(buf);
    return DEPNG_ERR_BAD_DATA;
  }
  *out = buf;
  *out_len = len;
  return DEPNG_OK;
}

#undef DEPNG_ENSURE

// zlib (RFC 1950) wrapper around the raw DEFLATE stream.
static DEPNG_ERROR depng_zlib_decompress(const u8 *in, usize in_len, u8 **out,
                                         usize *out_len, usize exact) {
  if (in_len < 6) // 2 header + >=1 deflate + 4 adler
    return DEPNG_ERR_BAD_DATA;
  u8 cmf = in[0], flg = in[1];
  if ((cmf & 0x0F) != 8 || (cmf >> 4) > 7) // deflate, CINFO <= 7 (32K window)
    return DEPNG_ERR_BAD_DATA;
  if ((((u16)cmf << 8 | flg) % 31) != 0)
    return DEPNG_ERR_BAD_DATA;
  if (flg & 0x20) // preset dictionary: PNG never uses it
    return DEPNG_ERR_BAD_DATA;

  DEPNG_ERROR err = depng_inflate(in + 2, in_len - 6, out, out_len, exact);
  if (err != DEPNG_OK)
    return err;
  if (depng_adler32(*out, *out_len) != depng_u32be(in + in_len - 4)) {
    free(*out);
    *out = NULL;
    return DEPNG_ERR_BAD_DATA;
  }
  return DEPNG_OK;
}

static u8 depng_paeth(i32 a, i32 b, i32 c) {
  i32 p = a + b - c, pa = abs(p - a), pb = abs(p - b), pc = abs(p - c);
  return (u8)(pa <= pb && pa <= pc ? a : pb <= pc ? b : c);
}

// Reverses PNG filtering and expands scanlines to RGBA.
static DEPNG_ERROR depng_unfilter_to_rgba(const u8 *raw, usize raw_len,
                                          const DepngIHDR *ih, u8 **out_rgba) {
  i32 bpp = ih->color_type == 6 ? 4 : ih->color_type == 2 ? 3 : 1;
  usize stride = (usize)ih->w * (usize)bpp;
  if (raw_len != (usize)ih->h * (stride + 1))
    return DEPNG_ERR_BAD_DATA;
  if ((u64)ih->w * (u64)ih->h > (u64)1 << 28) // 256MP cap before big malloc
    return DEPNG_ERR_UNSUPPORTED;

  u8 *px = (u8 *)malloc((usize)ih->w * ih->h * 4);
  u8 *prev = (u8 *)calloc(stride ? stride : 1, 1);
  u8 *cur = (u8 *)malloc(stride ? stride : 1);
  if (!px || !prev || !cur) {
    free(px);
    free(prev);
    free(cur);
    return DEPNG_ERR_OOM;
  }

  for (u32 y = 0; y < ih->h; y++) {
    const u8 *row = raw + (usize)y * (stride + 1);
    u8 f = row[0];
    if (f > 4) {
      free(px);
      free(prev);
      free(cur);
      return DEPNG_ERR_BAD_DATA;
    }
    for (usize i = 0; i < stride; i++) {
      i32 a = i >= (usize)bpp ? cur[i - bpp] : 0;
      i32 bb = prev[i];
      i32 c = i >= (usize)bpp ? prev[i - bpp] : 0;
      i32 v = row[1 + i];
      if (f == 1)
        v += a;
      else if (f == 2)
        v += bb;
      else if (f == 3)
        v += (a + bb) >> 1;
      else if (f == 4)
        v += depng_paeth(a, bb, c);
      cur[i] = (u8)v;
    }
    u8 *dp = px + (usize)y * ih->w * 4;
    if (ih->color_type == 6) {
      memcpy(dp, cur, stride);
    } else if (ih->color_type == 2) {
      for (u32 x = 0; x < ih->w; x++) {
        dp[x * 4] = cur[x * 3];
        dp[x * 4 + 1] = cur[x * 3 + 1];
        dp[x * 4 + 2] = cur[x * 3 + 2];
        dp[x * 4 + 3] = 255;
      }
    } else { // gray
      for (u32 x = 0; x < ih->w; x++) {
        dp[x * 4] = dp[x * 4 + 1] = dp[x * 4 + 2] = cur[x];
        dp[x * 4 + 3] = 255;
      }
    }
    u8 *tmp = prev;
    prev = cur;
    cur = tmp;
  }

  free(prev);
  free(cur);
  *out_rgba = px;
  return DEPNG_OK;
}

// Full pipeline: chunks -> zlib -> unfilter -> RGBA pixels.
static DEPNG_ERROR depng_decode_rgba(const u8 *buf, usize size, u8 **out_rgba,
                                     u32 *out_w, u32 *out_h) {
  DepngIHDR ih = {0};
  u8 *idat = NULL;
  usize idat_len = 0;
  DEPNG_ERROR err = depng_parse_png(buf, size, &ih, &idat, &idat_len);
  if (err != DEPNG_OK)
    return err;

  i32 bpp = ih.color_type == 6 ? 4 : ih.color_type == 2 ? 3 : 1;
  usize stride = (usize)ih.w * (usize)bpp;
  if ((u64)(stride + 1) * (u64)ih.h > (u64)1 << 32)
    return DEPNG_ERR_UNSUPPORTED;
  usize raw_len = (stride + 1) * (usize)ih.h;

  u8 *raw = NULL;
  usize raw_got = 0;
  err = depng_zlib_decompress(idat, idat_len, &raw, &raw_got, raw_len);
  free(idat);
  if (err != DEPNG_OK)
    return err;

  err = depng_unfilter_to_rgba(raw, raw_got, &ih, out_rgba);
  free(raw);
  if (err != DEPNG_OK)
    return err;

  *out_w = ih.w;
  *out_h = ih.h;
  return DEPNG_OK;
}

static DEPNG_ERROR depng_file_size(FILE *file, i64 *out_size) {
  long size;
  if (fseek(file, 0, SEEK_END) != 0)
    return DEPNG_ERR_IO;
  size = ftell(file);

  if (size < 0)
    return DEPNG_ERR_IO;
  if (fseek(file, 0, SEEK_SET) != 0)
    return DEPNG_ERR_IO;

  *out_size = (i64)size;
  return DEPNG_OK;
}

static DEPNG_ERROR depng_load_file(const char *path, u8 **out, usize *outsize) {
  FILE *file = fopen(path, "rb");
  if (!file)
    return DEPNG_ERR_IO;

  i64 size = 0;
  DEPNG_ERROR err = depng_file_size(file, &size);
  if (err != DEPNG_OK) {
    fclose(file);
    return err;
  }

  *outsize = (usize)size;

  *out = (u8 *)malloc((usize)size);

  if (!(*out)) {
    fclose(file);
    return DEPNG_ERR_OOM;
  }

  if (size > 0 && fread(*out, 1, *outsize, file) != *outsize) {
    free(*out);
    *out = NULL;
    *outsize = 0;
    fclose(file);
    return DEPNG_ERR_IO;
  }

  fclose(file);
  if (!is_png(*out, *outsize)) {
    free(*out);
    *out = NULL;
    *outsize = 0;
    return DEPNG_ERR_NOT_PNG;
  }
  return DEPNG_OK;
}

#endif
