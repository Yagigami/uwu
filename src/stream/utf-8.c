#include <stdbool.h>
#include <stdint.h>

#include "stream/utf-8.h"

bool is_valid_character_utf_8(const uint8_t *strm, long len, uint8_t **endptr) {
	long i = 0;
	if (strm[i] <= 0x7F) {
		if (1 > len) return false;
		i += 1;
	} else if (strm[i] <= 0xBF) {
		return false;
	} else if (strm[i] <= 0xDF) {
		if (2 > len) return false;
		if ((strm[i + 1] & 0xC0) != 0x80) return false;
		i += 2;
	} else if (strm[i] <= 0xEF) {
		if (3 > len) return false;
		if ((strm[i + 1] & 0xC0) != 0x80) return false;
		if ((strm[i + 2] & 0xC0) != 0x80) return false;
		i += 3;
	} else if (strm[i] <= 0xF7) {
		if (4 > len) return false;
		if ((strm[i + 1] & 0xC0) != 0x80) return false;
		if ((strm[i + 2] & 0xC0) != 0x80) return false;
		if ((strm[i + 3] & 0xC0) != 0x80) return false;
		// we have to also check if the char is not in the range U+D800 through U+DFFF
		uint32_t cp = (strm[0] & 0x1F) << 18;
		cp |= (strm[i + 1] & 0x3F) << 12;
		cp |= (strm[i + 2] & 0x3F) <<  6;
		cp |= (strm[i + 3] & 0x3F) <<  0;
		if (cp >= 0xD800 && cp <= 0xDFFF) return false;
		i += 4;
	} else {
		return false;
	}
	strm += i;
	if (endptr) *endptr = (uint8_t *) strm;
	return true;
}

bool is_valid_buffer_utf_8(const uint8_t *strm, long len) {
	for (const uint8_t *s = strm, *end = strm + len; s != end;) {
		if (!is_valid_character_utf_8(s, end - s, &s)) return false;
	}
	return true;
}

// only safe to use if strm has been checked by the functions above
uint32_t codepoint_utf_8(const uint8_t *strm, uint8_t **endptr) {
	uint32_t cp = 0;
	if (strm[0] <= 0x7F) {
		cp = strm[0];
		strm += 1;
	} else if (strm[0] <= 0xBF) {
		__builtin_unreachable();
	} else if (strm[0] <= 0xDF) {
		cp  = (strm[0] & 0x1F) <<  6;
		cp |= (strm[1] & 0x3F) <<  0;
		strm += 2;
	} else if (strm[0] <= 0xEF) {
		cp  = (strm[0] & 0x0F) << 12;
		cp |= (strm[1] & 0x3F) <<  6;
		cp |= (strm[2] & 0x3F) <<  0;
		strm += 3;
	} else if (strm[0] <= 0xF7) {
		cp  = (strm[0] & 0x07) << 18;
		cp |= (strm[1] & 0x3F) << 12;
		cp |= (strm[2] & 0x3F) <<  6;
		cp |= (strm[3] & 0x3F) <<  0;
		if (cp >= 0xD800 && cp <= 0xDFFF)
			__builtin_unreachable();
		strm += 4;
	} else {
		__builtin_unreachable();
	}
	if (endptr) *endptr = (uint8_t *) strm;
	return cp;
}

int encode_utf_8(uint8_t *strm, uint32_t cp, uint8_t **endptr) {
	int ret = len_character_utf_8(cp);
	if        (cp <= 0x00007F) {
		strm[0] = cp;
	} else if (cp <= 0x0007FF) {
		strm[0] = 0xC0 | ((cp >>  6) & 0x1F);
		strm[1] = 0x80 | ((cp >>  0) & 0x3F);
	} else if (cp <= 0x00FFFF) {
		strm[0] = 0xE0 | ((cp >> 12) & 0x0F);
		strm[1] = 0x80 | ((cp >>  6) & 0x3F);
		strm[2] = 0x80 | ((cp >>  0) & 0x3F);
	} else if (cp <= 0x10FFFF) {
		strm[0] = 0xF0 | ((cp >> 18) & 0x07);
		strm[1] = 0x80 | ((cp >> 12) & 0x3F);
		strm[2] = 0x80 | ((cp >>  6) & 0x3F);
		strm[3] = 0x80 | ((cp >>  0) & 0x3F);
	}
	strm += ret;
	if (endptr) *endptr = (uint8_t *) strm;
	return ret;
}

int len_character_utf_8(uint32_t cp) {
	if (cp <= 0x00007F) return 1;
	if (cp <= 0x0007FF) return 2;
	if (cp <= 0x00FFFF) return 3;
	if (cp <= 0x10FFFF) return 4;
	__builtin_unreachable();
}

