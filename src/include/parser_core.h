#ifndef _parser_core_h_
#define _parser_core_h_

#include <stdint.h>

#include "parser.h"
#include "../../utf8-utf16-converter/converter/include/converter.h"

#ifndef UXML_IMPL
/* make LSP happy */
static char		parser_byte_next(uxml_parser_t *restrict ctx);
static char 	parser_char_next(uxml_parser_t *restrict ctx);
static char 	parser_char_current(uxml_parser_t *restrict ctx);

#error "Unnamed parser implementation. Please define UXML_IMPL before including this file. E.g. ´#define UXML_IMPL my_impl´"
#endif

/* functions to handle the user provided buffer */

static inline void
parser_buf_entry_begin(uxml_parser_t *restrict ctx)
{
	ctx->buf_last_entry_idx = ctx->buf_len;
}

static inline int
parser_buf_entry_append(uxml_parser_t *restrict ctx, const char c)
{
	if (ctx->buf_len + 1 >= ctx->buf_size)
		return UXML_ERROR_OUT_OF_MEMORY;
	ctx->buf[ctx->buf_len++] = c;
	return UXML_OK;
}

static inline void
parser_buf_entry_remove_last(uxml_parser_t *restrict ctx, const size_t size)
{
	ctx->buf_len -= size;
}

static inline char *
parser_buf_last_entry(uxml_parser_t *restrict ctx, size_t *restrict length)
{
	*length = ctx->buf_len - ctx->buf_last_entry_idx;
	return ctx->buf + ctx->buf_last_entry_idx;
}


static inline char
parser_skip_whitespace(uxml_parser_t *restrict ctx)
{
	char c;
	/* assuming all characters <= ' ' as whitespace */
	for (c = parser_char_current(ctx); c <= ' ' && c; c = parser_char_next(ctx));
	//for (c = parser_char_current(ctx); c == ' ' || c == '\t' || c == '\r' || c == '\n'; c = parser_char_next(ctx));
	return c;
}

static inline int
parser_match_allow_whitespace(uxml_parser_t *restrict ctx, const char *restrict match)
{
	char c;
	for (c = parser_char_current(ctx); *match; match++, c = parser_char_next(ctx)) {
		c = parser_skip_whitespace(ctx);
		if (c != *match) return 0;
	}
	return 1;
}

static inline int
parser_match_exactly(uxml_parser_t *restrict ctx, const char *restrict match)
{
	char c;
	for (c = parser_char_current(ctx); *match; match++, c = parser_char_next(ctx))
		if (c != *match) return 0;
	return 1;
}

static inline uint_fast8_t
parser_match_exactly_from_buf(const char *restrict buf, const char *restrict match)
{
	for (; *match; match++, buf++)
		if (*buf != *match) return 0;
	return 1;
}

/* only lowercase, null terminated, at first digit (e.g. "abc" instead of "0xabc") */
static inline uint_fast16_t
_parser_hex_str_to_u16(const char *str)
{
	uint8_t b;
	uint16_t code = 0;
	uint_fast8_t i;
	for (i = 0; str[i]; i++) {
		b = str[i];
		if 		(str[i] >= '0' && str[i] <= '9') b = b - '0' + 10;
		else if (str[i] >= 'a' && str[i] <= 'f') b = b - 'a' + 10;
		else return 0;
		code = (code << 4) | (b & 0xF);
	}
	return code;
}

static inline uint_fast16_t
_parser_dec_str_to_u16(const char *str)
{
	int_fast8_t 	i;
	uint_fast16_t 	n = 0, j;
	/* strlen */
	for (i = 0; str[i]; i++);

	for (i--, j = 1; i >= 0; i--, j *= 10) {
		n += (str[i] - '0') * j;
//		printf("str[%d] = %c\n", i, str[i]);
	}
	return n;
//	printf("strtoint = %d\n", n);
}

/* how big is the largest xml entity in the `xml_entities` array + 1 ('\0') or the number of digits in uint16_t_max (65535) + 2 ('#' and 'x') + 1 ('\0') */
#define XML_ENTITY_MAX_LENGTH 6

