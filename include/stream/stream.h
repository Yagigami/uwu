#ifndef C_STREAM_STREAM_H
#define C_STREAM_STREAM_H

typedef struct Stream *Stream;

Stream __uwuin(void);
Stream __uwuout(void);
Stream __uwuerr(void);
Stream __uwunull(void);

#define C_STREAM_READ (1<<0)
#define C_STREAM_WRITE (1<<1)
#define C_STREAM_TEXT (1<<2)
#define C_STREAM_BINARY (1<<3)
#define C_STREAM_UTF_8 (1<<4)

#define uwuin  __uwuin ()
#define uwuout __uwuout()
#define uwuerr __uwuerr()
#define uwunull __uwunull()

#include <stddef.h>

int stream_test(void);

Stream stream_init(const char *title, int mode);
void stream_fini(Stream stream);

const char *stream_name(Stream stream, ptrdiff_t *len);
const char *stream_basename(Stream stream, ptrdiff_t *len);
const char *stream_extension(Stream stream, ptrdiff_t *len);
ptrdiff_t stream_size(Stream stream);

ptrdiff_t stream_read(Stream stream, void *buf, ptrdiff_t size);
ptrdiff_t stream_write(Stream stream, const void *buf, ptrdiff_t size);
ptrdiff_t stream_encode(Stream stream, void *dst, const void *src, ptrdiff_t num);
ptrdiff_t stream_encode_len(Stream stream, const void *src, void **endptr);
// obsolete for now
char *stream_getline(Stream stream, long *len);

#endif /* C_STREAM_STREAM_H */

