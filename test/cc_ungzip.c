/* cc_ungzip.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cc_inflate.h"

static int do_process(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	int code;

	code = do_process(argc, argv);

	return code;
}

int do_process(int argc, char* argv[])
{
	FILE* fp  = NULL;
	FILE* out = NULL;

	cci_context_t* context = NULL;

	int i, code = -1, gzip = 1, v = 0;

	unsigned char buf[0x10000];

	for (i = 1; i < argc; i++) {
		char* arg = argv[i];

		if (strcmp(arg, "-gzip") == 0) {
			gzip = 1;

		} else if (strcmp(arg, "-zlib") == 0) {
			gzip = 0;

		} else if (strcmp(arg, "-v") == 0) {
			v = 1;

		} else {
			if (fp == NULL) {
				fp = fopen(arg, "rb");
				if (fp == NULL) {
					return -1;
				}

			} else if (out == NULL) {
				out = fopen(arg, "wb");
				if (out == NULL) {
					return -1;
				}
			}
		}
	}

	if (fp == NULL) {
		return -1;
	}

	context = cci_create_context(NULL);
	if (context != NULL) {
		for (; ; ) {
			if (gzip) {
				code = cci_decode_gzip(context);
			} else {
				code = cci_decode_zlib(context);
			}

			if (code == CCI_R_INPUT) {
				size_t cb = fread(buf, 1, sizeof(buf), fp);
				if (cb == 0) {
					puts("[NG] unexpected EOF.");
					code = 2;
					break;
				}

				if (v) {
					printf("INP: %d\n", cb);
				}

				cci_setup_input(context, buf, cb);

			} else if (code == CCI_R_OUTPUT || code == CCI_R_FINAL) {
				size_t cb = 0;
				const void* pv = cci_fetch_output(context, &cb);
				if (cb > 0 && out != NULL) {
					fwrite(pv, 1, cb, out);
				}

				if (v) {
					printf("OUT: %d\n", cb);
				}

				if (code == CCI_R_FINAL) {
					puts("[OK] CCI_R_FINAL");
					code = 0;
					break;
				}

			} else {
				puts("[NG] Error.");
				code = 1;
				break;
			}
		}
	}

	cci_release_context(context);

	fclose(fp);

	if (out != NULL) {
		fclose(out);
	}

	return code;
}

