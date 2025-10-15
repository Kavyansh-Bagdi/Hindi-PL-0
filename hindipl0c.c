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
#include "hashmap/hashmap.c"

#define CHECK_LHS	0
#define CHECK_RHS	1
#define CHECK_CALL	2
#define TOK_WRITEINT 'w'
#define TOK_WRITECHAR 'H'
#define TOK_READINT 'R'
#define TOK_READCHAR 'h'
#define TOK_INTO 'n'

#define PL0C_VERSION "1.0.0" 
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
static long proc = 0;

struct symtab {
	int depth;
	int type;
	wchar_t *name;
	struct symtab *next;
};
static struct symtab *head;

HashMap* map;

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
* Semantics.
*/

static void
initsymtab(void)
{
	struct symtab *new;

	if ((new = malloc(sizeof(struct symtab))) == NULL)
		error("malloc failed");

	new->depth = 0;
	new->type = TOK_PROCEDURE;
	new->name = L"main";
	new->next = NULL;

	head = new;
}

static void
addsymbol(int type)
{
    struct symtab *curr, *new;

    if (head == NULL) {
        head = malloc(sizeof(struct symtab));
        if (head == NULL)
            error("malloc failed");

        head->depth = depth - 1;
        head->type = type;
        head->name = wcsdup(token);
        if (head->name == NULL)
            error("malloc failed");

        head->next = NULL;
        return;
    }

    curr = head;
    while (1) {
        if (!wcscmp(curr->name, token)) {
            if (curr->depth == (depth - 1))
                error("duplicate symbol: %ls", token);
        }

        if (curr->next == NULL)
            break;

        curr = curr->next;
    }

    new = malloc(sizeof(struct symtab));
    if (new == NULL)
        error("malloc failed");

    new->depth = depth - 1;
    new->type = type;
    new->name = wcsdup(token);
    if (new->name == NULL)
        error("malloc failed");

    new->next = NULL;
    curr->next = new;
}

