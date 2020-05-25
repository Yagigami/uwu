#ifndef C_INTERN_INTERN_H
#define C_INTERN_INTERN_H

#include <stdint.h>
#include <stddef.h>

struct Interns {
	ptrdiff_t len, cap;
	struct InternString *interns;
};

struct InternString {
	ptrdiff_t len;
	uint8_t *str;
};

int intern_init(struct Interns *interns);
void intern_fini(struct Interns *interns);
const struct InternString *intern_string(struct Interns *interns, const uint8_t *str, ptrdiff_t len);
const struct InternString *intern_find(const struct Interns *interns, const uint8_t *str, ptrdiff_t len);
bool intern_contains(const struct Interns *interns, const uint8_t *str, long ptrdiff_t);

int print_interns(const struct Interns *interns);
int print_intern(const struct InternString *intern);

const struct Interns *get_keyword_interns(void);

#endif /* C_INTERN_INTERN_H */

