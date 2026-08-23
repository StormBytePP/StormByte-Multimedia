/* Minimal gzip(1) for FFmpeg Meson on Windows: -nc9 → stdout gzip stream. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#if defined(_WIN32)
#  include <io.h>
#  include <fcntl.h>
#  ifndef _O_BINARY
#    define _O_BINARY 0x8000
#  endif
#endif

static int level = Z_DEFAULT_COMPRESSION;
static int to_stdout = 0;

static int gzip_stream(FILE *in, FILE *out)
{
	int ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char inbuf[16384];
	unsigned char outbuf[16384];

	memset(&strm, 0, sizeof(strm));
	ret = deflateInit2(&strm, level, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
	if (ret != Z_OK)
		return 1;

	do {
		strm.avail_in = (uInt)fread(inbuf, 1, sizeof(inbuf), in);
		if (ferror(in)) {
			deflateEnd(&strm);
			return 1;
		}
		flush = feof(in) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = inbuf;

		do {
			strm.avail_out = sizeof(outbuf);
			strm.next_out = outbuf;
			ret = deflate(&strm, flush);
			if (ret == Z_STREAM_ERROR) {
				deflateEnd(&strm);
				return 1;
			}
			have = sizeof(outbuf) - strm.avail_out;
			if (fwrite(outbuf, 1, have, out) != have || ferror(out)) {
				deflateEnd(&strm);
				return 1;
			}
		} while (strm.avail_out == 0);
	} while (flush != Z_FINISH);

	deflateEnd(&strm);
	return (ret == Z_STREAM_END) ? 0 : 1;
}

int main(int argc, char **argv)
{
	const char *path = NULL;
	FILE *in;
	int i;

	for (i = 1; i < argc; ++i) {
		const char *a = argv[i];
		if (a[0] != '-') {
			path = a;
			continue;
		}
		for (++a; *a; ++a) {
			if (*a >= '1' && *a <= '9')
				level = *a - '0';
			else if (*a == 'c')
				to_stdout = 1;
			else if (*a == 'n')
				; /* FFmpeg passes -n */
			else if (*a == 'h') {
				fprintf(stderr, "Usage: gzip [-nc] [-1..-9] [file]\n");
				return 0;
			}
		}
	}

	if (!to_stdout) {
		fprintf(stderr, "gzip: only -c (stdout) mode is implemented\n");
		return 1;
	}

	if (path) {
		in = fopen(path, "rb");
		if (!in) {
			perror(path);
			return 1;
		}
	} else {
		in = stdin;
#if defined(_WIN32)
		_setmode(_fileno(stdin), _O_BINARY);
#endif
	}

#if defined(_WIN32)
	_setmode(_fileno(stdout), _O_BINARY);
#endif

	if (gzip_stream(in, stdout) != 0) {
		fprintf(stderr, "gzip: compression failed\n");
		if (path)
			fclose(in);
		return 1;
	}
	if (path)
		fclose(in);
	return 0;
}
