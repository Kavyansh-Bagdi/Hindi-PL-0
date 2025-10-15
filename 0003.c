#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>

static char __stdin[24];

void
h(void)
{
long i;
long x;

{i=0;
x=7;
(void) fprintf(stdout, "%d", (unsigned char) x);;
x=x+2;
x=x+7;
while(i<2){(void) fprintf(stdout, "%d", (unsigned char) x);;
i=i+1;}
;
x=x+3;
(void) fprintf(stdout, "%d", (unsigned char) x);;
(void) fprintf(stdout, "%d", (unsigned char) 4);;
(void) fprintf(stdout, "%d", (unsigned char) 3);;
x=x+8;
(void) fprintf(stdout, "%d", (unsigned char) x);;
x=x-8;
(void) fprintf(stdout, "%d", (unsigned char) x);;
x=x+3;
(void) fprintf(stdout, "%d", (unsigned char) x);;
x=x-(3*2);
(void) fprintf(stdout, "%d", (unsigned char) x);;
x=x-8;
(void) fprintf(stdout, "%d", (unsigned char) x);;
x=x/3;
(void) fprintf(stdout, "%d", (unsigned char) x);;
(void) fprintf(stdout, "%d", (unsigned char) 1);;}
;
}

int
main(int argc, char *argv[])
{
    setlocale(LC_ALL, "en_US.UTF-8");
{h();
;}
;return 0;
}


/* PL/0 compiler 1.0.0 */
