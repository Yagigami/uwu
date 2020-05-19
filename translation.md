# Translation steps:

1. preprocessing (pp)
	* [x] trigraph expansion
	* [x] backslash-newlines discarded
	* [x] assert file ends with non-escaped newline
	* [ ] decomposition in pp-tokens
	* [x] comments discarded
	* [x] newlines kept
	* [ ] pp-directives execution
	* [ ] `_Pragma` directives too
	* [ ] `#include` directives recursively do this starting from .1
	* [ ] pp-directives deletion
	* [ ] escape-sequence expansion
	* [ ] string-literal concatenation

2. compilation (uwu)
	* [ ] tokenization
	* [ ] syntactic analysis
	* [ ] parsing
	* [ ] semantic analysis

3. linking (uld)
	* [ ] external object/function reference resolution
	* [ ] other library object/functions linking
	* [ ] creation of an executable image

4. execution
	* [ ] static initialization before any call to a C-function
	* [ ] in `int main(int argc, char **argv) { /* ... */ }`:
		* argc > 0
		* argv[argc] == NULL
		* if (argc > 0) argv[0 ... argc-1] != NULL; else argv[0][0] == '\0'
		* typeof(**argv) != const char
	* [ ] in `int main() { /* ... */ return x; }` is equivalent to `int main() { /* ... */ exit(x); }`

