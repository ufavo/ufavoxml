#include <stdint.h>

#include "include/parser_byte.h"
#include "include/parser_utf8.h"
#include "include/parser_utf16.h"

/* attempts to match a known BOM
 * returns 1 on success, 0 on failure */
static inline int
parser_detect_encoding_from_bom(uxml_parser_t *restrict ctx)
{
	uint8_t c;

	/* assuming utf-8 */
	ctx->encoding = UXML_ENCODING_UTF8;

	/* byte 0 */
	c = parser_byte_current(ctx);
	switch (c) {
		case 0xEF: /* utf-8 ? */
		case 0xFE: /* utf-16be ? */
		case 0xFF: /* utf-16le ? */
			break;
		/* not a BOM */
		default: return 0;
	}
	
	/* byte 1 */
	c = parser_byte_next(ctx);
	switch (c) {
		case 0xBB: /* utf-8 ? */
			break;
		case 0xFF: /* utf-16be */
			ctx->encoding = UXML_ENCODING_UTF16BE;
			parser_byte_next(ctx);
			return 1;
		case 0xFE: /* utf-16le */
			ctx->encoding = UXML_ENCODING_UTF16LE;
			parser_byte_next(ctx);
			return 1;
		default: return 0;
	}
	
	/* byte 2 */
	c = (uint8_t)parser_byte_next(ctx);
	/* not a BOM */
	if (c != 0xBF) return 0;

	/* utf-8 */
	return 1;
}

/* detects the encoding assuming a '<' as first char
 * returns XML_OK when successful, indicating that the next element is a tag ('<' already consumed).
 * returnx PARSER_IS_PROLOG when successful, indicating that the next element is the prolog ('<' already consumed). */
static inline int
parser_detect_encoding_from_content(uxml_parser_t *restrict ctx)
{
	char c, zero_cnt;

	/* assuming utf-8 */
	ctx->encoding = UXML_ENCODING_UTF8;

	/* skip whitespaces (encoding agnostic) */
	for (c = parser_byte_current(ctx), zero_cnt = 0; (c <= ' ' || c == '\n' || c == '\t') && zero_cnt < 2; c = parser_byte_next(ctx))
		if (c == 0) zero_cnt++;	else zero_cnt = 0;

	if (zero_cnt > 1) return UXML_ERROR_INPUT;


	if (c == '<') {
		/* utf-8 or utf-16le */
		c = parser_byte_next(ctx);
		switch (c) {
			case 0: 
				c = parser_byte_next(ctx);
				/* end of utf-8 */
				if (c == 0) 
					return UXML_ERROR_INPUT;
				
				/* utf-16le */
				ctx->encoding = UXML_ENCODING_UTF16LE;
				break;
			default: /* utf-8 */
				break;
		}
	} else if (c == '\0') {
		/* utf-16be or end of utf-8 */
		c = parser_byte_next(ctx);
		/* end of utf-8 */
		if (c == 0)
			return UXML_ERROR_INPUT;

		if (c == '<') {
			/* utf-16be */
			ctx->encoding = UXML_ENCODING_UTF16BE;
		} else {
			return UXML_ERROR_INPUT;
		}
	}

	return UXML_OK;
}

static inline int
detect_encoding(uxml_parser_t *restrict ctx)
{
	if (parser_detect_encoding_from_bom(ctx))
		return UXML_OK;
	return parser_detect_encoding_from_content(ctx);
}

int
uxml_parse(uxml_parser_t *restrict ctx)
{
	int err = UXML_OK;
	
	/* make sure that the data buffer is not empty */
	if (ctx->length == 0)
		parser_byte_next(ctx);
	
	err = detect_encoding(ctx);
	if (err != UXML_OK && err != PARSER_IS_PROLOG) return err;

	err = err == PARSER_IS_PROLOG;

	switch (ctx->encoding) {
		case UXML_ENCODING_UTF8:
			err = utf8_parse(ctx, err);
			break;
		case UXML_ENCODING_UTF16LE:
			err = utf16le_parse(ctx, err);
			break;
		case UXML_ENCODING_UTF16BE:
			err = utf16be_parse(ctx, err);
			break;
	}
	return err;
}
