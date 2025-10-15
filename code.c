#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>

static char __stdin[24];

void h(void)
{
    long a1;
    long a2;

    a1 = 0;
    a2 = 7;
    (void) fprintf(stdout, "%d", (unsigned char) a1);
    a2 = a2 + 2;
    a2 = a2 + 7;
    while (a1 < 2) {
        (void) fprintf(stdout, "%d", (unsigned char) a1);
        a1 = a1 + 1;
    }
    a2 = a2 + 3;
    (void) fprintf(stdout, "%d", (unsigned char) a2);
    (void) fprintf(stdout, "%d", (unsigned char) 4);
    (void) fprintf(stdout, "%d", (unsigned char) 3);
    a2 = a2 + 8;
    (void) fprintf(stdout, "%d", (unsigned char) a2);
    a2 = a2 - 8;
    (void) fprintf(stdout, "%d", (unsigned char) a2);
    a2 = a2 + 3;
    (void) fprintf(stdout, "%d", (unsigned char) a2);
    a2 = a2 - (3 * 2);
    (void) fprintf(stdout, "%d", (unsigned char) a2);
    a2 = a2 - 8;
    (void) fprintf(stdout, "%d", (unsigned char) a2);
    a2 = a2 / 3;
    (void) fprintf(stdout, "%d", (unsigned char) a2);
    (void) fprintf(stdout, "%d", (unsigned char) 1);
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "en_US.UTF-8");
    h();
    return 0;
}
/* PL/0 compiler 1.0.0 */
