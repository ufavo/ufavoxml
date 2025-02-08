#ifndef _parser_utf8_h_
#define _parser_utf8_h_

#include "parser.h"

#define UXML_IMPL utf8
int		UXML_IMPL_FUNC(parse)(uxml_parser_t *restrict ctx, int prolog);
#undef UXML_IMPL

#endif
