#include "common/log.h"

#include <stream/stream.h>

#include <stdarg.h>
#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define MIN(a, b) ((a) < (b) ? (a): (b))
#define MAX(a, b) ((a) > (b) ? (a): (b))
#define MINUS_SIGN '-'
#define PLUS_SIGN '+'
#define ZERO '0'
#define INVALID '@'

#ifndef NDEBUG
#define SPACE '_'
#else
#define SPACE ' '
#endif

struct uwuprintfInfo {
	ptrdiff_t width;
	ptrdiff_t precision;
	bool left:  1;
	bool plus:  1;
	bool space: 1;
	bool alt:   1;
	bool zero:  1;
	bool caps:  1;
	bool selp:  1;
	bool ptr:   1;
	enum {
		LENGTH_NONE,
		LENGTH_HH,
		LENGTH_H,
		LENGTH_L,
		LENGTH_LL,
		LENGTH_J,
		LENGTH_Z,
		LENGTH_T,
		LENGTH_CAP_L,
		LENGTH_P,
		LENGTH_MAX = LENGTH_P,
	} length: 4;
	enum {
		STAGE_FLAGS,
		STAGE_WIDTH,
		STAGE_PREC,
		STAGE_LENGTH,
		STAGE_CVT,
		STAGE_END,
	} stage:  3;
	enum {
		BASE_DEC = 10,
		BASE_OCT =  8,
		BASE_HEX = 16,
	} base: 5;
};

long uwuprintf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	long prn = uwuvfprintf(uwuout, fmt, args);
	va_end(args);
	return prn;
}

long uwufprintf(Stream stream, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	long prn = uwuvfprintf(stream, fmt, args);
	va_end(args);
	return prn;
}

static long _uwuvfprintf_c(Stream stream, struct uwuprintfInfo info, va_list args) {
	uint8_t locbuf[64], encbuf[4];
	bool alloc = info.width > (ptrdiff_t) sizeof locbuf;
	uint32_t c = info.length == LENGTH_L ? va_arg(args, uint32_t): va_arg(args, int);
	ptrdiff_t len = stream_encode(stream, encbuf, &c, 1);
	ptrdiff_t size = MAX(len, info.width);
	uint8_t *buf = alloc ? malloc(info.width): locbuf;
	if (!buf) return -1;
	ptrdiff_t pad = size - len;
	void *out_pad = info.left ? buf + len: buf;
	void *out_enc = info.left ? buf: buf + pad;
	memset(out_pad, SPACE, pad);
	memcpy(out_enc, encbuf, len);
	size = stream_write(stream, buf, size);
	if (alloc) free(buf);
	return size;
}

static long _uwuvfprintf_s(Stream stream, struct uwuprintfInfo info, va_list args) {
	uint8_t locbuf[64];
	ptrdiff_t size = -1, len, pad, elen;
	bool alloc, wide = info.length == LENGTH_L;
	const void *s;
	if (wide) {
		s = va_arg(args, const uint32_t *);
		void *end;
		elen = stream_encode_len(stream, s, &end);
	} else {
		s = va_arg(args, const char *);
		elen = strlen(s);
	}
	if (!info.selp) info.precision = PTRDIFF_MAX;
	len = MIN(elen, info.precision);
	size = MAX(len, info.width);
	pad = size - len;
	alloc = size > (ptrdiff_t) sizeof locbuf;
	uint8_t *buf = alloc ? malloc(size): locbuf;
	if (!buf) return -1;
	void *out_pad = info.left ? buf + len: buf;
	void *out_s = info.left ? buf: buf + pad;
	memset(out_pad, SPACE, pad);
	if (wide) {
		stream_encode(stream, out_s, s, len);
	} else {
		memcpy(buf, s, len);
	}
	size = stream_write(stream, buf, size);
	if (alloc) free(buf);
	return size;
}