static const char *xml_entities[] 	= { "lt",	"gt",	"amp", 	"apos",	"quot"	};
static const char xml_entities_c[]	= { '<',	'>',	'&', 	'\'',	'"'		};

/* assumes that the current char already is '&' */
static inline uint16_t
parser_char_current_resolve_xml_entity(uxml_parser_t *restrict ctx)
{
	uint16_t 		code;
	char 			buf[XML_ENTITY_MAX_LENGTH], c;
	uint_fast8_t 	i;
	
	c = parser_char_current(ctx);
	for (i = 0, c = parser_char_next(ctx); c != ';' && i < XML_ENTITY_MAX_LENGTH; i++, c = parser_char_next(ctx)) {
		if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
		buf[i] = c;
	}

	buf[i] = 0;

	/* handle entities by code (HTML Codes) */
	if (buf[0] == '#') {
		if (buf[1] == 'x') {
			/* hexadecimal */
			code = _parser_hex_str_to_u16(buf + 2);
		} else {
			/* decimal */
			code = _parser_dec_str_to_u16(buf + 1);
		}
		return code;
	}

	/* handle named xml entities */
	for (i = 0; i < (uint_fast8_t)(sizeof(xml_entities)/sizeof(xml_entities[0])); i++) {
		if (parser_match_exactly_from_buf(buf, xml_entities[i])) {
			return xml_entities_c[i];
		}
	}
	/* unknown xml entity */
	return 0;
}

static inline uint_fast8_t
parser_buf_entry_append_with_callback(uxml_parser_t *restrict ctx, const char c, size_t *restrict len, uxml_tag_func callback)
{
	if (parser_buf_entry_append(ctx, c) == UXML_ERROR_OUT_OF_MEMORY) {
		if (*len == 0) return UXML_ERROR_OUT_OF_MEMORY;

		/* notify the user */
		callback(ctx->usrdata, ctx->buf + ctx->buf_last_entry_idx, *len);

		parser_buf_entry_remove_last(ctx, *len);
		*len = 0;
		parser_buf_entry_append(ctx, c);
	}
	return UXML_OK;
}

static inline uint_fast8_t
parser_buf_entry_append_resolved_entity_with_callback(uxml_parser_t *restrict ctx, size_t *restrict len, uxml_tag_func callback)
{
	uint_fast8_t err = UXML_OK;
	uint16_t xml_entity;
	uint_fast8_t utf8_entities = 0;
	char c, utf8_entity_buf[4];
	xml_entity = parser_char_current_resolve_xml_entity(ctx);
	if (xml_entity == 0) return UXML_ERROR_UNKNOWN_XML_ENTITY;
	/* decode utf-16 entity */
	utf8_entities = utf16_to_utf8(&xml_entity, 1, (utf8_t *)utf8_entity_buf, sizeof(utf8_entity_buf));
	do {
		c = utf8_entity_buf[sizeof(utf8_entity_buf) - utf8_entities - 1];
		utf8_entities--;
		if (parser_buf_entry_append_with_callback(ctx, c, len, callback) == UXML_ERROR_OUT_OF_MEMORY)
			return UXML_ERROR_OUT_OF_MEMORY;
		*len+=1;
	} while (utf8_entities > 0);
	return err;
}

/* returns 1 when a comment was skipped */
static inline uint_fast8_t
parser_skip_comment(uxml_parser_t *restrict ctx)
{
	if (!parser_match_exactly(ctx, "<!--")) return 0;
	for (; !parser_match_exactly(ctx, "-->"); parser_char_next(ctx));
	return 1;
}

static inline char
parser_skip_any_whitespace_or_comment(uxml_parser_t *restrict ctx)
{
	char c = parser_skip_whitespace(ctx);
	while(parser_skip_comment(ctx) && c)
		c = parser_skip_whitespace(ctx);
	return c;
}

