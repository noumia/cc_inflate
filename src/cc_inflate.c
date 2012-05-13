/* cc_inflate.c */

#include <malloc.h>
#include <memory.h>

#include "cc_inflate.h"
#include "cc_inflate_private.h"

/* */

static void* cc_malloc(cci_env_t* env, size_t sz)
{
	return malloc(sz);
}

static void cc_free(cci_env_t* env, void* p)
{
	free(p);
}

static cci_env_t cc_env = {
	cc_malloc,
	cc_free
};

/* */

cci_context_t* cci_create_context(cci_env_t* env)
{
	cci_context_t* t;

	cci_env_t* e;

	BYTE_T* window;

	e = env;
	if (e == NULL) {
		e = &cc_env;
	}

	t = (cci_context_t*) (e->cmalloc)(e, sizeof(cci_context_t));
	if (t == NULL) {
		return NULL;
	}

	window = (BYTE_T*) (e->cmalloc)(e, CCI_WINDOW_SIZE);
	if (window == NULL) {
		(e->cfree)(e, t);
		return NULL;
	}

	memset(t, 0, sizeof(cci_context_t));

	t->env = e;

	t->window  = window;
	t->os      = window;
	t->oe      = window + CCI_WINDOW_SIZE;

	t->container = HEADER;
	t->state     = BLOCK;

	return t;
}

void cci_release_context(cci_context_t* t)
{
	if (t != NULL) {
		cci_env_t* e = t->env;

		(e->cfree)(e, t->window);
		(e->cfree)(e, t);
	}
}

/* */

void cci_setup_input(cci_context_t* t, const void* input, size_t size)
{
	t->stream = input;

	t->bs = (const BYTE_T*) input;
	t->be = t->bs + size;

	t->input_size += size;
}

const void* cci_fetch_output(cci_context_t* t, size_t* size)
{
	size_t sz = t->os - t->window;

	t->os = t->window;

	t->output_size += sz;

	*size = sz;

	return t->window;
}

/* */

static void build_tree(INT32_T count[16], INT16_T lookup[], const BYTE_T length[], INT32_T symbols)
{
	INT32_T i;
	INT16_T offset[16];

	for (i = 0; i < 16; i++) {
		count[i] = 0;
	}

	for (i = 0; i < symbols; i++) {
		count[length[i]]++;
	}

	offset[1] = 0;

	for (i = 2; i < 16; i++) {
		offset[i] = offset[i - 1] + count[i - 1];
	}

	for (i = 0; i < symbols; i++) {
		lookup[i] = -1;
	}

	for (i = 0; i < symbols; i++) {
		if (length[i] > 0) {
			lookup[offset[length[i]]++] = (INT16_T) i;
		}
	}
}

/* RFC 1951 */

/* 3.2.6. */

static const INT32_T T_LENGTH_COUNT[16] = {
	0, 0, 0, 0, 0, 0, 0, 24, 152, 112, 0, 0, 0, 0, 0, 0
};

