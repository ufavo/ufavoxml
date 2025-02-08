#ifndef _uxml_utf8_impl_
#define _uxml_utf8_impl_

#include "include/parser_byte.h"

static inline char
parser_char_current(uxml_parser_t *restrict ctx)
{
	return parser_byte_current(ctx);
}

static inline char
parser_char_next(uxml_parser_t *restrict ctx)
{
	char n = parser_byte_next(ctx);
	if (n == '\n') ctx->line_num++;
	return n;
}

#define UXML_IMPL utf8
#include "include/parser_core.h"
#undef UXML_IMPL

#endif
