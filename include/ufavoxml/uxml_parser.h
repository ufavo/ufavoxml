#ifndef _uxml_parser_h_
#define _uxml_parser_h_

#include <stdint.h>
#include <stddef.h>

enum {
	
	UXML_OK = 0,
	UXML_ABORT,

	UXML_ERROR_EOF,
	UXML_ERROR_INPUT,
	UXML_ERROR_XML_NAME,
	UXML_ERROR_OUT_OF_MEMORY,
	UXML_ERROR_TAG_CLOSE_MISMATCH,
	UXML_ERROR_UNKNOWN_ESCAPE_SEQUENCE,
	UXML_ERROR_UNKNOWN_XML_ENTITY,
	UXML_ERROR_UNCLOSED_OPEN_TAG,
	UXML_ERROR_UNCLOSED_CLOSE_TAG,
};

static inline const char *
uxml_errstr(const int err)
{
	static const char *errv[] = {
		"Ok",
		"Aborted by user",
		"Unexpected end of file",
		"Malformed XML document",
		"Invalid XML Name",
		"Out of memory, consider increasing the user provided buffer size",
		"Close tag name does not match with the last opened tag",
		"Unknown escape sequence",
		"Unknown XML Entity",
		"Incomplete/unclosed open tag",
		"Incomplete/unclosed close tag"
	};
	if (err < (int)(sizeof(errv)/sizeof(errv[0])) && err >= 0)
		return errv[err];
	return "Unknown error code.";
}

enum {
	UXML_ENCODING_UTF8 = 0,
	UXML_ENCODING_UTF16LE,
	UXML_ENCODING_UTF16BE,
};

/* called when the parser needs more data.
 * the callee should set the `*data_out` to a buffer containing more data and return the amount avalible.
 * when the callee returns 0 the parser assumes EOF */
typedef size_t 	(*uxml_data_func)(void *usrdata, const char **data_out);
 /* the callee must return UXML_OK or UXML_ABORT. */
typedef int 	(*uxml_tag_func)(void *usrdata, char *str, size_t length);

typedef struct uxml_parser {
	/* callbacks */
	uxml_data_func 		data_func;
	/* called whenever a tag opens. */
	uxml_tag_func 		tag_open_func;
	/* called whenever a tag closes. */
	uxml_tag_func		tag_close_func;
	/* called after a tag opens, once for each attribute. */
	uxml_tag_func		tag_attribute_name_func;
	/* may be called multiple times after an attribute name.
	 * if the attribute has no value this function will not be called. */
	uxml_tag_func		tag_attribute_value_func;
	/* called when parsing a tag inner value. May be called multiple times for the same tag. */
	uxml_tag_func 		tag_value_func;
	void 				*usrdata;
	
	/* intermediate buffer to store transcoded utf16 characters */
	char 			utf8_buf[16];
	uint_fast8_t 	utf8_buf_len;
	uint_fast8_t 	utf8_buf_idx;
	/* flag to speed up utf16 parsing (avoid decoding utf16 characters when not needed) */
	uint_fast8_t 	parser_needs_decoded_char;
	
	uint_fast8_t 	encoding;

	/* input data ptr */
	const char 		*data;
	size_t 			length;
	
	/* user provided buffer */
	size_t 		buf_last_entry_idx;
	size_t 		buf_len;
	size_t 		buf_size;
	char 		*restrict buf;

	/* tracking */
	size_t 		line_num;
} uxml_parser_t;

int	uxml_parse(uxml_parser_t *restrict ctx);

#endif