static void
destroysymbols(void)
{
    struct symtab *curr = head;
    struct symtab *prev = NULL;

    while (curr != NULL) {
        if (curr->depth >= depth) {
            struct symtab *to_free = curr;
            if (prev)
                prev->next = curr->next;
            else
                head = curr->next;

            curr = curr->next;
            free(to_free->name);
            free(to_free);
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
}

static void
symcheck(int check)
{
	struct symtab *curr, *ret = NULL;

	curr = head;
	while (curr != NULL) {
		if (!wcscmp(token, curr->name))
			ret = curr;
		curr = curr->next;
	}

	if (ret == NULL)
		error("undefined symbol: %s", token);

	switch (check) {
	case CHECK_LHS:
		if (ret->type != TOK_VAR)
			error("must be a variable: %s", token);
		break;
	case CHECK_RHS:
		if (ret->type == TOK_PROCEDURE)
			error("must not be a procedure: %s", token);
		break;
	case CHECK_CALL:
		if (ret->type != TOK_PROCEDURE)
			error("must be a procedure: %s", token);
	}
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

    while (*raw != L'\0' && (iswalpha(*raw) || iswdigit(*raw) || *raw == L'_' || is_devanagari_combining(*raw)))
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
	else if (!wcscmp(token, L"अंक_लिखें"))
		return TOK_WRITEINT;
	else if (!wcscmp(token, L"वर्ण_लिखें"))
		return TOK_WRITECHAR;
	else if (!wcscmp(token, L"अंक_पढ़ें"))
		return TOK_READINT;
	else if (!wcscmp(token, L"वर्ण_पढ़ें"))
		return TOK_READCHAR;
	else if (!wcscmp(token, L"में"))
		return TOK_INTO;

	insert(map,token);
	no_ident++;
	return TOK_IDENT;
}

static int
number(void) {
    wchar_t *start = raw;
    while (iswdigit(*raw) || isdigit(*raw))
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

    if (iswdigit(*raw) || isdigit(*raw))
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
 * Code generator.
 */

static void
aout(const wchar_t *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vwprintf(fmt, ap);
    va_end(ap);
}

static void
cg_end(void)
{

	aout(L"\n/* PL/0 compiler %s */\n", PL0C_VERSION);
}

static void
cg_const(void)
{
	aout(L"const long %s=", get(map,token));
}

static void
cg_semicolon(void)
{

	aout(L";\n");
}

static void
cg_symbol(void)
{
	switch (type) {
	case TOK_IDENT:
		aout(L"%s", get(map,token));
		break;
	case TOK_NUMBER:
		aout(L"%ls", token);
		break;
	case TOK_BEGIN:
		aout(L"{");
		break;
	case TOK_END:
		aout(L";}\n");
		break;
	case TOK_IF:
		aout(L"if(");
		break;
	case TOK_THEN:
	case TOK_DO:
		aout(L")");
		break;
	case TOK_ODD:
		aout(L"(");
		break;
	case TOK_WHILE:
		aout(L"while(");
		break;
	case TOK_EQUAL:
		aout(L"==");
		break;
	case TOK_COMMA:
		aout(L",");
		break;
	case TOK_ASSIGN:
		aout(L"=");
		break;
	case TOK_HASH:
		aout(L"!=");
		break;
	case TOK_LESSTHAN:
		aout(L"<");
		break;
	case TOK_GREATERTHAN:
		aout(L">");
		break;
	case TOK_PLUS:
		aout(L"+");
		break;
	case TOK_MINUS:
		aout(L"-");
		break;
	case TOK_MULTIPLY:
		aout(L"*");
		break;
	case TOK_DIVIDE:
		aout(L"/");
		break;
	case TOK_LPAREN:
		aout(L"(");
		break;
	case TOK_RPAREN:
		aout(L")");
	}
}

static void
cg_crlf(void)
{

	aout(L"\n");
}

static void
cg_var(void)
{

	aout(L"long %hs;\n", get(map,token));
}

static void
cg_procedure(void)
{
    if (proc == 0) {
        aout(L"int\n");
        aout(L"main(int argc, char *argv[])\n");
        aout(L"{\n");
        aout(L"    setlocale(LC_ALL, \"en_US.UTF-8\");\n");
    } else {
        aout(L"void\n");
        aout(L"%s(void)\n", get(map,token));
        aout(L"{\n");
    }
}

static void
cg_epilogue(void)
{

	aout(L";");

	if (proc == 0)
		aout(L"return 0;");

	aout(L"\n}\n\n");
}

static void
cg_readchar(void)
{
    aout(L"wint_t __wch = fgetwc(stdin);\n");
    aout(L"if (__wch == WEOF) {\n");
    aout(L"    /* treat EOF as -1 or handle error */\n");
    aout(L"    (void) fprintf(stderr, \"unexpected EOF when reading character\\n\");\n");
    aout(L"    exit(1);\n");
    aout(L"}\n");
    aout(L"%s = (long) __wch;\n", get(map,token));
}


cg_call(void)
{

	aout(L"%s();\n", get(map,token));
}

static void
cg_odd(void)
{

	aout(L")&1");
}

static void
cg_writechar(int isIdent)
{
	if(isIdent)
		aout(L"(void) fprintf(stdout, \"%%c\", (unsigned char) %s);", get(map,token));
	else	
		aout(L"(void) fprintf(stdout, \"%%c\", (unsigned char) %s);", token);
}

static void
cg_readchar(void)
{
    aout(L"wint_t __wch = fgetwc(stdin);\n");
    aout(L"if (__wch == WEOF) {\n");
    aout(L"    /* treat EOF as -1 or handle error */\n");
    aout(L"    (void) fprintf(stderr, \"unexpected EOF when reading character\\n\");\n");
    aout(L"    exit(1);\n");
    aout(L"}\n");
    aout(L"%s = (long) __wch;\n", get(map,token));
}


static void
cg_readint(void)
{
    /* Portable integer input using strtoll */
    aout(L"char __stdin[64];\n");
    aout(L"char *endptr;\n");
    aout(L"long long __val_ll;\n");
    aout(L"if (!fgets(__stdin, sizeof(__stdin), stdin)) { perror(\"fgets\"); exit(1); }\n");
    aout(L"if(__stdin[strlen(__stdin) - 1] == '\\n') __stdin[strlen(__stdin) - 1] = '\\0';\n");
    aout(L"errno = 0;\n");
    aout(L"__val_ll = strtoll(__stdin, &endptr, 10);\n");
    aout(L"if (endptr == __stdin || *endptr != '\\0' || (errno == ERANGE && (__val_ll == LLONG_MAX || __val_ll == LLONG_MIN))) {\n");
    aout(L"    (void) fprintf(stderr, \"invalid number: %%s\\n\", __stdin);\n");
    aout(L"    exit(1);\n");
    aout(L"}\n");
    aout(L"%s = (long) __val_ll;\n", get(map,token));
}

static void
cg_writeint(int isIdent)
{	
	if(isIdent)
		aout(L"(void) fprintf(stdout, \"%%ld\", (long) %s);", get(map,token));
	else
		aout(L"(void) fprintf(stdout, \"%%ld\", (long) %s);", token);
}

static void
cg_init(void)
{
	aout(L"#include <stdio.h>\n");
	aout(L"#include <wchar.h>\n");
	aout(L"#include <wctype.h>\n");
	aout(L"#include <locale.h>\n\n");
	aout(L"static char __stdin[24];\n\n");
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

static void term(void);

static void
expression(void)
{
	if (type == TOK_PLUS || type == TOK_MINUS) {
		cg_symbol();
		next();
	}
	term();
	while (type == TOK_PLUS || type == TOK_MINUS) {
		cg_symbol();
		next();
		term();
	}
}

static void
factor(void)
{
	switch (type) {
	case TOK_IDENT:
		symcheck(CHECK_RHS);
		/* Fallthru */
	case TOK_NUMBER:
		cg_symbol();
		next();
		break;
	case TOK_LPAREN:
		cg_symbol();
		expect(TOK_LPAREN);
		expression();
		if (type == TOK_RPAREN)
			cg_symbol();
		expect(TOK_RPAREN);
	}
}

static void
term(void)
{
	factor();
	while (type == TOK_MULTIPLY || type == TOK_DIVIDE) {
		cg_symbol();
		next();
		factor();
	}
}

static void
condition(void)
{
	if (type == TOK_ODD) {
		cg_symbol();
		expect(TOK_ODD);
		expression();
		cg_odd();
	} else {
		expression();

		switch (type) {
		case TOK_EQUAL:
		case TOK_HASH:
		case TOK_LESSTHAN:
		case TOK_GREATERTHAN:
			cg_symbol();
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
		symcheck(CHECK_LHS);
		cg_symbol();
		expect(TOK_IDENT);
		if (type == TOK_ASSIGN)
			cg_symbol();
		expect(TOK_ASSIGN);
		expression();
		break;
	case TOK_CALL:
		expect(TOK_CALL);
		if (type == TOK_IDENT) {
			symcheck(CHECK_CALL);
			cg_call();
		}
		expect(TOK_IDENT);
		break;
	case TOK_BEGIN:
		cg_symbol();
		expect(TOK_BEGIN);
		statement();
		while (type == TOK_SEMICOLON) {
			cg_semicolon();
			expect(TOK_SEMICOLON);
			statement();
		}
		if (type == TOK_END)
			cg_symbol();
		expect(TOK_END);
		break;
	case TOK_IF:
		cg_symbol();
		expect(TOK_IF);
		condition();
		if (type == TOK_THEN)
			cg_symbol();
		expect(TOK_THEN);
		statement();
		break;
	case TOK_WHILE:
		cg_symbol();
		expect(TOK_WHILE);
		condition();
		if (type == TOK_DO)
			cg_symbol();
		expect(TOK_DO);
		statement();
		break;
	case TOK_WRITEINT:
		expect(TOK_WRITEINT);
		if (type == TOK_IDENT) {
			if (type == TOK_IDENT)
				symcheck(CHECK_RHS);
			cg_writeint(1);
		}
		else if(type == TOK_NUMBER){
			cg_writeint(0);
		}

		if (type == TOK_IDENT)
			expect(TOK_IDENT);
		else if (type == TOK_NUMBER)
			expect(TOK_NUMBER);
		else
			error("writeInt takes an identifier or a number");

		break;
	case TOK_WRITECHAR:
		expect(TOK_WRITECHAR);
		if (type == TOK_IDENT) {
			if (type == TOK_IDENT)
				symcheck(CHECK_RHS);
			cg_writechar(1);
		}
		if(type == TOK_NUMBER) {
			cg_writechar(0);
		}

		if (type == TOK_IDENT)
			expect(TOK_IDENT);
		else if (type == TOK_NUMBER)
			expect(TOK_NUMBER);
		else
			error("writeChar takes an identifier or a number");

		break;
	case TOK_READINT:
		expect(TOK_READINT);
		if (type == TOK_INTO)
			expect(TOK_INTO);

		if (type == TOK_IDENT) {
			symcheck(CHECK_LHS);
			cg_readint();
		}

		expect(TOK_IDENT);

		break;
	case TOK_READCHAR:
		expect(TOK_READCHAR);
		if (type == TOK_INTO)
			expect(TOK_INTO);

		if (type == TOK_IDENT) {
			symcheck(CHECK_LHS);
			cg_readchar();
		}

		expect(TOK_IDENT);
	}
	
}

void block(void)
{
    if (depth++ > 1)
        error("nesting depth exceeded");

    if (type == TOK_CONST) {
        expect(TOK_CONST);
        if (type == TOK_IDENT) {
            addsymbol(TOK_CONST);
            cg_const();
        }
        expect(TOK_IDENT);
        expect(TOK_EQUAL);
        if (type == TOK_NUMBER) {
            cg_symbol();
            cg_semicolon();
        }
        expect(TOK_NUMBER);
        while (type == TOK_COMMA) {
            expect(TOK_COMMA);
            if (type == TOK_IDENT) {
                addsymbol(TOK_CONST);
                cg_const();
            }
            expect(TOK_IDENT);
            expect(TOK_EQUAL);
            if (type == TOK_NUMBER) {
                cg_symbol();
                cg_semicolon();
            }
            expect(TOK_NUMBER);
        }
        expect(TOK_SEMICOLON);
    }

    if (type == TOK_VAR) {
        expect(TOK_VAR);
        if (type == TOK_IDENT) {
            addsymbol(TOK_VAR);
            cg_var();
        }
        expect(TOK_IDENT);
        while (type == TOK_COMMA) {
            expect(TOK_COMMA);
            if (type == TOK_IDENT) {
                addsymbol(TOK_VAR);
                cg_var();
            }
            expect(TOK_IDENT);
        }
        expect(TOK_SEMICOLON);
        cg_crlf();
    }

    while (type == TOK_PROCEDURE) {
        proc = 1;

        expect(TOK_PROCEDURE);
        if (type == TOK_IDENT) {
            addsymbol(TOK_PROCEDURE);
            cg_procedure();             
        }
        expect(TOK_IDENT);
        expect(TOK_SEMICOLON);

        block();                        
        expect(TOK_SEMICOLON);

        cg_epilogue();                  

        proc = 0;
        destroysymbols();
    }

    if (proc == 0) {
        cg_procedure();                
    }

    statement();

    if (proc == 0) {
        cg_epilogue();                  
    }

    if (--depth < 0)
        error("nesting depth fell below 0");
}

static void 
parse(void) {
	cg_init();    
    next();
    block();
    expect(TOK_DOT);

    if ( type != 0) {
        error("extra tokens at end of file");
    }

	cg_end();
}

/*
 * Main.
 */

int
main(int argc, char *argv[])
{
	setlocale(LC_ALL, "en_US.UTF-8");
	map = create_hashmap(1000);
    wchar_t *startp;

	if (argc != 2) {
		(void) fputs("[INFO] Usage: hindipl0c file.hindi\n", stderr);
		exit(1);
	}

	readin(argv[1]);
	startp = raw;

	parse();

	free(startp);
	free(token);
	free_hashmap(map);
	return 0;
}