static intmax_t _arg_signed(struct uwuprintfInfo info, va_list args) {
	switch (info.length) {
	case LENGTH_L:
		return va_arg(args, long);
	case LENGTH_LL:
		return va_arg(args, long long);
	case LENGTH_J:
		return va_arg(args, intmax_t);
	case LENGTH_Z: // ssize_t nonstandard
	case LENGTH_T:
		return va_arg(args, ptrdiff_t);
	case LENGTH_P:
		return va_arg(args, intptr_t);
	case LENGTH_H: // promoted
	case LENGTH_HH: // promoted
	case LENGTH_CAP_L: // ignore
	case LENGTH_NONE: // default
	default:
		return va_arg(args, int);
	}
}

static uintmax_t _arg_unsigned(struct uwuprintfInfo info, va_list args) {
	switch (info.length) {
	case LENGTH_L:
		return va_arg(args, unsigned long);
	case LENGTH_LL:
		return va_arg(args, unsigned long long);
	case LENGTH_J:
		return va_arg(args, uintmax_t);
	case LENGTH_Z:
	case LENGTH_T:
		return va_arg(args, size_t);
	case LENGTH_P:
		return va_arg(args, uintptr_t);
	case LENGTH_H:
	case LENGTH_HH:
	case LENGTH_CAP_L:
	case LENGTH_NONE:
	default:
		return va_arg(args, unsigned);
	}
}

static int hex2char(int i, struct uwuprintfInfo info) {
	if (i < 10) return i + '0';
	if (info.caps) return i + 'A' - 0xA;
	return i + 'a' - 0xa;
}

static ptrdiff_t _transform_integer(char *buf, uintmax_t u, struct uwuprintfInfo info) {
	ptrdiff_t len = 0, prec = 0;
	do {
		int digit = u % info.base;
		buf[len++] = hex2char(digit, info);
		u /= info.base;
		prec++;
	} while (u || prec < info.precision);
	return len;
}

static long _uwuvfprintf_d(Stream stream, struct uwuprintfInfo info, va_list args) {
	char locbuf[32], finbuf[32];
	intmax_t i = _arg_signed(info, args);
	int sign = 0;
	if (i < 0) {
		sign = MINUS_SIGN;
		i = -i;
	} else if (info.plus) {
		sign = PLUS_SIGN;
	} else if (info.space) {
		sign = SPACE;
	}
	ptrdiff_t len = _transform_integer(locbuf, i, info);
	ptrdiff_t size = MAX(len, info.width); // the width spec doesn't increase size
	ptrdiff_t pad = size - len;
	bool alloc = size > (ptrdiff_t) sizeof finbuf;
	char *buf = alloc ? malloc(size): finbuf;
	if (!buf) return -1;
#ifndef NDEBUG
	memset(buf, INVALID, size);
#endif
	if (sign) pad--;
	char *insert = info.left ? buf: buf + pad;
	char *out_pad = info.left ? buf + len: buf;
	// doesn't work if (info.zero && info.left) but don't really care, is invalid
	if (sign) {
		if (info.zero)
			*out_pad++ = sign;
		else {
			*insert = sign;
			if (info.left) out_pad++;
		}
		insert++;
	}
	char fill = info.zero ? ZERO: SPACE;
	memset(out_pad, fill, pad);
	// put number in the right order
	for (ptrdiff_t idx = 0, symm = len - 1; idx < len; idx++, symm--)
		insert[idx] = locbuf[symm];
	size = stream_write(stream, buf, size);
	if (alloc) free(buf);
	return size;
}

