#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include <iostream.h>
#include <dos.h>
//const int size=1000;
typedef struct {long n; long *a;} aaa;

aaa * A;
aaa buff;
//long n;
long double gtdem;
long nho,i,j,size;
//-------------------------------
long re_alloc(aaa * A, long size )
{long k;
if(buff.a=(long*)calloc(size,sizeof(long)));
else
	{cout<<"\nerror "<<size;
	return 1;
	}
buff.n=A->n;
for(k=0;k<=buff.n;k++)
	buff.a[k]=A->a[k];
//free(A->a);
//free(A);
A=&buff;
return 0;
}
//-------------------------------
long nhan1(aaa *A,long *x)		//nhan A voi so x
{long i;
for(i=0;i<=A->n;i++)
	{gtdem=nho+ (A->a[i] * (long double)(*x));
	nho=(long)	(gtdem /      1000000000);
	A->a[i]=(long)	(gtdem - (nho*1000000000));
	}
if (nho>0)
	{if((A->n)>=size)
		{
		size += size/10;
		if(re_alloc(A,size)) return 1;
		}
	(A->n)=i;
	A->a[i]=nho;
	nho=0;
	}
return 0;
}
//-------------------------------
void out(FILE *ft,aaa *A)		//in A ra file "ft"
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

//A->n=0;A->a[0]=1;
cout<<"\nnhap size cua mang a:";	//nhap kich co vung nho
cin>>size;
cin.ignore();
A=(aaa*)calloc(sizeof(aaa),1);
A->a=(long*)realloc(A->a,size*sizeof(long));	//cap vung nho
A->n=0;A->a[0]=1;
cout<<"\nnhap x:";			//nhap x
cin>>x;
cin.ignore();
for(j=1;j<=x;j++)			//tinh giai thua cua x
	if (nhan1(A,&j) !=0) return j;
ft=fopen(ftn,"a+");			//open file
fprintf(ft,"\n--------------");
fprintf(ft,"\n%ld!",x);
fprintf(ft,"\n%ld\n",A->n);
out(ft,A);				//xuat kq ra file

cout<<"\n"<<A->n;
//free(A->a);				//giai phong vung nho
fclose(ft);
return 0;
}
//-------------------------------
void main()
{char c='y';
while (c=='y')				//loop
	{cout<<"\n"<<gt();
	cout<<"\ncontinue?(y,n) ";
	cin>>c;
	cin.ignore();
	}
}