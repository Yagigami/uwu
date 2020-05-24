#include <stdbool.h>
#include <stdint.h>

#include "stream/utf-8.h"

static int cp_tp_utf_8(uint8_t *strm, uint32_t cp, uint8_t **endptr);

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
		uint8_t *out;
		if (!is_valid_character_utf_8(s, end - s, &out)) return false;
		s = out;
	}
	return true;
}

// only safe to use if strm has been checked by the functions above
uint32_t codepoint_utf_8(const uint8_t *strm, uint8_t **endptr) {
	uint32_t cp = 0;
	if (strm[0] <= 0x7F) {
		cp = strm[0];
		strm += 1;
	} else if (!(strm[0] <= 0xBF) && strm[0] <= 0xDF) {
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
	}
	if (endptr) *endptr = (uint8_t *) strm;
	return cp;
}

int cp_tp_utf_8(uint8_t *strm, uint32_t cp, uint8_t **endptr) {
	int len = 0;
	unsigned or, msk, shamt;
	if      (cp <= 0x00007F)
		len = 1, or = 0x00, msk = 0xFF;
	else if (cp <= 0x0007FF)
		len = 2, or = 0xC0, msk = 0x1F;
	else if (cp <= 0x00FFFF)
		len = 3, or = 0xE0, msk = 0x0F;
	else if (cp <= 0x10FFFF)
		len = 4, or = 0xF0, msk = 0x07;
	else
		goto end;
	shamt = (len - 1) * 6;
	switch (len) {
	case 4:
		*strm++ = or | ((cp >> shamt) & msk);
		shamt -= 6, or = 0x80, msk = 0x3F;
		/* fallthrough */
	case 3:
		*strm++ = or | ((cp >> shamt) & msk);
		shamt -= 6, or = 0x80, msk = 0x3F;
		/* fallthrough */
	case 2:
		*strm++ = or | ((cp >> shamt) & msk);
		shamt -= 6, or = 0x80, msk = 0x3F;
		/* fallthrough */
	case 1:
		*strm++ = or | ((cp >> shamt) & msk);
		shamt -= 6, or = 0x80, msk = 0x3F;
		/* fallthrough */
	}
end:
	if (endptr) *endptr = (uint8_t *) strm;
	return len;
}

int len_character_utf_8(uint32_t cp) {
	if (cp <= 0x00007F) return 1;
	if (cp <= 0x0007FF) return 2;
	if (cp <= 0x00FFFF) return 3;
	if (cp <= 0x10FFFF) return 4;
	return -1;
}

ptrdiff_t encode_utf_8_len(const uint32_t *src, uint32_t **endptr) {
	const uint32_t *cur;
	ptrdiff_t len = 0;
	for (cur = src; *cur; cur++) {
		int l = len_character_utf_8(*cur);
		if (l == -1) return -1;
		len += l;
	}
	if (endptr) *endptr = (uint32_t *) cur;
	return len;
}

ptrdiff_t encode_utf_8(uint8_t *dst, const uint32_t *src, ptrdiff_t num) {
	const uint32_t *cur = src;
	ptrdiff_t len = 0;
	for (uint8_t *w = dst; len < num && *cur; cur++) {
		uint8_t *out;
		ptrdiff_t l = cp_tp_utf_8(w, *cur, &out);
		if (out == w) return -1;
		w = out;
		len += l;
	}
	return len;
}