static const INT16_T T_LENGTH_LOOKUP[288] = {
	256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271,
	272, 273, 274, 275, 276, 277, 278, 279,   0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143, 280, 281, 282, 283, 284, 285, 286, 287,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

static const INT32_T T_DISTANCE_COUNT[16] = {
	0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const INT16_T T_DISTANCE_LOOKUP[32] = {
	  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31
};

/* 3.2.5. */

static const INT32_T T_LENGTH_ADDS[29] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
	2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};

static const INT32_T T_LENGTH_BASE[29] = {
	0x003, 0x004, 0x005, 0x006, 0x007, 0x008, 0x009, 0x00a, 0x00b, 0x00d, 0x00f, 0x011, 0x013, 0x017, 0x01b,
	0x01f, 0x023, 0x02b, 0x033, 0x03b, 0x043, 0x053, 0x063, 0x073, 0x083, 0x0a3, 0x0c3, 0x0e3, 0x102
};

static const INT32_T T_DISTANCE_ADDS[30] = {
	0,  0,  0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,
	6,  7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13
};

static const INT32_T T_DISTANCE_BASE[30] = {
	0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0007, 0x0009, 0x000d, 0x0011, 0x0019, 0x0021, 0x0031, 0x0041, 0x0061, 0x0081,
	0x00c1, 0x0101, 0x0181, 0x0201, 0x0301, 0x0401, 0x0601, 0x0801, 0x0c01, 0x1001, 0x1801, 0x2001, 0x3001, 0x4001, 0x6001
};

/* */

#define GET_BITS(L, V, C, O)									\
	case L:														\
		t->state = L;											\
		for (; ; ) {											\
			if (t->has >= (C)) {								\
				(V)      = (t->bit & ((1 << (C)) - 1)) + (O);	\
				t->bit >>= (C);									\
				t->has  -= (C);									\
				break;											\
			}													\
			if (t->bs >= t->be) {								\
				return CCI_R_INPUT;								\
			} else {											\
				t->bit |= (*(t->bs++) << t->has);				\
				t->has += 8;									\
			}													\
		}

#define GET_SYMBOL(L, HC, HL)									\
	t->huff   = (HC); 											\
	t->hend   = (HC) + 16;										\
	t->value  = 0;												\
	t->lookup = (HL);											\
	do {														\
		if (++t->huff >= t->hend) {								\
			return CCI_R_ERROR;									\
		}														\
		GET_BITS(L, t->symbol, 1, 0)							\
		t->value   = ((t->value << 1) | t->symbol) - t->huff[0];\
		t->lookup += t->huff[0];								\
	} while (t->value >= 0);									\
	t->symbol = t->lookup[t->value];

#define GET_BYTE(L, V)											\
	case L:														\
		t->state = L;											\
		if (t->bs >= t->be) {									\
			return CCI_R_INPUT;									\
		}														\
		(V) = *(t->bs++);

#define PUT_BYTE(L, V)											\
	case L:														\
		t->state = L;											\
		if (t->os >= t->oe) {									\
			return CCI_R_OUTPUT;								\
		}														\
		*(t->os++) = (V);

/* */

int cci_inflate(cci_context_t* t)
{
	do { switch (t->state) {

	/* */

	GET_BITS(BLOCK, t->code[0], 3, 0)

	t->final =  t->code[0]       & 0x1;
	t->type  = (t->code[0] >> 1) & 0x3;

	if (t->type == 0) {
		/* no compression (3.2.4.) */
		t->bit = 0;
		t->has = 0;

		for (t->index = 0; t->index < 4; t->index++) {
			GET_BYTE(UNCOMP, t->code[t->index])
		}

		t->count = (t->code[1] << 8) | t->code[0];
		if (t->count != (~((t->code[3] << 8) | t->code[2]) & 0xffff)) {
			return CCI_R_ERROR;
		}

		for (t->index = 0; t->index < t->count; t->index++) {
			GET_BYTE(UNCOMP_GET,          t->code[0])
			PUT_BYTE(UNCOMP_PUT, (BYTE_T) t->code[0])
		}

		break;
	}

	if (t->type == 1) {
		/* fixed Huffman codes (3.2.6.) */
		t->huff_length_count  = T_LENGTH_COUNT;
		t->huff_length_lookup = T_LENGTH_LOOKUP;

		t->huff_distance_count  = T_DISTANCE_COUNT;
		t->huff_distance_lookup = T_DISTANCE_LOOKUP;

	} else if (t->type == 2) {
		/* dynamic Huffman codes (3.2.7.) */
		static const INT32_T T_COUNT[3 + 3] = {   5, 5, 4,  2, 3,  7 };
		static const INT32_T T_BASE [3 + 3] = { 257, 1, 4,  3, 3, 11 };

		static const INT32_T T_INDEX[19] = {
			16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
		};

		for (t->index = 0; t->index < 3; t->index++) {
			GET_BITS(DYNA, t->code[t->index], T_COUNT[t->index], T_BASE[t->index])
		}

		memset(t->length, 0, 19);

		for (t->index = 0; t->index < t->code[HCLEN]; t->index++) {
			GET_BITS(DYNA_LENGTH, t->length[T_INDEX[t->index]], 3, 0)
		}

		build_tree(t->length_count, t->length_lookup, t->length, 19);

		/* decode length */

		t->count = t->code[HLIT] + t->code[HDIST];

		for (t->index = 0; t->index < t->count; ) {
			GET_SYMBOL(DYNA_SYMBOL, t->length_count, t->length_lookup)

			if (t->symbol >= 0 && t->symbol < 16) {
				t->length[t->index++] = (BYTE_T) t->symbol;

			} else if (t->symbol >= 16 && t->symbol < 19) {
				t->symbol -= 16;

				if (t->symbol == 0) {
					if (t->index == 0) {
						return CCI_R_ERROR;
					}
					t->value = t->length[t->index - 1];
				} else {
					t->value = 0;
				}

				GET_BITS(DYNA_ADDS, t->code[HADDS], T_COUNT[3 + t->symbol], T_BASE[3 + t->symbol])

				if (t->index + t->code[HADDS] > t->count) {
					return CCI_R_ERROR;
				}

				memset(t->length + t->index, t->value, t->code[HADDS]);

				t->index += t->code[HADDS];

			} else {
				return CCI_R_ERROR;
			}
		}

		build_tree(t->length_count,   t->length_lookup,   t->length,                 t->code[HLIT ]);
		build_tree(t->distance_count, t->distance_lookup, t->length + t->code[HLIT], t->code[HDIST]);

		/* */

		t->huff_length_count  = t->length_count;
		t->huff_length_lookup = t->length_lookup;

		t->huff_distance_count  = t->distance_count;
		t->huff_distance_lookup = t->distance_lookup;

	} else {
		return CCI_R_ERROR;
	}

	/* compressed block (3.2.3.) */

	for (; ; ) {
		/* decode symbol */
		GET_SYMBOL(COMP, t->huff_length_count, t->huff_length_lookup)

		if (t->symbol >= 0 && t->symbol < 256) {
			/* literal */
			PUT_BYTE(COMP_PUT, (BYTE_T) t->symbol)

		} else if (t->symbol >= 257 && t->symbol < 286) {
			/* decode length */
			t->symbol -= 257;

			t->count = T_LENGTH_BASE[t->symbol];
			if (T_LENGTH_ADDS[t->symbol] > 0) {
				GET_BITS(COMP_LENGTH_ADDS, t->count, T_LENGTH_ADDS[t->symbol], t->count)
			}

			/* decode distance */
			GET_SYMBOL(COMP_DISTANCE, t->huff_distance_count, t->huff_distance_lookup)

			if (t->symbol < 0 || t->symbol >= 30) {
				return CCI_R_ERROR;
			}

			t->index = T_DISTANCE_BASE[t->symbol];
			if (T_DISTANCE_ADDS[t->symbol] > 0) {
				GET_BITS(COMP_DISTANCE_ADDS, t->index, T_DISTANCE_ADDS[t->symbol], t->index)
			}

			t->index  = (t->os - t->window) - t->index;
			t->count += t->index;

			/* copy */
			for (; t->index < t->count; t->index++) {
				PUT_BYTE(COMP_PUT_COPY, t->window[t->index & CCI_WINDOW_MASK])
			}

		} else if (t->symbol == 256) {
			break;

		} else {
			return CCI_R_ERROR;
		}
	}

	break;

	/* */

	} /* switch (t->state) */

	/* */

	t->state = BLOCK;

	} while (t->final == 0);

	return CCI_R_FINAL;
}

/* */

