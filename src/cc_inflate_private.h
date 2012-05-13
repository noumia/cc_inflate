/* cc_inflate_private.h */

#pragma once

#ifndef CC_INFLATE_PRIVATE_H_
#define CC_INFLATE_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char BYTE_T;

typedef short INT16_T;
typedef int   INT32_T;

typedef enum cci_container {

	HEADER,

	G_HEADER,
	G_FEXTRA_LENGTH,
	G_FEXTRA,
	G_FNAME,
	G_FCOMMENT,
	G_FHCRC,
	G_INFLATE,
	G_CHECK,

	Z_HEADER,
	Z_INFLATE,
	Z_CHECK

} cci_container_t;

typedef enum cci_state {

	BLOCK,

	UNCOMP,
	UNCOMP_GET,
	UNCOMP_PUT,

	DYNA,
	DYNA_LENGTH,
	DYNA_SYMBOL,
	DYNA_ADDS,

	COMP,
	COMP_PUT,
	COMP_LENGTH_ADDS,
	COMP_DISTANCE,
	COMP_DISTANCE_ADDS,
	COMP_PUT_COPY

} cci_state_t;

/* cci_context */
struct cci_context {

	cci_env_t* env;

	/* I/O */

	BYTE_T* window;

	BYTE_T* os;
	BYTE_T* oe;

	const void* stream;

	const BYTE_T* bs;
	const BYTE_T* be;

	size_t input_size;
	size_t output_size;

	/* Header */

	cci_container_t container;

	BYTE_T header[20];

	INT32_T crc;
	INT32_T uncomp_size;

	/* Inflate */

	cci_state_t state;

	INT32_T bit;
	INT32_T has;

	const INT32_T* huff;
	const INT32_T* hend;
	INT32_T        symbol;
	INT32_T        value;
	const INT16_T* lookup;

	INT32_T final;
	INT32_T type;

	INT32_T code[4];

	INT32_T count;
	INT32_T index;

	const INT32_T* huff_length_count;
	const INT16_T* huff_length_lookup;

	const INT32_T* huff_distance_count;
	const INT16_T* huff_distance_lookup;

	BYTE_T length[288 + 32];

	INT32_T length_count [ 16];
	INT16_T length_lookup[288];

	INT32_T distance_count [16];
	INT16_T distance_lookup[32];

}; /* cci_context */

#define CCI_WINDOW_SIZE 0x8000 /* 32 KiB */
#define CCI_WINDOW_MASK 0x7fff

#define HLIT  0
#define HDIST 1
#define HCLEN 2
#define HADDS 3

#ifdef __cplusplus
}
#endif

#endif /* CC_INFLATE_PRIVATE_H_ */

