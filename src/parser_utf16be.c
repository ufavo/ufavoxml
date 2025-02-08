#include "include/parser_utf16_decoder.h"

static inline char
parser_char_current(uxml_parser_t *restrict ctx)
{
	return ctx->utf8_buf[ctx->utf8_buf_idx];
}

static inline char
parser_char_next(uxml_parser_t *restrict ctx)
{
	if (ctx->utf8_buf_idx +1 == ctx->utf8_buf_len)
		return utf16be_decode(ctx);

	return ctx->utf8_buf[++ctx->utf8_buf_idx];
}

#define UXML_IMPL utf16be
#include "include/parser_core.h"
#undef UXML_IMPL
