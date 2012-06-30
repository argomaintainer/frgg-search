#include "frgg.h"


void
do_log(int log_type, const char *fmt, ...)
{
	const char *logfile;
	va_list ap;
	char msgbuf[BUFLEN];
	FILE *fp;
	
	switch (log_type) {
	case LOG_QUERY:
		logfile = LOG_DIR"/query.log";
		break;
	case LOG_TRACE:	
		logfile = LOG_DIR"/trace.log";
		break;
	default:
		ERROR("unknown log type");
		return;
	}

	fp = fopen(logfile, "a");
	if (fp == NULL) {
		ERROR1("fopen %s failed", logfile);
		return;
	}

	va_start(ap, fmt);
	vsnprintf(msgbuf, BUFLEN, fmt, ap);
	va_end(ap);

	fputs(msgbuf, fp);
	fputc('\n', fp);
	fclose(fp);
}


void
do_err(const char *func, const char *file, int line, const char *fmt, ...)
{
	char *error_file = LOG_DIR"/error";
	char msgbuf[BUFLEN];
	char errbuf[BUFLEN];
	va_list ap;
	FILE *fp;
	time_t now;

	fp = fopen(error_file, "a");
	if (fp == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		return;
	}
		     
	va_start(ap, fmt);
	vsnprintf(msgbuf, BUFLEN, fmt, ap);
	va_end(ap);

	time(&now);
	if (errno) {
		snprintf(errbuf, sizeof(errbuf), "ERROR: %24.24s %s() (%s::%d): %s "
			 "(system error is '%s')", ctime(&now), func, file, line, msgbuf,
			 strerror(errno));
		errno = 0;
	} else {
		snprintf(errbuf, sizeof(errbuf), "ERROR: %24.24s %s() (%s::%d): %s",
			 ctime(&now), func, file, line, msgbuf);
	}
	fputs(errbuf, fp);
	fputc('\n', fp);
	fclose(fp);
}

void
do_debug(const char *func, const char *file, int line, const char *fmt, ...)
{
	char *debug_file = LOG_DIR"/debug";
	char msgbuf[BUFLEN], errbuf[BUFLEN];
	time_t now;
	va_list ap;
	FILE *fp;

	fp = fopen(debug_file, "a");
	if (fp == NULL)
		return;
	
	va_start(ap, fmt);
	vsnprintf(msgbuf, sizeof(msgbuf), fmt, ap);
	va_end(ap);

	time(&now);
	if (errno) {
		snprintf(errbuf, sizeof(errbuf), "DEBUG: %24.24s %s() (%s::%d): %s "
			 "(system error is '%s')", ctime(&now), func, file, line,
			 msgbuf, strerror(errno));
		errno = 0;
	} else {
		snprintf(errbuf, sizeof(errbuf), "DEBUG: %24.24s %s() (%s::%d): %s",
			 ctime(&now), func, file, line, msgbuf);
	}
	fputs(errbuf, fp);
	fputc('\n', fp);
	fclose(fp);
}
