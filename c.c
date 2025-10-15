void
h(void)
{
long a1;
long a2;

a1=0;
a2=7;
(void) fprintf(stdout, "%c", (unsigned char) x);;
a2=a2+2;
a2=a2+7;
while(a1<2){
(void) fprintf(stdout, "%c", (unsigned char) x);;
a1=a1+1;
}
;
a2=a2+3;
(void) fprintf(stdout, "%c", (unsigned char) x);;
(void) fprintf(stdout, "%c", (unsigned char) 4);;
(void) fprintf(stdout, "%c", (unsigned char) 3);;
a2=a2+8;
(void) fprintf(stdout, "%c", (unsigned char) x);;
a2=a2-8;
(void) fprintf(stdout, "%c", (unsigned char) x);;
a2=a2+3;
(void) fprintf(stdout, "%c", (unsigned char) x);;
a2=a2-(3*2);
(void) fprintf(stdout, "%c", (unsigned char) x);;
a2=a2-8;
(void) fprintf(stdout, "%c", (unsigned char) x);;
a2=a2/3;
(void) fprintf(stdout, "%c", (unsigned char) x);;
(void) fprintf(stdout, "%c", (unsigned char) 1);;
}
int
main(int argc, char *argv[])
{
h();
}
