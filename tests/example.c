#include <stdio.h>
#include <stdint.h>

#include "../include/ufavoxml/uxml_parser.h"

struct stream {
	FILE 	*file;
	char 	buffer[512];
};

size_t
data_cb_eof(void *usrdata, const char **data_out)
{
	puts("EOF");
	return 0;
}

size_t
data_cb_stream(void *usrdata, const char **data_out)
{
	struct stream *s = usrdata;
	size_t i, read;
	
	if (!s->file) {
		printf("Parser asked for more data but the file pointer is NULL.\n Returning 0.\n");
		return 0;
	}

	for (i = 0, read = 0; (i = fread(s->buffer + read, 1, sizeof(s->buffer) - read, s->file)) > 0; read += i);
	
	*data_out = s->buffer;
	printf("Parser asked for more data...\nProviding %zd bytes.\n", read);
	if (read < sizeof(s->buffer)) {
		printf("Reached EOF... The file will be closed.\n");
		fclose(s->file);
		s->file = NULL;
	}
	return read;
}

int
tag_open_cb(void *usrdata, char *tag_name, size_t tag_name_length)
{
	printf("TAG OPEN       [%.*s]\n", (int)tag_name_length, tag_name);
	return UXML_OK;
}

int
tag_close_cb(void *usrdata, char *tag_name, size_t tag_name_length)
{
	printf("TAG CLOSE      [%.*s]\n", (int)tag_name_length, tag_name);
	return UXML_OK;
}

int
tag_value_cb(void *usrdata, char *value, size_t value_length)
{
	printf("TAG VALUE      [%.*s]\n", (int)value_length, value);
	return UXML_OK;
}

int
tag_attribute_name_cb(void *usrdata, char *attribute_name, size_t attribute_name_length)
{
	printf("TAG ATTR NAME  [%.*s]\n", (int)attribute_name_length, attribute_name);
	return UXML_OK;
}
int
tag_attribute_value_cb(void *usrdata, char *attribute_value, size_t attribute_value_length)
{
	printf("TAG ATTR VALUE [%.*s]\n", (int)attribute_value_length, attribute_value);
	return UXML_OK;
}

int
main()
{
	/* create a zero initialized parser and assign a buffer to it */
	uxml_parser_t ctx = {0};
	char buf[1024];
	ctx.buf = buf;
	ctx.buf_size = sizeof(buf);

	/* assign the callbacks */
	ctx.tag_open_func = tag_open_cb;
	ctx.tag_close_func = tag_close_cb;
	ctx.tag_value_func = tag_value_cb;
	ctx.tag_attribute_name_func = tag_attribute_name_cb;
	ctx.tag_attribute_value_func = tag_attribute_value_cb;
	ctx.data_func = data_cb_stream;
	/* tell uxml to trim the tag values */
	ctx.opt_tag_value__trim = 1;
	/* load up a test xml file */
	struct stream stream = {0};
	stream.file = fopen("tree.xml", "r");
	if (!stream.file) return 1;
	
	/* assign the userdata */
	ctx.usrdata = &stream;

	/* parse the file */
	int error = uxml_parse(&ctx);

	printf("Parser returned: %d (%s)\n", error, uxml_errstr(error));

	return 0;
}