static ptrdiff_t _uwuvfprintf_unsigned(Stream stream, struct uwuprintfInfo info, va_list args) {
	char locbuf[32], finbuf[32];
	uintmax_t u = _arg_unsigned(info, args);
	ptrdiff_t len = _transform_integer(locbuf, u, info);
	if (info.ptr && !u)
		strcpy(locbuf, "(nil)");
	if (info.alt && u) switch (info.base) {
	case BASE_DEC:
		break;
	case BASE_OCT:
		if (locbuf[len-1] != '0')
			locbuf[len++] = '0';
		break;
	case BASE_HEX:
		locbuf[len++] = info.caps ? 'X': 'x';
		locbuf[len++] = '0';
		break;
	default:
		__builtin_unreachable();
	}
	ptrdiff_t size = MAX(len, info.width);
	ptrdiff_t pad = size - len;
	bool alloc = size > (ptrdiff_t) sizeof finbuf;
	char *buf = alloc ? malloc(size): finbuf;
	if (!buf) return -1;
#ifndef NDEBUG
	memset(buf, INVALID, size);
#endif
	char *insert = info.left ? buf: buf + pad;
	char *out_pad = info.left ? buf + len: buf;
	char fill = info.zero ? ZERO: SPACE;
	memset(out_pad, fill, pad);
	for (ptrdiff_t idx = 0, symm = len - 1; idx < len; idx++, symm--)
		insert[idx] = locbuf[symm];
	size = stream_write(stream, buf, size);
	if (alloc) free(buf);
	return size;
}

static long _uwuvfprintf_f(Stream stream, struct uwuprintfInfo info, va_list args) {
	char locbuf[64];
	if (info.length == LENGTH_CAP_L) return -1;
	double x = va_arg(args, double);
	if (!info.precision && !info.selp) info.precision = 6;
	ptrdiff_t l10 = ceil(log10(x));
	ptrdiff_t ilen = MAX(l10, 1);
	ptrdiff_t flen = info.precision;
	// the radix point is only drawn if precison > 0
	if (info.precision || info.alt) flen++;
	ptrdiff_t len = ilen + flen;
	int sign = 0;
	if (signbit(x)) {
		sign = MINUS_SIGN;
	} else if (info.plus) {
		sign = PLUS_SIGN;
	} else if (info.space) {
		sign = SPACE;
	}
	ptrdiff_t size = MAX(len, info.width);
	ptrdiff_t pad = size - len;
	bool alloc = size > (ptrdiff_t) sizeof locbuf;
	char *buf = alloc ? malloc(size): locbuf;
	if (!buf) return -1;
#ifndef NDEBUG
	memset(buf, INVALID, size);
#endif
	char *insert = info.left ? buf: buf + pad;
	char *out_pad = info.left ? buf + len: buf;
	if (sign) {
		if (info.zero)
			*out_pad++ = sign;
		else {
			*insert = sign;
			if (info.left) out_pad++;
		}
		insert++;
	}
	char fill = info.zero ? ZERO: SPACE;
	memset(out_pad, fill, pad);
	char *iinsert = insert;
	char *finsert = iinsert + ilen;
	double i, f = modf(x, &i), base = 10.0;
	for (ptrdiff_t idx = ilen - 1; idx >= 0; idx--) {
		int digit = fmod(i, base);
		if (digit >= 10) __builtin_unreachable();
		iinsert[idx] = hex2char(digit, info);
		i /= base;
	}
	if (flen) *finsert = '.';
	for (ptrdiff_t idx = 1; idx < flen; idx++) {
		f *= 10.0;
		int digit = f;
		finsert[idx] = hex2char(digit, info);
		f -= digit;
	}
	size = stream_write(stream, buf, size);
	if (alloc) free(buf);
	return size;
}

static long _uwuvfprintf_e(Stream stream, struct uwuprintfInfo info, va_list args) {
	(void) stream, (void) info, (void) args;
	return -1;
}

static long _uwuvfprintf_a(Stream stream, struct uwuprintfInfo info, va_list args) {
	(void) stream, (void) info, (void) args;
	return -1;
}

static long _uwuvfprintf_g(Stream stream, struct uwuprintfInfo info, va_list args) {
	(void) stream, (void) info, (void) args;
	return -1;
}

