# UfavoXML
UfavoXML is a callback based XML parser library written in C99.

## Features
- Can read from a data stream (a callback function is called whenever more data is required);
- Doesn't allocate any memory (the user must provide a buffer);
- UTF-8 and UTF-16 support;
- Self-closing elements;
- Comments;
- XML entity references;
- HTML character entities in hexadecimal and decimal codes only (E.g `&#969;` and `&#x3C9;` are interpreted as `ω` while a HTML named entity such as `&omega;` will be treated as normal text);
- Loose attributes (E.g. instead of erroring out with something like `<element foo="123" bar baz/>` the parser interpret it as `<element foo="123" bar="" baz=""/>`).

## Known limitations
- Only accepts ASCII characters in XML Names (Being the lead character `[A-Z]`,`[a-z]`,`[0-9]` or `_` followed by any amount of `[A-Z]`, `[a-z]`, `[0-9]`, `-`, `.`, `:` or `_`);
- XML Prolog attributes are completely ignored;
- CDATA sections are not supported yet;
- No support for processing instructions.

## TODO
- [ ] Handle CDATA sections
- [ ] Add build option to disable UTF-16 support (can decrease the lib size by ~ 2/3)
- [ ] Add build instructions for platforms other then unix
- [ ] Documentation
- [ ] Tests
- [ ] Benchmarks?
- [ ] Complete the serializer implementation

## Building
### Unix
#### Clone
```sh
git clone --recurse-submodules https://github.com/ufavo/ufavoxml && cd ufavoxml
```
#### Options
The current build and install options are shown by running
```sh
make options
```
those can be overridden with environment variables before running `make`.

**Important:** *Build with at least `-O2` optimization level to ensure that the `inline` functions get inlined. This ensures good performance and avoid stack overflowing when dealing with large files. (The default optimization level is `-O3`).*

#### Build
```sh
make -j4
```

## Installing
```sh
[sudo] make install
```

## Usage

See [`test/example.c`](/test/example.c).
To compile and run the example, after building the lib:
```sh
cd test 
gcc example.c ../libufavoxml.a -o example 
./example
```

## License
This library is licensed under MIT.
Check the LICENCE file.
