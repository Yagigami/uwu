#ifndef C_INTERN_INTERN_H
#define C_INTERN_INTERN_H

struct Interns {
	struct InternString *interns;
	long len, cap;
};

struct InternString {
	char *str;
	long len;
};

int intern_init(struct Interns *interns);
void intern_fini(struct Interns *interns);
const struct InternString *intern_string(struct Interns *interns, const char *str, long len);
const struct InternString *intern_find(const struct Interns *interns, const char *str, long len);
bool intern_contains(const struct Interns *interns, const char *str, long len);

int print_interns(const struct Interns *interns);
int print_intern(const struct InternString *intern);

const struct Interns *get_keyword_interns(void);

#endif /* C_INTERN_INTERN_H */

