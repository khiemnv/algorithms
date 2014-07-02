#include<stdio.h>
#include<conio.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>
//--------------------------------------
long out_n(FILE * f)
{long n,cur;
cur=ftell(f);
fseek(f,0,0);
fread(&n,sizeof(long),1,f);
//	printf("\nn=%ld",n);
fseek(f,cur,0);
return n;
}
//--------------------------------------
void in_n(FILE * f,long n)
{long cur;
cur=ftell(f);
fseek(f,0,0);
fwrite(&n,sizeof(long),1,f);
fseek(f,cur,0);
}
//--------------------------------------
long out_ai(FILE * f,long i)
{long ai,cur;
cur=ftell(f);
fseek(f,sizeof(long)*(i+1),0);
fread(&ai,sizeof(long),1,f);
//	printf("\na%ld=%ld",i,ai);
fseek(f,cur,0);
return ai;
}
//--------------------------------------
void in_ai(FILE * f, long a,long i)
{long cur;
cur=ftell(f);
fseek(f,sizeof(long)*(i+1),0);
fwrite(&a,sizeof(long),1,f);
fseek(f,cur,0);
}
//--------------------------------------
void nhan(FILE *f,long x)
{long double gtdem; long ai,i,n,nho=0;
n=out_n(f);
for (i=0;i<=n;i++)
	{ai=out_ai(f,i);
	gtdem=nho+ (ai * (long double)x);
	nho=(long)	(gtdem /      1000000000);
	ai=(long)	(gtdem - (nho*1000000000));
	in_ai(f,ai,i);
	}
if (nho>0) {n++;in_ai(f,nho,n);nho=0;}
in_n(f,n);
//printf("\n%ld",n);
}
//--------------------------------------
void main()
{FILE *f;
long i,x;
char *fn="abc.txt";
clrscr();
f=fopen(fn,"wb+");
in_n(f,0);
in_ai(f,1,0);
printf("nhap x:");
fflush(stdin);
scanf("%ld",&x);
for (i=2;i<=x;i++)
	nhan(f,i);
//for (i=0;i<=out_n(f);i++){printf("\n%ld",out_ai(f,i));}
printf("\n%ld",out_n(f));
printf("\n%ld",out_ai(f,out_n(f)));
fclose(f);
getch();
}