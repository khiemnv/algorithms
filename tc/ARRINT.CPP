#include <stdio.h>
#include <conio.h>
#include <alloc.h>
//--------------------------//
int insertionSort(int * arrint,int n);
//--------------------------//
int main(){

int n=0,temp,j;
int *arrint;
//nhap day so
printf ("\nnhap so phan tu: ");
fflush(stdin);
scanf("%d",&n);
arrint=(int *)calloc(sizeof (int),n+1);
if (arrint==0) return 0;
arrint[0]=n;
for (int i=1;i<=n;i++){
	printf ("nhap phan tu %d:",i);
	fflush(stdin);
	if (!scanf("%d",arrint+i)) return 0;
	}
//for (i=1;i<=n;i++) printf ("%d  ",*(arrint+i));
//sap xep
insertionSort(arrint,n);//phan tu dau tien cua day la 1
/*for (i=1;i<n;i++) {
	if (arrint[i]>arrint[i+1]){temp=arrint[i];
		arrint[i]=arrint[i+1];
		arrint[i+1]=temp;
		j=i;
		while(j>1){j--;
			if (arrint[j]>arrint[j+1]) {temp=arrint[j];
				arrint[j]=arrint[j+1];
				arrint[j+1]=temp;}
			}
		}
	} */


//in day da sx
for (i=1;i<=n;i++) printf ("%d  ",*(arrint+i));
getch();
return n;}
//sap xep
int insertionSort(int * arrint,int n)
{
for (int i=1;i<n;i++) {
	if (arrint[i]>arrint[i+1]){int temp=arrint[i];
		arrint[i]=arrint[i+1];
		arrint[i+1]=temp;
		int j=i;
		while(j>1){j--;
			if (arrint[j]>arrint[j+1]) {temp=arrint[j];
				arrint[j]=arrint[j+1];
				arrint[j+1]=temp;}
			}
		}
	}
return 0;}


