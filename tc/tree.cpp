
#define debug
#include <stdio.h>
#include <conio.h>
#include<STDLIB.H>
typedef struct aaa{float f;aaa*re;aaa*l;aaa*r;} //r:right
bbb;						//re:reverse
						//l:left

	void creattree(bbb*a,float**m,int n)
	{int i;
	a->f=*m[0];a->l=0;a->r=0;a->re=0;
	for(i=1;i<n;)
		{
		if(*m[i]<=a->f)
			if(a->l==0)
				{a->l=(bbb*)calloc(sizeof(bbb),1);
				a->l->f=*m[i];
				a->l->r=0;a->l->l=0;a->l->re=a;
				i++;
				while(a->re!=0)
					{a=a->re;
					#ifdef debug
					printf("\na=%d",a);
					#endif
					}
				#ifdef debug
				printf("\na->f=%f",a->f);
				#endif
				}
			else {a=a->l;}
		if(*m[i]>a->f)
			if(a->r==0)
				{a->r=(bbb*)calloc(sizeof(bbb),1);
				a->r->f=*m[i];
				a->r->r=0;a->r->l=0;a->r->re=a;
				i++;
				while(a->re!=0)
					{a=a->re;
					#ifdef debug
					printf("\na=%d",a);
					#endif
					}
				}
			else {a=a->r;}
		}
	}

	float mintree(bbb*a)
	{
	if(a->l==0) return a->f;
	else return mintree(a->l);
	}
	float Maxtree(bbb*a)
	{
	if(a->r==0) return a->f;
	else return Maxtree(a->r);
	}

	void readtree(bbb*a,float**m,int n)
	{bbb*atrg;int i=-1;
	//while(a->l!=0) {a=a->l;}
	while(a->re!=0||a->l!=0)
		{if(a->l==0)
			{i++;*m[i]=a->f;
			a->re->l=a->r;
			if(a->r!=0)
				{a->r->re=a->re;a=a->r;}
			else
				{atrg=a;a=a->re;}
			//free(atrg);
			}
		else a=a->l;
		}
	#ifdef debug
	printf("\na0->f=%f",a->f);
	#endif
	if(a->re==0) {i++;*m[i]=a->f;}
	//while(a->r!=0) {a=a->r;}
	while(a->re!=0||a->r!=0)
		{if(a->r==0)
			{n--;*m[n]=a->f;
			a->re->r=a->l;
			if(a->l!=0)
				{a->l->re=a->re;a=a->l;}
			else
				{atrg=a;a=a->re;}
			//free(atrg);
			}
		 else a=a->r;
		}
	//free(a);
	}

	void input(float**m,int*n)
	{int i=0;char c;float mm;
	//clrscr();
	while(1)
		{
		gotoxy(35, 12);
		printf("press E to quit!\n");
		gotoxy(1,i+1);
		printf("m[%d]=",i+1);
		fflush(stdin);
		if(scanf("%f",&mm))
			{*n=i+1;
			m[i]=(float*)malloc(sizeof(float));
			*m[i]=mm;
			mm=0;
			i++;
			continue;
			}
		else break;
		}
	}

	void output(float**m,int n)
	{int i;
	for(i=0;i<n;i++)
	printf("\n%f",*m[i]);
	}

void main()
{int n;float**m;
bbb*a0;
clrscr();
input(m,&n);
creattree(a0,m,n);
readtree(a0,m,n);
output(m,n);
getch();
}