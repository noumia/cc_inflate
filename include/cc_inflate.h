/* cc_inflate.h */

#pragma once

#ifndef CC_INFLATE_H_
#define CC_INFLATE_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cci_env cci_env_t;

typedef void* (*cci_malloc_t)(cci_env_t*, size_t);
typedef void  (*cci_free_t  )(cci_env_t*, void* );

struct cci_env {
	cci_malloc_t cmalloc;
	cci_free_t   cfree;
};

typedef struct cci_context cci_context_t;

cci_context_t* cci_create_context(cci_env_t* env);

void cci_release_context(cci_context_t* t);

/* */

#define CCI_R_ERROR  0
#define CCI_R_INPUT  1
#define CCI_R_OUTPUT 2
#define CCI_R_FINAL  3

/* */

void cci_setup_input(cci_context_t* t, const void* input, size_t size);

int cci_inflate(cci_context_t* t);

const void* cci_fetch_output(cci_context_t* t, size_t* size);

/* */

int cci_decode_gzip(cci_context_t* t);
int cci_decode_zlib(cci_context_t* t);

/* */

#ifdef __cplusplus
}
#endif

#endif /* CC_INFLATE_H_ */

