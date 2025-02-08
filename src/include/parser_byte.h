#ifndef _parser_byte_h_
#define _parser_byte_h_

#include "parser.h"

static inline char
parser_byte_next(uxml_parser_t *restrict ctx)
{
	if (ctx->length == 0) {
		if (ctx->data_func)
			if ((ctx->length = ctx->data_func(ctx->usrdata, &ctx->data)) > 0) {
				/* got some data */
				ctx->length--;
				return *ctx->data;
			}
		return '\0';
	}
	ctx->data++;
	ctx->length--;
	return *ctx->data;
}

static inline char
parser_byte_current(uxml_parser_t *restrict ctx)
{
	return *ctx->data;
}

#endif
