#ifndef C_STREAM_STREAM_H
#define C_STREAM_STREAM_H

typedef struct Stream *Stream;

#define C_STREAM_READ (1<<0)
#define C_STREAM_WRITE (1<<1)
#define C_STREAM_TEXT (1<<2)
#define C_STREAM_BINARY (1<<3)
#define C_STREAM_UTF_8 (1<<4)

int stream_test(void);

Stream stream_init(const char *title, int mode);
void stream_fini(Stream stream);

const char *stream_name(Stream stream, long *len);
const char *stream_basename(Stream stream, long *len);
const char *stream_extension(Stream stream, long *len);
long stream_size(Stream stream);

long stream_read(Stream stream, void *buf, long size);
long stream_write(Stream stream, const void *buf, long size);
char *stream_getline(Stream stream, long *len);

#endif /* C_STREAM_STREAM_H */

