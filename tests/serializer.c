#include <stdio.h>
#include <stdlib.h>

#include "../include/ufavoxml/uxml_serializer.h"

int
main(int argc, char **argv)
{
	uxml_serializer_t s = {0};
	s.pretty = 1;

	uxml_serialize_tag_open(&s, "root");
	
		uxml_serialize_tag_open(&s, "node");
		uxml_serialize_tag_attribute_name(&s, "foo");
		uxml_serialize_tag_attribute_value(&s, "");
		uxml_serialize_tag_attribute_name(&s, "bar");
		uxml_serialize_tag_attribute_value(&s, "baz");
		uxml_serialize_tag_close(&s, "node");
		
		uxml_serialize_tag_open(&s, "node");
		uxml_serialize_tag_attribute_name(&s, "foo");
		uxml_serialize_tag_attribute_value(&s, "");
		uxml_serialize_tag_attribute_name(&s, "bar");
		uxml_serialize_tag_attribute_value(&s, "baz");

			uxml_serialize_tag_open(&s, "a");
			uxml_serialze_tag_value(&s, "some value");
			uxml_serialize_tag_close(&s, "a");

			uxml_serialize_tag_open(&s, "a");
			uxml_serialize_tag_attribute_name(&s, "foo");
			uxml_serialize_tag_attribute_value(&s, "\"lmao, double quoted\" & his brother, as a \"tag\" <lmfao/>");
			uxml_serialze_tag_value(&s, "'some single quoted value'");
			uxml_serialize_tag_close(&s, "a");

			uxml_serialize_tag_open(&s, "a");
			uxml_serialize_tag_close(&s, "a");

		uxml_serialize_tag_close(&s, "node");
		
		uxml_serialize_tag_open(&s, "node");
		uxml_serialize_tag_attribute_name(&s, "foo");
		uxml_serialize_tag_attribute_value(&s, "");
		uxml_serialize_tag_attribute_name(&s, "bar");
		uxml_serialize_tag_attribute_value(&s, "baz");
		uxml_serialize_tag_close(&s, "node");

	uxml_serialize_tag_close(&s, "root");
	return 0;
}
