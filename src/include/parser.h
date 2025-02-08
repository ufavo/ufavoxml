#ifndef _parser_h_
#define _parser_h_

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	#define IS_LITTLE_ENDIAN
#else
	#define IS_BIG_ENDIAN
#endif

#include "../../include/ufavoxml/uxml_parser.h"

#define PASTER(x,y) x ## _ ## y
#define EVALUATOR(x,y) PASTER(x,y)
#define UXML_IMPL_FUNC(name) EVALUATOR(UXML_IMPL,name)

enum {
	PARSER_CHILD_END = 128,
	PARSER_IS_CLOSE_TAG,
	PARSER_IS_PROLOG,
	PARSER_IS_TAG,
};

#endif