static const uint8_t xml_name_first_char_map[0xff] = {
	/* underscore */
	['_']=1,
	/* A ~ Z */
	['A']=1,['B']=1,['C']=1,['D']=1,['E']=1,['F']=1,['G']=1,['H']=1,['I']=1,['J']=1,
	['K']=1,['L']=1,['M']=1,['N']=1,['O']=1,['P']=1,['Q']=1,['R']=1,['S']=1,['T']=1,
	['U']=1,['V']=1,['W']=1,['X']=1,['Y']=1,['Z']=1,
	/* a ~ z */
	['a']=1,['b']=1,['c']=1,['d']=1,['e']=1,['f']=1,['g']=1,['h']=1,['i']=1,['j']=1,
	['k']=1,['l']=1,['m']=1,['n']=1,['o']=1,['p']=1,['q']=1,['r']=1,['s']=1,['t']=1,
	['u']=1,['v']=1,['w']=1,['x']=1,['y']=1,['z']=1,
};

static const uint8_t xml_name_char_map[0xff] = {
	/* hyphen, period, colon, and underscore */
	['-']=1,['.']=1,[':']=1,['_']=1,
	/* Numbers */
	['0']=1,['1']=1,['2']=1,['3']=1,['4']=1,['5']=1,['6']=1,['7']=1,['8']=1,['9']=1,
	/* A ~ Z */
	['A']=1,['B']=1,['C']=1,['D']=1,['E']=1,['F']=1,['G']=1,['H']=1,['I']=1,['J']=1,
	['K']=1,['L']=1,['M']=1,['N']=1,['O']=1,['P']=1,['Q']=1,['R']=1,['S']=1,['T']=1,
	['U']=1,['V']=1,['W']=1,['X']=1,['Y']=1,['Z']=1,
	/* a ~ z */
	['a']=1,['b']=1,['c']=1,['d']=1,['e']=1,['f']=1,['g']=1,['h']=1,['i']=1,['j']=1,
	['k']=1,['l']=1,['m']=1,['n']=1,['o']=1,['p']=1,['q']=1,['r']=1,['s']=1,['t']=1,
	['u']=1,['v']=1,['w']=1,['x']=1,['y']=1,['z']=1,
};

static inline int
parser_parse_xml_name(uxml_parser_t *restrict ctx)
{
	int err = UXML_OK;
	uint8_t c = parser_char_current(ctx);
	/* a tag name or attribute MUST start with a letter or underscore */
	if (!xml_name_first_char_map[c])
		return UXML_ERROR_XML_NAME;
	
	parser_buf_entry_begin(ctx);
	err = parser_buf_entry_append(ctx, c);

	for (c = parser_char_next(ctx); xml_name_char_map[c] && err == UXML_OK; c = parser_char_next(ctx))
		err = parser_buf_entry_append(ctx, c);
	return err;
}


static inline int
parser_parse_xml_attribute_value(uxml_parser_t *restrict ctx)
{
	int err = UXML_OK;
	size_t len = 0;
//	uint16_t xml_entity;
//	uint_fast8_t utf8_entities = 0;
	char c;//, utf8_entity_buf[4];
	char quotechar;
	

	if (!parser_match_allow_whitespace(ctx, "="))
		return UXML_OK;

	if ((c = parser_skip_whitespace(ctx)) == '\0')
		return UXML_ERROR_EOF;

	if (c != '"' && c != '\'')
		return err;

	parser_buf_entry_begin(ctx);
	quotechar = c;
	ctx->parser_needs_decoded_char = 1;
	c = parser_char_next(ctx);
	do {
		if (c == quotechar) break;
//		if (c == '\\') {
//			c = parser_char_next(ctx);
//			if (!c) return UXML_ERROR_EOF;
//				if (!(c == '\\' || c == quotechar))
//					return UXML_ERROR_UNKNOWN_ESCAPE_SEQUENCE;
		if (c == '&') {
			/*
			xml_entity = parser_char_current_resolve_xml_entity(ctx);
			if (xml_entity == 0) return UXML_ERROR_UNKNOWN_XML_ENTITY;
			/ decode utf-16 entity /
			utf8_entities = utf16_to_utf8(&xml_entity, 1, (utf8_t *)utf8_entity_buf, sizeof(utf8_entity_buf));
			do {
				c = utf8_entity_buf[sizeof(utf8_entity_buf) - utf8_entities - 1];
				utf8_entities--;
				err = parser_buf_entry_append(ctx, c);
				if (err != UXML_OK)
					return err;
			} while (utf8_entities > 0);*/
			switch(parser_buf_entry_append_resolved_entity_with_callback(ctx, &len, ctx->tag_attribute_value_func)) {
				case UXML_ERROR_UNKNOWN_XML_ENTITY:
					return UXML_ERROR_UNKNOWN_XML_ENTITY;
				case UXML_ERROR_OUT_OF_MEMORY:
					return UXML_ERROR_OUT_OF_MEMORY;
			}
			c = parser_char_next(ctx);
			continue;
		}
		err = parser_buf_entry_append_with_callback(ctx, c, &len, ctx->tag_attribute_value_func);
//			err = parser_buf_entry_append(ctx, c);
		if (err != UXML_OK)
			return err;

		len++;
		c = parser_char_next(ctx);
	} while (c);

	/* notify the user */
	if (len > 0) {
		err = ctx->tag_attribute_value_func(ctx->usrdata, ctx->buf + ctx->buf_last_entry_idx, len);
		if (err != UXML_OK) return err;
	}
	parser_buf_entry_remove_last(ctx, len);

	ctx->parser_needs_decoded_char = 0;
	if (c) parser_char_next(ctx);
	return err;
}


