#ifndef C_STREAM_UTF_8_H
#define C_STREAM_UTF_8_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool is_valid_character_utf_8(const uint8_t *strm, long len, uint8_t **endptr);
bool is_valid_buffer_utf_8(const uint8_t *strm, long len);

// only safe to use if strm has been checked by the functions above
uint32_t codepoint_utf_8(const uint8_t *strm, uint8_t **endptr);

ptrdiff_t encode_utf_8_len(const uint32_t *src, uint32_t **endptr);

// only safe if `num` was given by `encode_utf_8_len`
ptrdiff_t encode_utf_8(uint8_t *dst, const uint32_t *src, ptrdiff_t num);

int len_character_utf_8(uint32_t cp);

#endif /* C_STREAM_UTF_8_H */

