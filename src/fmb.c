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
#define MAXPREC 3

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

	fprintf(stderr, "\t-p, --precision <PRECISION>\n");
	fprintf(stderr,
		"\t\tThe number of decimal places to output for fractional values.\n");
	fprintf(stderr, "\t\tAccepted range: 0=< p <=3\n");
	fprintf(stderr, "\t\t[default: 2]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-s, --separator <STRING>\n");
	fprintf(stderr,
		"\t\tWhen provided `separator` will be prefixed to any units\n");
	fprintf(stderr,
		"\t\tE.g. `fmb 2048` -> 2KB ; fmb -s ' ' 2048 -> 2 KB\n");
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

// todo: base as a int is gross. move this work to comp time
char *fmb(uint64_t bytes, char *buf, size_t buflen, int precision, int base,
	  char *sep)
{
	static const char *units[] = { "", "K", "M", "G", "T", "P", "E" };
	static const uint64_t b10[] = { 1ULL, 10ULL, 100ULL, 1000ULL };
	const size_t n_units = sizeof(units) / sizeof(units[0]);

	if (!sep)
		sep = "";

	if (base != 1000)
		base = 1024;

	if (precision > MAXPREC)
		precision = MAXPREC;

	size_t idx = 0;
	__uint128_t scaled = (__uint128_t)bytes * b10[precision];

	uint64_t whole, frac;

	while (scaled >= (__uint128_t)base * b10[precision] &&
	       idx < n_units - 1) {
		scaled = (scaled + base / 2) / base;
		++idx;
	}

	if (!idx || !precision) {
		snprintf(buf, buflen, "%" PRIu64 "%s%s",
			 (uint64_t)scaled / b10[precision], sep, units[idx]);
		return buf;
	}

	whole = (uint64_t)(scaled / b10[precision]);
	frac = (uint64_t)(scaled % b10[precision]);

	while (precision && frac % 10 == 0) {
		frac /= 10;
		--precision;
	}

	if (precision) {
		snprintf(buf, buflen, "%" PRIu64 ".%0*" PRIu64 "%s%s", whole,
			 precision, frac, sep, units[idx]);
	} else {
		snprintf(buf, buflen, "%" PRIu64 "%s%s", whole, sep,
			 units[idx]);
	}
	return buf;
}

int main(int argc, char **argv)
{
	static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "precision", required_argument, 0, 'p' },
		{ "separator", required_argument, 0, 's' },
		{ 0, 0, 0, 0 }
	};

	int option_index = 0;
	int c;
	int rc;

	char *numstr = 0;
	uint64_t *nums;
	nums = calloc(MAXNUMS, sizeof(*nums));

	int precision = 2;
	char *separator = "";
	while ((c = getopt_long(argc, argv, "hp:s:", long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(*argv, 0);
			return 0;
		case 's':
			separator = optarg;
			break;
		case 'p':
			if ((parse_int(optarg, &precision)))
				invalid_input("Invalid precision string: '%s'",
					      optarg);
			break;
		case '?':
			return 1;
		default:
			break;
		}
	}

	int i = 0;
	if (optind == argc) {
		ssize_t read = 0;
		while (i < MAXNUMS && fscanf(stdin, "%" PRIu64, &nums[i]) == 1)
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
		printf("%s\n",
		       fmb(*nums++, buf, 32, precision, 1024, separator));

	return 0;
}