static inline int
parser_parse_prolog(uxml_parser_t *restrict ctx)
{
//	for (c = parser_char_current(ctx);c != '';);

	for (; !parser_match_exactly(ctx, "?>"); parser_char_next(ctx));
/*
	if (!parser_match_allow_whitespace(ctx, "<?")) return UXML_ERROR_INPUT;

	parser_skip_whitespace(ctx);
	
	if (parser_match_exactly(ctx, "version")) {
		if (!parser_match_allow_whitespace(ctx, "=")) return UXML_ERROR_INPUT;
		c = parser_skip_whitespace(ctx);
		if (c != '"' && c != '\'')
			return UXML_ERROR_INPUT;

	}

	if (!parser_match_allow_whitespace(ctx, "?>")) return UXML_ERROR_INPUT;*/
	return UXML_OK;
}


static inline int
parser_parse_tag_value(uxml_parser_t *restrict ctx)
{
	int 			err = UXML_OK;
	size_t 			len;
	uint_fast8_t 	comment_skip = 0;//, utf8_entities = 0;
//	uint16_t 		xml_entity;
	char 			c;//, utf8_entity_buf[4];

	/* attempt to skip comments */
	for (;parser_skip_comment(ctx););
	c = parser_char_current(ctx);
	
	if (c == '\0') return UXML_ERROR_EOF;
	
	ctx->parser_needs_decoded_char = 1;
	parser_buf_entry_begin(ctx);

	for (len = 0; ;) {
		while (c != '<') {
			if (c == '\0') return UXML_ERROR_EOF;
			if (c == '&') {
				/* resolve xml entity into utf-16 /
				xml_entity = parser_char_current_resolve_xml_entity(ctx);
				if (xml_entity == 0) return UXML_ERROR_UNKNOWN_XML_ENTITY;
				/ decode utf-16 entity /
				utf8_entities = utf16_to_utf8(&xml_entity, 1, (utf8_t *)utf8_entity_buf, sizeof(utf8_entity_buf));
				do {
					c = utf8_entity_buf[sizeof(utf8_entity_buf) - utf8_entities - 1];
					utf8_entities--;
					if (parser_buf_entry_append(ctx, c) == UXML_ERROR_OUT_OF_MEMORY) {
						if (len == 0) return UXML_ERROR_OUT_OF_MEMORY;
						
						/ notify the user /
						ctx->tag_value_func(ctx->usrdata, ctx->buf + ctx->buf_last_entry_idx, len);

						parser_buf_entry_remove_last(ctx, len);
						len = 0;
						parser_buf_entry_append(ctx, c);
					}
					len++;
				} while (utf8_entities > 0);*/

				err = parser_buf_entry_append_resolved_entity_with_callback(ctx, &len, ctx->tag_value_func);
				if (err != UXML_OK)
					return err;

				c = parser_char_next(ctx);
				continue;	
			}
/*			if (parser_buf_entry_append(ctx, c) == UXML_ERROR_OUT_OF_MEMORY) {
				if (len == 0) return UXML_ERROR_OUT_OF_MEMORY;

				/ notify the user /
				ctx->tag_value_func(ctx->usrdata, ctx->buf + ctx->buf_last_entry_idx, len);
				
				parser_buf_entry_remove_last(ctx, len);
				len = 0;
				parser_buf_entry_append(ctx, c);
			}*/
			err = parser_buf_entry_append_with_callback(ctx, c, &len, ctx->tag_value_func);
			if (err != UXML_OK)
				return err;
			len++;
			c = parser_char_next(ctx);
		}

		/* notify the user */
		if (len > 0) {
			err = ctx->tag_value_func(ctx->usrdata, ctx->buf + ctx->buf_last_entry_idx, len);
			if (err != UXML_OK) return err;
			parser_buf_entry_remove_last(ctx, len);
			len = 0;
		}
		
		/* check if it's a tag closing (this consumes the char '<' to test agains the next one '/' ) */
		if (parser_match_allow_whitespace(ctx, "</")) {
			ctx->parser_needs_decoded_char = 0;
			return PARSER_IS_CLOSE_TAG;
		}

		/* attempt to skip comments
		 * the first comment is handled differently because the first character was consumed by the previous match */
		ctx->parser_needs_decoded_char = 0;
		comment_skip = 0;
		if (parser_match_exactly(ctx, "!--")) {
			comment_skip = 1;
			for (;!parser_match_exactly(ctx, "-->"); parser_char_next(ctx));
		}

		while (parser_skip_comment(ctx));
		
		c = parser_char_current(ctx);
		if (c == '\0') return UXML_ERROR_EOF;

		if (!comment_skip)
			break;
	}
	ctx->parser_needs_decoded_char = 0;
	return UXML_OK;
}

