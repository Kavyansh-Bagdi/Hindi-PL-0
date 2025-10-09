
#include <sys/stat.h>

#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#define TOK_IDENT	'I'
#define TOK_NUMBER	'N'
#define TOK_CONST	'C'
#define TOK_VAR		'V'
#define TOK_PROCEDURE	'P'
#define TOK_CALL	'c'
#define TOK_BEGIN	'B'
#define TOK_END		'E'
#define TOK_IF		'i'
#define TOK_THEN	'T'
#define TOK_WHILE	'W'
#define TOK_DO		'D'
#define TOK_ODD		'O'
#define TOK_DOT		'.'
#define TOK_EQUAL	'='
#define TOK_COMMA	','
#define TOK_SEMICOLON	';'
#define TOK_ASSIGN	':'
#define TOK_HASH	'#'
#define TOK_LESSTHAN	'<'
#define TOK_GREATERTHAN	'>'
#define TOK_PLUS	'+'
#define TOK_MINUS	'-'
#define TOK_MULTIPLY	'*'
#define TOK_DIVIDE	'/'
#define TOK_LPAREN	'('
#define TOK_RPAREN	')'

/* 
* hindi pl0c -- PL/0 Compiler
* program     = block "|" |
* 
* block       = [ "नियत" ident "=" number { "," ident "=" number } ";" ]
*               [ "चर" ident { "," ident } ";" ]
*               { "प्रक्रिया" ident ";" block ";" } statement |
* 
* statement   = [ ident ":=" expression
*               | "आह्वान" ident
*               | "आरम्भ" statement { ";" statement } "समापन"
*               | "यदि" condition "तो" statement
*               | "जबतक" condition "करो" statement ] |
* 
* condition   = "विषम" expression
*               | expression ( "=" | "#" | "<" | ">" ) expression |
* 
* expression  = [ "+" | "-" ] term { ( "+" | "-" ) term } |
* term        = factor { ( "*" | "/" ) factor } |
* factor      = ident
*               | number
*               | "(" expression ")" |
* ident       = "अ-ह" { "अ-ह0-9_" } |
* number      = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" |
*/ 

static char *raw, *token;
static int type;
static size_t line = 1;

/*
 * Misc. functions.
 */

static void
error(const char *fmt, ...)
{
	va_list ap;

	(void) fprintf(stderr, "[ERROR] %lu: ", line);

	va_start(ap, fmt);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);

	(void) fputc('\n', stderr);

	exit(1);
}

static void
readin(char *file)
{
	int fd;
	struct stat st;

	if (strrchr(file, '.') == NULL)
		error("file must end in '.hindi'");

	if (!!strcmp(strrchr(file, '.'), ".hindi"))
		error("file must end in '.hindi'");

	if ((fd = open(file, O_RDONLY)) == -1)
		error("couldn't open %s", file);

	if (fstat(fd, &st) == -1)
		error("couldn't get file size");

	if ((raw = malloc(st.st_size + 1)) == NULL)
		error("malloc failed");

	if (read(fd, raw, st.st_size) != st.st_size)
		error("couldn't read %s", file);
	raw[st.st_size] = '\0';

	(void) close(fd);
}

/*
 * Lexer.
 */

static void
comment(void)
{
	int ch;

	while ((ch = *raw++) != '}') {
		if (ch == '\0')
			error("unterminated comment");

		if (ch == '\n')
			++line;
	}
}

static int
ident(void)
{
	char *p;
	size_t i, len;

	p = raw;
	while (isalnum(*raw) || *raw == '_')
		++raw;

	len = raw - p;

	--raw;

	free(token);

	if ((token = malloc(len + 1)) == NULL)
		error("malloc failed");

	for (i = 0; i < len; i++)
		token[i] = *p++;
	token[i] = '\0';

	if (!strcmp(token, "नियत"))
		return TOK_CONST;
	else if (!strcmp(token, "चर"))
		return TOK_VAR;
	else if (!strcmp(token, "प्रक्रिया"))
		return TOK_PROCEDURE;
	else if (!strcmp(token, "आह्वान"))
		return TOK_CALL;
	else if (!strcmp(token, "आरम्भ"))
		return TOK_BEGIN;
	else if (!strcmp(token, "समापन"))
		return TOK_END;
	else if (!strcmp(token, "यदि"))
		return TOK_IF;
	else if (!strcmp(token, "तो"))
		return TOK_THEN;
	else if (!strcmp(token, "जबतक"))
		return TOK_WHILE;
	else if (!strcmp(token, "करो"))
		return TOK_DO;
	else if (!strcmp(token, "विषम"))
		return TOK_ODD;

	return TOK_IDENT;
}

long number() {
    char *endptr;
    const char *errstr = NULL;
    errno = 0;

    long val = strtol(token, &endptr, 10);

    if (endptr == token || *endptr != '\0')
        errstr = "invalid";
    else if ((val == LONG_MAX || val == LONG_MIN) && errno == ERANGE)
        errstr = "out of range";

    if (errstr)
        fprintf(stderr, "number parse error: %s\n", errstr);

    return val;
}

static int
lex(void)
{

again:
	/* Skip whitespace.  */
	while (*raw == ' ' || *raw == '\t' || *raw == '\n') {
		if (*raw++ == '\n')
			++line;
	}

	if (isalpha(*raw) || *raw == '_')
		return ident();

	if (isdigit(*raw))
		return number();

	switch (*raw) {
	case '{':
		comment();
		goto again;
	case '.':
	case '=':
	case ',':
	case ';':
	case '#':
	case '<':
	case '>':
	case '+':
	case '-':
	case '*':
	case '/':
	case '(':
	case ')':
		return (*raw);
	case ':':
		if (*++raw != '=')
			error("unknown token: ':%c'", *raw);

		return TOK_ASSIGN;
	case '\0':
		return 0;
	default:
		error("unknown token: '%c'", *raw);
	}

	return 0;
}

/*
 * Parser.
 */

static void
parse(void)
{

	while ((type = lex()) != 0) {
		++raw;
		(void) fprintf(stdout, "%lu|%d\t", line, type);
		switch (type) {
		case TOK_IDENT:
		case TOK_NUMBER:
		case TOK_CONST:
		case TOK_VAR:
		case TOK_PROCEDURE:
		case TOK_CALL:
		case TOK_BEGIN:
		case TOK_END:
		case TOK_IF:
		case TOK_THEN:
		case TOK_WHILE:
		case TOK_DO:
		case TOK_ODD:
			(void) fprintf(stdout, "%s", token);
			break;
		case TOK_DOT:
		case TOK_EQUAL:
		case TOK_COMMA:
		case TOK_SEMICOLON:
		case TOK_HASH:
		case TOK_LESSTHAN:
		case TOK_GREATERTHAN:
		case TOK_PLUS:
		case TOK_MINUS:
		case TOK_MULTIPLY:
		case TOK_DIVIDE:
		case TOK_LPAREN:
		case TOK_RPAREN:
			(void) fputc(type, stdout);
			break;
		case TOK_ASSIGN:
			(void) fputs(":=", stdout);
		}
		(void) fputc('\n', stdout);
	}
}

/*
 * Main.
 */

int
main(int argc, char *argv[])
{
	char *startp;

	if (argc != 2) {
		(void) fputs("[INFO] Usage: pl0c file.hindi\n", stderr);
		exit(1);
	}

	readin(argv[1]);
	startp = raw;

	parse();

	free(startp);

	return 0;
}
