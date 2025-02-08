#ifndef _parser_utf16_decoder_h_
#define _parser_utf16_decoder_h_

#include "../../utf8-utf16-converter/converter/include/converter.h"
#include "parser_byte.h"
#include <stdint.h>

static inline char
__utf16_native_decode(uxml_parser_t *restrict ctx, uint8_t *in, const uint_fast8_t i)
{
	ctx->utf8_buf_len = utf16_to_utf8((utf16_t *)in, i / sizeof(utf16_t), (utf8_t *)ctx->utf8_buf, sizeof(ctx->utf8_buf));
	ctx->utf8_buf_idx = 0;
	if (ctx->utf8_buf_len == 0) return '\0';
	return *ctx->utf8_buf;
}

static inline char
utf16le_decode(uxml_parser_t *restrict ctx)
{
	uint8_t 		in[8];
	uint_fast8_t 	i;

	if (!ctx->parser_needs_decoded_char) {
		ctx->utf8_buf[0] = parser_byte_next(ctx);
		ctx->utf8_buf_idx = 0;
		ctx->utf8_buf_len = 1;
		if (parser_byte_next(ctx) > 0)
			ctx->utf8_buf[0] = (char)255;
		return *ctx->utf8_buf;
	}

	/* load utf16 characters */
	for (i = 0; i < (uint_fast8_t)sizeof(in); i+=2) {
#ifdef IS_BIG_ENDIAN
		in[i+1] = (uint8_t)parser_byte_next(ctx);
		in[i] = (uint8_t)parser_byte_next(ctx);
#endif
#ifdef IS_LITTLE_ENDIAN
		in[i] = (uint8_t)parser_byte_next(ctx);
		in[i+1] = (uint8_t)parser_byte_next(ctx);
#endif
	}
	return __utf16_native_decode(ctx, in, i);
}

static inline char
utf16be_decode(uxml_parser_t *restrict ctx)
{
	uint8_t 		in[8];
	uint_fast8_t 	i;

	if (!ctx->parser_needs_decoded_char) {
		ctx->utf8_buf_idx = 0;
		ctx->utf8_buf_len = 1;
		if (parser_byte_next(ctx) > 0) {
			ctx->utf8_buf[0] = (char)255;
			parser_byte_next(ctx);
			return *ctx->utf8_buf;
		}
		ctx->utf8_buf[0] = parser_byte_next(ctx);
		return *ctx->utf8_buf;	
	}
	/* load utf16 characters */
	for (i = 0; i < (uint_fast8_t)sizeof(in); i+=2) {
#ifdef IS_BIG_ENDIAN
		in[i] = (uint8_t)parser_byte_next(ctx);
		in[i+1] = (uint8_t)parser_byte_next(ctx);
#endif
#ifdef IS_LITTLE_ENDIAN
		in[i+1] = (uint8_t)parser_byte_next(ctx);
		in[i] = (uint8_t)parser_byte_next(ctx);
#endif
	}
	return __utf16_native_decode(ctx, in, i);
}

#endif
