//trương trình in kết quả ra file "D:\gaithua.txt"

#include<stdio.h>
#include<conio.h>
#include<string.h>
#include <stdlib.h>
#include <Windows.h>

typedef struct {
	UINT32 a[55000];
	UINT32 n;
}aaa; //mảng a để lưu các số (long) có giá trị từ 0 đến 999'999'999

char so[10]; //n lưu số phần tử của mảng a
//----------------------------------- nhân A với x.10 mũ 9.Bn
UINT32 i,j;
UINT64 gtdem,nho;
void nhan(aaa& A, UINT32& x)
{
nho=0;
for(i=0;i<=A.n;i++)
{gtdem=nho+ (UINT64)A.a[i] * (UINT64)x;
nho= (gtdem / 1000000000);
A.a[i]=(UINT32)(gtdem % 1000000000);
}
if(nho>0)
{A.n++;A.a[A.n]=(UINT32)nho;nho=0;}
}
//-----------------------------------------------tính giai thừa của 1 số (<=10'000)
aaa d;
aaa* giaithuaX(UINT32 x)
{
UINT32 i=1;
d.n=0;d.a[0]=1; //gán giá trị cho số lớn d=1
while(x>i)
{i++;
nhan(d,i); //nhân d lần lượt với 1,2,3....x
}
return &d;
}
//-------------------------------------- in kết quả ra file (ở đây em đặt s="D:\giaithua.txt")
void outputltxt(aaa& A,char* s) //A là số lớn kiểu aaa, s là xâu chứa tên file in ra
{
	int i,j;
UINT32 d1,d2;
FILE *f;
f=fopen(s,"a+");
fprintf(f,"-------------------------\n");
fprintf(f,"%s! khoang 10 mu 9 x %d\n", so , A.n );
for(i=A.n;i>=0;i--) //in lần lượt từng phần tử của mảng A.a ra file
{d1=A.a[i]; d2=10; j=8;
while(d1>d2) {j--;d2=d2* 10;}
while(j>0) {fprintf(f,"0");j--;}
fprintf(f,"%d",d1);
}
fprintf(f,"\n");
fclose(f);
}
//----------------------------------------
void main()
{
	int i=1;long x; //A là số lớn kiểu aaa
	char *s="D:\giaithua.txt"; //chường trình in kq ra file "D:\giaithua.txt"
	while(i!=0)
	{
	printf("nhap x:"); // nhập số x
	fflush(stdin);
	scanf("%d",&x);
	ltoa(x,so,10);
	giaithuaX(x); // A chứa giá trị của x!
	printf("\n%d\n",d.n); // in ra số phần tử chứa trong mảng A.a
	outputltxt(d,s); // in mảng A.a lần lượt ra file s
	printf("\ncontinue?(1/0):");
	fflush(stdin);
	scanf("%d",&i);
	}
}