static inline int
parser_parse_tag_closing_name(uxml_parser_t *restrict ctx, const char *restrict tag_name, const size_t tag_name_length)
{
	size_t 	i;	
	uint8_t	c;

	/* parse the tag name comparing against opening tag name */
	for (i = 0, c = parser_skip_whitespace(ctx); i < tag_name_length; i++, c = parser_char_next(ctx))
		if (tag_name[i] != c)
			return UXML_ERROR_TAG_CLOSE_MISMATCH;

	/* if c is a valid xml_name char */
	if (xml_name_char_map[c])
		return UXML_ERROR_TAG_CLOSE_MISMATCH;

	return UXML_OK;
}


/* when called this function assumes that the first character '<' is already consumed */
static inline int
parser_parse_tag(uxml_parser_t *restrict ctx, const char *restrict parent_tag_name, const size_t parent_tag_name_length)
{
	char 		*tag_name,
				*attribute_name; 

	size_t 		tag_name_length,
				attribute_name_length;

	int 		err = UXML_OK;

	/* skip any whitespace in between the tag name and the '<' symbol */
	if (parser_skip_whitespace(ctx) == '\0')
		return UXML_ERROR_EOF;

	/* parse the tag name */
	err = parser_parse_xml_name(ctx);
	if (err != UXML_OK) return err;
	tag_name = parser_buf_last_entry(ctx, &tag_name_length);

	/* notify the user */
	err = ctx->tag_open_func(ctx->usrdata, tag_name, tag_name_length);
	if (err != UXML_OK) return err;

	/* skip any whitespace in between the tag name and the first attribute */
	if (parser_skip_whitespace(ctx) == '\0')
		return UXML_ERROR_EOF;

	/* parse attributes: (key = 'value') or only (key) */
	for (err = parser_parse_xml_name(ctx); err == UXML_OK; err = parser_parse_xml_name(ctx)) {

		attribute_name = parser_buf_last_entry(ctx, &attribute_name_length);

		/* notify the user */
		err = ctx->tag_attribute_name_func(ctx->usrdata, attribute_name, attribute_name_length);
		if (err != UXML_OK)
			return err;
		parser_buf_entry_remove_last(ctx, attribute_name_length);
	

		err = parser_parse_xml_attribute_value(ctx);
		if (err != UXML_OK) return err;

//		attribute_value = parser_buf_last_entry(ctx, &attribute_value_length);
//		if (attribute_value_length == 0)
//			attribute_value = NULL;

		/* notify the user */
//		ctx->tag_attribute_func(ctx->usrdata, attribute_name, attribute_name_length, attribute_value, attribute_value_length);

		if (parser_skip_whitespace(ctx) == '\0')
			return UXML_ERROR_EOF;

		/* remove entries */
//		parser_buf_entry_remove_last(ctx, attribute_value_length);
	}
	if (err == UXML_ERROR_OUT_OF_MEMORY)
		return err;
	err = UXML_OK;

	/* handle self closing tag */
	if (parser_match_allow_whitespace(ctx, "/>"))
		goto tag_closed;

	/* handle last '>' of open tag */
	if (!parser_match_allow_whitespace(ctx, ">"))
		return UXML_ERROR_UNCLOSED_OPEN_TAG;

	/* while not a closing tag */
	for (;;) {

		/* attempt to parse the tag value.
		 * parser_parse_tag_value only stops after the first character that is not part of the token '<!--') */
		err = parser_parse_tag_value(ctx);
		if (err == PARSER_IS_CLOSE_TAG)
			break;

		if (err != UXML_OK) return err;

		/* attempt to parse a child */
		err = parser_parse_tag(ctx, tag_name, tag_name_length);
		if (err == PARSER_CHILD_END) break;

		if (err != UXML_OK) return err;
	}

	/* handle closing tag name */
	/* previous match already consumed '</' */
	err = parser_parse_tag_closing_name(ctx, tag_name, tag_name_length);
	if (err != UXML_OK) return err;

	if (!parser_match_allow_whitespace(ctx, ">"))
		return UXML_ERROR_UNCLOSED_CLOSE_TAG;


tag_closed:
	ctx->tag_close_func(ctx->usrdata, tag_name, tag_name_length);
	/* remove itself from the buffer */
	parser_buf_entry_remove_last(ctx, tag_name_length);

	/* if this tag is the root element we are done */
	if (parent_tag_name_length == 0)
		return UXML_OK;
	
//	if (parser_skip_any_whitespace_or_comment(ctx) == '\0')
//		return ctx->buf_len > 0? UXML_ERROR_INPUT : UXML_OK;
	if (parser_skip_any_whitespace_or_comment(ctx) == '\0')
		return UXML_ERROR_EOF;

	/* if parent is closing there is no 'next' sibling (note: comment function already consumed the '<' character to check if the next one is a '-' )*/
	if (parser_match_allow_whitespace(ctx, "/"))
		return PARSER_CHILD_END;

	/* parse the next sibling */
	err = parser_parse_tag(ctx, parent_tag_name, parent_tag_name_length);
	if (err != UXML_OK) return err;
	
	return UXML_OK;
}

