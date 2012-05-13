/* cc_zlib.c */

#include "cc_inflate.h"
#include "cc_inflate_private.h"

/* */

#define A32_MOD 65521
#define A32_MAX  5552

static INT32_T adler32(const BYTE_T* p, const BYTE_T* end, INT32_T s)
{
	unsigned int s1 =  s        & 0xffff;
	unsigned int s2 = (s >> 16) & 0xffff;

	while (p < end) {
		const BYTE_T* e = p + A32_MAX;
		if (e > end) {
			e = end;
		}

		while (p < e) {
			s1 += *(p++);
			s2 += s1;
		}

		s1 %= A32_MOD;
		s2 %= A32_MOD;
	}

	return (s2 << 16) | s1;
}

/* */

#define GET_BYTE(L, V)												\
	case L:															\
		t->container = L;											\
		if (t->bs >= t->be) {										\
			return CCI_R_INPUT;										\
		}															\
		(V) = *(t->bs++);

/* */

int cci_decode_zlib(cci_context_t* t)
{
	switch (t->container) {
	case HEADER:

	for (t->index = 0; t->index < 2; t->index++) {
		GET_BYTE(Z_HEADER, t->header[t->index])
	}

	if ((((t->header[0] << 8) | t->header[1]) % 31) != 0) {
		return CCI_R_ERROR;
	}

	if ((t->header[0] & 0xf) != 8) {
		return CCI_R_ERROR;
	}

	if ((t->header[0] >> 4) > 7) {
		return CCI_R_ERROR;
	}

	if ((t->header[1] & 0x20) != 0) {
		return CCI_R_ERROR;
	}

	t->container = Z_INFLATE;
	t->crc       = 1;

	/* */

	case Z_INFLATE:
	{
		int code = cci_inflate(t);
		if (code == CCI_R_ERROR) {
			return CCI_R_ERROR;
		}

		if (code == CCI_R_OUTPUT || code == CCI_R_FINAL) {
			t->crc          = adler32(t->window, t->os, t->crc);
			t->uncomp_size += (t->os - t->window);
		}

		if (code != CCI_R_FINAL) {
			return code;
		}
	}

	/* */

	for (t->index = 0; t->index < 4; t->index++) {
		GET_BYTE(Z_CHECK, t->header[2 + t->index])
	}

	if (t->crc != ((t->header[2] << 24) | (t->header[3] << 16) | (t->header[4] << 8) | t->header[5])) {
		return CCI_R_ERROR;
	}

	break;

	/* */

	default:
		return CCI_R_ERROR;

	} /* switch (t->container) */

	return CCI_R_FINAL;
}

/* */

