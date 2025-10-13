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
#include <wchar.h>
#include <wctype.h>
#include <locale.h>

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
* program     = block ";" .
* 
* block       = [ "नियत" ident "=" number { "," ident "=" number } ";" ]
*               [ "चर" ident { "," ident } ";" ]
*               { "प्रक्रिया" ident ";" block ";" } statement .
* 
* statement   = [ ident ":=" expression
*               | "आह्वान" ident
*               | "आरम्भ" statement { ";" statement } "समापन"
*               | "यदि" condition "तो" statement
*               | "जबतक" condition "करो" statement ] .
* 
* condition   = "विषम" expression
*               | expression ( "=" | "#" | "<" | ">" ) expression .
* 
* expression  = [ "+" | "-" ] term { ( "+" | "-" ) term } .
* term        = factor { ( "*" | "/" ) factor } .
* factor      = ident
*               | number
*               | "(" expression ")" .
* ident       = "अ-ह" { "अ-ह0-9_" } .
* number      = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" .
*/ 

static wchar_t *raw, *token;
static int depth, type;
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

	char *raw_bytes;
    if ((raw_bytes = malloc(st.st_size + 1)) == NULL)
        error("malloc failed");

    if (read(fd, raw_bytes, st.st_size) != st.st_size)
        error("couldn't read %s", file);
    raw_bytes[st.st_size] = '\0';

    size_t wlen = mbstowcs(NULL, raw_bytes, 0); 
    if (wlen == (size_t)-1)
        error("invalid multibyte sequence");

    raw = malloc((wlen + 1) * sizeof(wchar_t));
    if (!raw)
        error("malloc failed");

    mbstowcs(raw, raw_bytes, wlen + 1);
    free(raw_bytes);

	token = malloc((wlen + 1) * sizeof(wchar_t));
	if (!token)
		error("malloc failed for token");

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

static int is_devanagari_combining(wchar_t c) {
    return (c >= 0x093A && c <= 0x094F) ||
           (c >= 0x0951 && c <= 0x0954) ||
           (c >= 0x0962 && c <= 0x0963);
}

static int
ident(void)
{
    wchar_t *start = raw;
    
    if (!iswalpha(*raw) && *raw != L'_')
        error("invalid identifier start: %lc", *raw);

    raw++;

    while (iswalpha(*raw) || iswdigit(*raw) || *raw == L'_' || is_devanagari_combining(*raw))
        raw++;

    size_t len = raw - start;
    wcsncpy(token, start, len);
    token[len] = L'\0';

	if (!wcscmp(token, L"नियत"))
		return TOK_CONST;
	else if (!wcscmp(token, L"चर"))
		return TOK_VAR;
	else if (!wcscmp(token, L"प्रक्रिया"))
		return TOK_PROCEDURE;
	else if (!wcscmp(token, L"आह्वान"))
		return TOK_CALL;
	else if (!wcscmp(token, L"आरम्भ"))
		return TOK_BEGIN;
	else if (!wcscmp(token, L"समापन"))
		return TOK_END;
	else if (!wcscmp(token, L"यदि"))
		return TOK_IF;
	else if (!wcscmp(token, L"तो"))
		return TOK_THEN;
	else if (!wcscmp(token, L"जबतक"))
		return TOK_WHILE;
	else if (!wcscmp(token, L"करो"))
		return TOK_DO;
	else if (!wcscmp(token, L"विषम"))
		return TOK_ODD;

	return TOK_IDENT;
}

static int
number(void) {
    wchar_t *start = raw;
    while (iswdigit(*raw))
        raw++;

    size_t len = raw - start;
    wcsncpy(token, start, len);
    token[len] = L'\0';

    return TOK_NUMBER;
}