static long _uwuvfprintf_n(Stream stream, struct uwuprintfInfo info, va_list args, ptrdiff_t pos) {
	(void) stream;
	switch (info.length) {
	case LENGTH_HH:
		*va_arg(args, signed char *) = (signed char) pos;
		break;
	case LENGTH_H:
		*va_arg(args, short *) = (short) pos;
		break;
	case LENGTH_CAP_L: // ignore
	default:
	case LENGTH_NONE:
		*va_arg(args, int *) = (int) pos;
		break;
	case LENGTH_L:
		*va_arg(args, long *) = (long) pos;
		break;
	case LENGTH_LL:
		*va_arg(args, long long *) = (long long) pos;
		break;
	case LENGTH_J:
		*va_arg(args, intmax_t *) = (intmax_t) pos;
		break;
	case LENGTH_Z:
	case LENGTH_T:
		*va_arg(args, ptrdiff_t *) = (ptrdiff_t) pos;
		break;
	case LENGTH_P:
		*va_arg(args, intptr_t *) = (intptr_t) pos;
		break;
	}
	return 0;
}

long uwuvfprintf(Stream stream, const char *fmt, va_list args) {
	ptrdiff_t prn = 0, w;
	const char *cur = fmt, *last = cur;
	while (*cur) {
		if (*cur++ != '%') continue;
		w = cur - 1 - last;
		if (stream_write(stream, last, w) != w) {
			return -1;
		}
		prn += w;
		struct uwuprintfInfo info = { 0 };
		for (; info.stage < STAGE_END; cur++) switch (*cur) {
			ptrdiff_t n;
		case '-':
			if (info.left || info.stage > STAGE_FLAGS) break;
			info.left = true;
			break;
		case '+':
			if (info.plus || info.space || info.stage > STAGE_FLAGS) break;
			info.plus = true;
			break;
		case ' ':
			if (info.plus || info.space || info.stage > STAGE_FLAGS) break;
			info.space = true;
			break;
		case '#':
			if (info.alt || info.stage > STAGE_FLAGS) break;
			info.alt = true;
			break;
		case '0':
			if (info.zero || info.stage > STAGE_FLAGS) break;
			info.zero = true;
			break;
		case '1' ... '9':
			if (info.stage > STAGE_PREC) break;
			if (info.stage < STAGE_WIDTH) info.stage = STAGE_WIDTH;
			for (n = *cur - '0'; isdigit(cur[1]); cur++)
				n = n * 10 + cur[1] - '0';
			info.selp ? info.precision = n: (info.width = n);
			break;
		case '*':
			if (info.stage > STAGE_PREC) break;
			info.stage++;
			// int gets troublesome uwu
			n = va_arg(args, ptrdiff_t);
			if (n < 0) n = 0;
			info.selp ? info.precision = n: (info.width = n);
			break;
		case '.':
			if (info.selp || info.stage > STAGE_WIDTH) break;
			info.selp = true;
			break;
		case 'h':
			if (info.length || info.stage > STAGE_LENGTH) break;
			info.length = LENGTH_H;
			if (cur[1] == 'h') {
				cur++;
				info.length = LENGTH_HH;
			}
			info.stage = STAGE_CVT;
			break;
		case 'l':
			if (info.length || info.stage > STAGE_LENGTH) break;
			info.length = LENGTH_L;
			if (cur[1] == 'l') {
				cur++;
				info.length = LENGTH_LL;
			}
			info.stage = STAGE_CVT;
			break;
		case 'j':
			if (info.length || info.stage > STAGE_LENGTH) break;
			info.length = LENGTH_J;
			info.stage = STAGE_CVT;
			break;
		case 'z':
			if (info.length || info.stage > STAGE_LENGTH) break;
			info.length = LENGTH_Z;
			info.stage = STAGE_CVT;
			break;
		case 't':
			if (info.length || info.stage > STAGE_LENGTH) break;
			info.length = LENGTH_T;
			info.stage = STAGE_CVT;
			break;
		case 'L':
			if (info.length || info.stage > STAGE_LENGTH) break;
			info.length = LENGTH_CAP_L;
			info.stage = STAGE_CVT;
			break;
		case '%':
			if (cur[-1] != '%') break;
			break;
		case 'c':
			info.stage = STAGE_END;
			w = _uwuvfprintf_c(stream, info, args);
			break;
		case 's':
			info.stage = STAGE_END;
			w = _uwuvfprintf_s(stream, info, args);
			break;
		case 'd':
			info.stage = STAGE_END;
			info.base = BASE_DEC;
			w = _uwuvfprintf_d(stream, info, args);
			break;
		case 'i':
			info.stage = STAGE_END;
			info.base = BASE_DEC;
			w = _uwuvfprintf_d(stream, info, args);
			break;
		case 'o':
			info.stage = STAGE_END;
			info.base = BASE_OCT;
			w = _uwuvfprintf_unsigned(stream, info, args);
			break;
		case 'x':
			info.stage = STAGE_END;
			info.base = BASE_HEX;
			w = _uwuvfprintf_unsigned(stream, info, args);
			break;
		case 'X':
			info.stage = STAGE_END;
			info.base = BASE_HEX;
			info.caps = true;
			w = _uwuvfprintf_unsigned(stream, info, args);
			break;
		case 'u':
			info.stage = STAGE_END;
			info.base = BASE_DEC;
			w = _uwuvfprintf_unsigned(stream, info, args);
			break;
		case 'f':
			info.stage = STAGE_END;
			info.base = BASE_DEC;
			w = _uwuvfprintf_f(stream, info, args);
			break;
		case 'F':
			info.stage = STAGE_END;
			info.base = BASE_DEC;
			info.caps = true;
			w = _uwuvfprintf_f(stream, info, args);
			break;
		case 'e':
			info.stage = STAGE_END;
			info.base = BASE_DEC;
			w = _uwuvfprintf_e(stream, info, args);
			break;
		case 'E':
			info.stage = STAGE_END;
			info.base = BASE_DEC;
			info.caps = true;
			w = _uwuvfprintf_e(stream, info, args);
			break;
		case 'a':
			info.stage = STAGE_END;
			info.base = BASE_HEX;
			w = _uwuvfprintf_a(stream, info, args);
			break;
		case 'A':
			info.stage = STAGE_END;
			info.base = BASE_HEX;
			info.caps = true;
			w = _uwuvfprintf_a(stream, info, args);
			break;
		case 'g':
			info.stage = STAGE_END;
			info.base = BASE_DEC;
			w = _uwuvfprintf_g(stream, info, args);
			break;
		case 'G':
			info.stage = STAGE_END;
			info.base = BASE_DEC;
			info.caps = true;
			w = _uwuvfprintf_g(stream, info, args);
			break;
		case 'n':
			info.stage = STAGE_END;
			w = _uwuvfprintf_n(stream, info, args, prn);
			break;
		case 'p':
			info.stage = STAGE_END;
			info.base = BASE_HEX;
			info.length = LENGTH_P;
			info.ptr = true;
			info.alt = true;
			w = _uwuvfprintf_unsigned(stream, info, args);
			break;
		default:
			goto fmt_end;
		}
	fmt_end:
		if (w == -1) return -1;
		prn += w;
		last = cur;
	}
	w = cur - last;
	w = stream_write(stream, last, w);
	if (w == -1) return -1;
	return prn + w;
}

#if 0
long uwusprintf(char *buf, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	long prn = uwuvsprintf(buf, fmt, args);
	va_end(args);
	return prn;
}

long uwusnprintf(char *buf, long n, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	long prn = uwuvsnprintf(buf, n, fmt, args);
	va_end(args);
	return prn;
}

long uwuvsprintf(char *buf, const char *fmt, va_list args) {
	return uwuvsnprintf(buf, LONG_MAX, fmt, args);
}

long uwuvsnprintf(char *buf, long n, const char *fmt, va_list args) {
	(void) buf, (void) n, (void) fmt, (void) args;
	return 0;
}
#endif