int
UXML_IMPL_FUNC(parse)(uxml_parser_t *restrict ctx, int prolog)
{
	int err = UXML_OK;
	char c;
//	printf("encoding = %d\n", ctx->encoding);

	ctx->parser_needs_decoded_char = 0;
	
	c = parser_byte_current(ctx);
	if (c == '?') prolog = 1;

	if (ctx->encoding != UXML_ENCODING_UTF8) {
		/* this is needed to fullfill the condition to start decoding utf16 */
		ctx->utf8_buf_len = 1;
		ctx->utf8_buf_idx = 0;
		parser_byte_next(ctx);
		c = parser_char_next(ctx);
//		printf("char = %d (%c)\n", c, c);
//		printf("buf = %.*s\n", ctx->utf8_buf_len, ctx->utf8_buf);
	}
	
	if (parser_match_allow_whitespace(ctx, "?"))
		prolog = 1;

	/* handle prolog */
	if (prolog) {
//		puts("handle prolog");
		err = parser_parse_prolog(ctx);
		if (err != UXML_OK) return err;
	}

	/* skip whitespace / comments */
	if (parser_char_current(ctx) <= ' ') {
		/* this function already consumes the first '<' character */
		if (parser_skip_any_whitespace_or_comment(ctx) == '\0')
			return UXML_ERROR_EOF;
	}

//	if (!parser_match_allow_whitespace(ctx, "<"))
//		return UXML_ERROR_INPUT;

	/* parse the root element */
	err = parser_parse_tag(ctx, NULL, 0);
	return err;
}


#endif
