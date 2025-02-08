#ifndef _parser_utf16_h_
#define _parser_utf16_h_

#include "parser.h"

#define UXML_IMPL utf16le
int		UXML_IMPL_FUNC(parse)(uxml_parser_t *restrict ctx, int prolog);
#undef UXML_IMPL

#define UXML_IMPL utf16be
int		UXML_IMPL_FUNC(parse)(uxml_parser_t *restrict ctx, int prolog);
#undef UXML_IMPL

#endif
