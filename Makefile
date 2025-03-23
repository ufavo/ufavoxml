NAME = ufavoxml
#config
DESTDIR?=
PREFIX = /usr/local
#LDFLAGS += 
CFLAGS?=-O3 
CFLAGS += -std=c99 -pedantic -Wall -Wextra -Wrestrict -fPIC -Iutf8-utf16-converter/converter/include
CC?=gcc

SRC = src/parser.c src/parser_utf16be.c src/parser_utf16le.c src/parser_utf8.c utf8-utf16-converter/converter/src/converter.c
OBJ = ${SRC:.c=.o}

all: lib$(NAME).a

.c.o:
	$(CC) -c ${CFLAGS} $< -o $@

options:
	@echo $(NAME) build options:
	@echo "CFLAGS   = ${CFLAGS}"
#	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo ""
	@echo $(NAME) install options:
	@echo "DESTDIR  = ${DESTDIR}"
	@echo "PREFIX   = ${PREFIX}"

clean:
	rm -f $(OBJ) lib$(NAME).a

lib$(NAME).a: $(OBJ)
	ar rcs $@ $(OBJ)

#$(NAME): $(OBJ)
#	$(CC) ${OBJ} -o $@ $(LDFLAGS) 

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/lib/
	mkdir -p $(DESTDIR)$(PREFIX)/include/
	cp -f lib$(NAME).a $(DESTDIR)$(PREFIX)/lib/
	cp -rf include/$(NAME) $(DESTDIR)$(PREFIX)/include/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/lib/lib$(NAME).a
	rm -rf $(DESTDIR)$(PREFIX)/include/$(NAME)/


.PHONY: all options clean install uninstall
