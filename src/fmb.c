#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG
#ifdef DEBUG
#define log(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#else
#define log(fmt, ...) ((void)0)
#endif

#define MAXNUMS 1024

char *envstr(char *name)
{
	char *value;
	value = getenv(name);
	return value ? value : "";
}

int usage(char *name, int rc)
{
	fprintf(stderr, "Usage: %s [options]\n", name);
	fprintf(stderr, "Options:\n");

	fprintf(stderr, "\t-h, --help\n");
	fprintf(stderr, "\t\tShow this help message\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "\t-i, --input FILE\n");
	fprintf(stderr, "\t\tSpecify input file\n");
	fprintf(stderr, "\t\t[default: stdin]\n");
	fprintf(stderr, "\t\t[env: INPUT_FILE=%s]\n", envstr("INPUT_FILE"));
	fprintf(stderr, "\n");
	return rc;
}

void invalid_input(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
	usage("main", 1);
}

int parse_uint64(const char *input, uint64_t *res)
{
	char *endp;
	uint64_t out;
	errno = 0;
	out = strtoull(input, &endp, 0);
	if (*endp != 0 || errno == ERANGE)
		return 1;
	*res = out;
	return 0;
}

int parse_int(char *input, int *res)
{
	char *endp;
	long out;
	out = strtol(input, &endp, 0);
	if (*endp != 0)
		return 1;
	*res = (int)out;
	return 0;
}

char *fmb(uint64_t bytes, char *buf, size_t buflen)
{
	static const char *units[] = { "", "KB", "MB", "GB", "TB", "PB", "EB" };
	const size_t n_units = sizeof(units) / sizeof(units[0]);

	size_t idx = 0;
	__uint128_t scaled = (__uint128_t)bytes * 100u;
	uint64_t whole, frac;

	while (scaled >= ((__uint128_t)1024 * 100u) && idx < n_units - 1) {
		scaled = (scaled + 512u) / 1024u;
		++idx;
	}

	if (idx == 0) {
		snprintf(buf, buflen, "%" PRIu64 "%s", bytes, units[idx]);
		return buf;
	}

	whole = (uint64_t)(scaled / 100u);
	frac = (uint64_t)(scaled % 100u);

	if (frac == 0) {
		snprintf(buf, buflen, "%" PRIu64 "%s", whole, units[idx]);
	} else if (whole < 10) {
		if (frac % 10 == 0) {
			snprintf(buf, buflen, "%" PRIu64 ".%01" PRIu64 "%s",
				 whole, frac / 10, units[idx]);
		} else {
			snprintf(buf, buflen, "%" PRIu64 ".%02" PRIu64 "%s",
				 whole, frac, units[idx]);
		}
	} else if (whole < 100) {
		uint64_t tenths = (frac + 5) / 10;
		if (tenths == 0) {
			snprintf(buf, buflen, "%" PRIu64 "%s", whole,
				 units[idx]);
		} else {
			snprintf(buf, buflen, "%" PRIu64 ".%01" PRIu64 "%s",
				 whole, tenths, units[idx]);
		}
	} else {
		if (frac >= 50)
			++whole;
		snprintf(buf, buflen, "%" PRIu64 "%s", whole, units[idx]);
	}
	return buf;
}

int main(int argc, char **argv)
{

	static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

	int option_index = 0;
	int c;
	int rc;

	char *numstr = 0;
	uint64_t *nums;
	nums = calloc(MAXNUMS, sizeof(*nums));

	while ((c = getopt_long(argc, argv, "h", long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(*argv, 0);
			return 0;
		case '?':
			return 1;
		default:
			break;
		}
	}

	int i = 0;
	if (optind == argc) {
		ssize_t read = 0;
		while (i < MAXNUMS && fscanf(stdin, "%"PRIu64, &nums[i]) == 1)
			++i;
	} else {
		for (; i + optind < argc; ++i) {
			if ((rc = parse_uint64(argv[i + optind], &nums[i])))
				invalid_input("Malformed input numer: '%s'",
					      argv[i + optind]);
		}
	}

	char buf[64] = { 0 };
	for (; i--;)
		printf("%s\n", fmb(*nums++, buf, 32));

	return 0;
}
