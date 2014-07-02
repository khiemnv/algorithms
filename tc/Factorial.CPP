#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include <iostream.h>
//using namespace std;
#include <dos.h>

const int size=60000;
typedef struct {long n; long a[size];} aaa;

aaa * A;
aaa buff;
//long n;
long double gtdem;
long nho,i,j;
//-------------------------------
long nhan1(aaa *A,long *x)
{long i;
for(i=0;i<=A->n;i++)
	{gtdem=nho+ (A->a[i] * (long double)(*x));
	nho=(long)	(gtdem /      1000000000);
	A->a[i]=(long)	(gtdem - (nho*1000000000));
	}
if (nho>0)
	if((A->n)<size)
	{(A->n)=i;
	A->a[i]=nho;
	nho=0;
	}
	else 
	{cout<<"\nover A";
	return nho;
	}
return 0;
}
//-------------------------------
void out(FILE *ft,aaa *A)
{long d;
for (i=A->n;i>=0;i--)
	{
	d=10; j=8;
	while((A->a[i])>=d)	{j--;d=d* 10;}
	while(j>0)   	{fprintf(ft,"0");j--;}
	fprintf(ft,"%ld",A->a[i]);
	}
}
//-------------------------------
long gt()
{FILE *ft;
char *ftn="gt2.txt";
long x;

//A=(aaa*)calloc(sizeof(aaa),1);
A=&buff;
A->n=0;A->a[0]=1;
cout<<"\nnhap x:";

cin>>x;
cin.ignore();
for(j=1;j<=x;j++)
	if(nhan1(A,&j)!=0) 
	{
		return j;
	}
ft=fopen(ftn,"a+");
fprintf(ft,"\n--------------");
fprintf(ft,"\n%ld!",x);
fprintf(ft,"\n%ld\n",A->n);
out(ft,A);

cout<<"\n"<<A->n;
fclose(ft);
return 0;
}
//-------------------------------
void main()
{char c='y';
while (c=='y')
	{cout<<"\n"<<gt();
	cout<<"\ncontinue?(y,n) ";
	cin>>c;
	cin.ignore();
	}
}