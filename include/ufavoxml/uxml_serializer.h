#ifndef _uxml_serializer_h_
#define _uxml_serializer_h_

#include <stdio.h>
#include <stdint.h>

enum {
	NONE = 0,
	TAG_OPENED,
	TAG_CLOSED,
	ATTR_NAME,
	ATTR_VALUE,
	TAG_VALUE,
};

typedef struct uxml_serializer {
	uint_fast16_t depth;
	uint_fast8_t pretty;
	uint_fast8_t state;
} uxml_serializer_t;

static inline void
uxml_serializer_write(const char *str, size_t size)
{
	// TODO
	size_t i;
	for (i = 0; i < size; i++) {
		printf("%c",str[i]);
	}
}

static inline void
uxml_serializer_write_str(const char *str)
{
	// TODO
	printf("%s",str);
}

static inline void
uxml_serializer_write_escaped_str(const char *str)
{
	for (; *str; str++) {
		switch (*str) {
			case '<':	uxml_serializer_write("&lt;", 4);		break;
			case '>':	uxml_serializer_write("&gt;", 4);		break;
			case '&':	uxml_serializer_write("&amp;", 5);		break;
			case '\'':	uxml_serializer_write("&apos;", 6);		break;
			case '"':	uxml_serializer_write("&quot;", 6);		break;
			default: 	uxml_serializer_write(str,1); 			break;
		}
	}
}

static inline void
uxml_serialize_ident(size_t depth)
{
	for (; depth > 0; depth--)
		uxml_serializer_write("\t",1);
}

static inline void
uxml_serialize_tag_open(uxml_serializer_t *ctx, const char *name)
{
	if (ctx->state == ATTR_VALUE)
		uxml_serializer_write("\"",1);

	if (ctx->state != TAG_CLOSED && ctx->state != NONE)
		uxml_serializer_write(">",1);

	if (ctx->pretty) {
		if (ctx->depth > 0) {
			uxml_serializer_write("\n",1);
			uxml_serialize_ident(ctx->depth);
		}
	}
	uxml_serializer_write("<",1);
	uxml_serializer_write_str(name);
	ctx->depth++;
	ctx->state = TAG_OPENED;
}

static inline void
uxml_serialize_tag_close(uxml_serializer_t *ctx, const char *name)
{
	ctx->depth--;
	if (ctx->state == ATTR_VALUE)
		uxml_serializer_write("\"",1);
	if (ctx->state == TAG_OPENED || ctx->state == ATTR_VALUE) {
		uxml_serializer_write("/>",2);
	} else {
		if (ctx->pretty && ctx->state != TAG_VALUE) {
			uxml_serializer_write("\n",1);
			if (ctx->state == TAG_CLOSED) {
				uxml_serialize_ident(ctx->depth);
			}
		}
		uxml_serializer_write("</",2);
		uxml_serializer_write_str(name);
		uxml_serializer_write(">",1);
	}
	ctx->state = TAG_CLOSED;
}

static inline void
uxml_serialze_tag_value(uxml_serializer_t *ctx, const char *value)
{
	if (ctx->state == ATTR_VALUE)
		uxml_serializer_write("\"",1);
	if (ctx->state != TAG_CLOSED)
		uxml_serializer_write(">",1);
	uxml_serializer_write_escaped_str(value);
	ctx->state = TAG_VALUE;
}

static inline void
uxml_serialize_tag_attribute_name(uxml_serializer_t *ctx, const char *name)
{
	if (ctx->state == ATTR_VALUE)
		uxml_serializer_write("\"",1);

	uxml_serializer_write(" ",1);
	uxml_serializer_write_str(name);
	uxml_serializer_write("=\"",2);
	ctx->state = ATTR_NAME;
}

static inline void
uxml_serialize_tag_attribute_value(uxml_serializer_t *ctx, const char *value)
{
	uxml_serializer_write_escaped_str(value);
	ctx->state = ATTR_VALUE;
}


#endif