static int
lex(void)
{
again:
    while (*raw == L' ' || *raw == L'\t' || *raw == L'\n') {
        if (*raw++ == L'\n')
            ++line;
    }

    if (iswalpha(*raw) || *raw == L'_')
        return ident();

    if (iswdigit(*raw))
        return number();

    switch (*raw) {
    case L'{':
        comment();
        goto again;
    case L'.': case L'=': case L',': case L';':
    case L'#': case L'<': case L'>': case L'+':
    case L'-': case L'*': case L'/': case L'(':
    case L')':
        return *raw++;
    case L':':
        if (*++raw != L'=')
            error("unknown token: ':%lc'", *raw);
        ++raw;
        return TOK_ASSIGN;
    case L'\0':
        return 0;
    default:
        error("unknown token: '%lc'", *raw);
    }

    return 0;
}


/*
 * Parser.
 */

static void
next(void)
{

	type = lex();
	// ++raw;
}

static void
expect(int match)
{

	if (match != type)
		error("syntax error");

	next();
}

static void expression(void);

static void
factor(void)
{
	switch (type) {
	case TOK_IDENT:
	case TOK_NUMBER:
		next();
		break;
	case TOK_LPAREN:
		expect(TOK_LPAREN);
		expression();
		expect(TOK_RPAREN);
	}
}

static void
term(void)
{

	factor();

	while (type == TOK_MULTIPLY || type == TOK_DIVIDE) {
		next();
		factor();
	}
}

static void
expression(void)
{

	if (type == TOK_PLUS || type == TOK_MINUS)
		next();

	term();

	while (type == TOK_PLUS || type == TOK_MINUS) {
		next();
		term();
	}
}

static void
condition(void)
{

	if (type == TOK_ODD) {
		expect(TOK_ODD);
		expression();
	} else {
		expression();

		switch (type) {
		case TOK_EQUAL:
		case TOK_HASH:
		case TOK_LESSTHAN:
		case TOK_GREATERTHAN:
			next();
			break;
		default:
			error("invalid conditional");
		}

		expression();
	}
}

static void
statement(void)
{

	switch (type) {
	case TOK_IDENT:
		expect(TOK_IDENT);
		expect(TOK_ASSIGN);
		expression();
		break;
	case TOK_CALL:
		expect(TOK_CALL);
		expect(TOK_IDENT);
		break;
	case TOK_BEGIN:
		expect(TOK_BEGIN);
		statement();
		while (type == TOK_SEMICOLON) {
			expect(TOK_SEMICOLON);
			statement();
		}
		expect(TOK_END);
		break;
	case TOK_IF:
		expect(TOK_IF);
		condition();
		expect(TOK_THEN);
		statement();
		break;
	case TOK_WHILE:
		expect(TOK_WHILE);
		condition();
		expect(TOK_DO);
		statement();
	}
}

static void 
block(void) {
    if ( depth++ > 1) {
        error("nesting depth exceeded");
    }

    if (type == TOK_CONST) {
		expect(TOK_CONST);
		expect(TOK_IDENT);
		expect(TOK_EQUAL);
		expect(TOK_NUMBER);
		while (type == TOK_COMMA) {
			expect(TOK_COMMA);
			expect(TOK_IDENT);
			expect(TOK_EQUAL);
			expect(TOK_NUMBER);
		}
		expect(TOK_SEMICOLON);
	}

    if (type == TOK_VAR) {
		expect(TOK_VAR);
		expect(TOK_IDENT);
		while (type == TOK_COMMA) {
			expect(TOK_COMMA);
			expect(TOK_IDENT);
		}
		expect(TOK_SEMICOLON);
	}

    while (type == TOK_PROCEDURE) {
		expect(TOK_PROCEDURE);
		expect(TOK_IDENT);
		expect(TOK_SEMICOLON);

		block();

		expect(TOK_SEMICOLON);
	}

    statement();

	if (--depth < 0)
		error("nesting depth fell below 0");
}

static void 
parse(void) {
    next();
    block();
    expect(TOK_DOT);

    if ( type != 0) {
        error("extra tokens at end of file");
    }
}

/*
 * Main.
 */

int
main(int argc, char *argv[])
{
	setlocale(LC_ALL, "en_US.UTF-8");
    wchar_t *startp;

	if (argc != 2) {
		(void) fputs("[INFO] Usage: pl0c file.hindi\n", stderr);
		exit(1);
	}

	readin(argv[1]);
	startp = raw;

	parse();

	free(startp);
	free(token);

	return 0;
}
