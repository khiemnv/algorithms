#define debug
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <STDLIB.H>
#include<STRING.H>
typedef struct aaa{int c; float x;}ra;

	float cal(char*s,float*A,float*B,float*C)
	{int l,i,j,k,n=-1,gthua=0;
	int ll,kk=1;
	char ss[100];
	float gtdem,gtdem2=0,gt=0;
	ra **mx;
/*	ra mm,nn;
	mx[n]=(ra *)calloc(1,sizeof (ra));
	mx[n]=&mm;                        */

	l=strlen(s)-1;
	for(i=l;i>=0;)
	  if(((s[i]<='9'&&s[i]>='0')||s[i]=='.')&&sscanf(s+i,"%f",&gtdem))
		{i--;
		if(i<0) {sscanf(s+i+1,"%f",&gtdem);gtdem2=1;
			n++;mx[n]=(ra *)calloc(1,sizeof (ra));mx[n]->x=gtdem;mx[n]->c=0;
			}
		}
	  else
/*b1*/	  {
/*gtdem*/ if((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.') {sscanf(s+i+1,"%f",&gtdem);gtdem2=1;}
	  if(s[i]=='A') {gtdem=*A;i--;gtdem2=1;}
	  if(s[i]=='B') {gtdem=*B;i--;gtdem2=1;}
/*egtdem*/if(s[i]=='C') {gtdem=*C;i--;gtdem2=1;}
	  if(s[i]==')') {
			for(ll=i-1;ll>=0;ll--) { if(s[ll]=='(')kk--;
						 if(s[ll]==')')k++;
						 if(kk==0) break;
						}
			strncpy(ss,s+ll+1,(i-ll-1));ss[i-ll-1]=0;
			i=ll-1;
			gtdem=cal(ss,A,B,C);
			}

/*gth=1*/ if(gthua==1) 	if(gtdem>=0||(gtdem==(int)gtdem))
			{gtdem2=1;
			while(gtdem>1) {gtdem2*=gtdem;gtdem--;}
			gtdem=gtdem2;
			gthua=0;
/*egth=1*/		}
			else gthua=0;

	  if(gtdem2){n++;mx[n]=(ra *)calloc(1,sizeof (ra));mx[n]->x=gtdem;mx[n]->c=1;}
	  #ifdef debug
	  printf("\n%f",gtdem);
	  #endif
	  switch(s[i])
		{case'+':mx[n]->c=0;i--;break;
		case'*':mx[n]->c=1;i--;break;
		case'/':mx[n]->c=2;i--;break;
		case'^':mx[n]->c=3;i--;break;
		case'g':if((s[i-1]=='o')&&(s[i-2]=='l')){mx[n]->c=4;i-=3;break;}
			if(s[i-1]=='l'){mx[n]->x=log10(mx[n]->x);i-=2;break;}
		case'n':if(s[i-1]=='l'){mx[n]->x=log(mx[n]->x);i-=2;break;}
			if((s[i-1]=='i')&&(s[i-2]=='s')){mx[n]->x=sin(mx[n]->x);i-=3;break;}
			if((s[i-1]=='a')&&(s[i-2]=='t')){mx[n]->x=tan(mx[n]->x);i-=3;break;}
		case's':if((s[i-1]=='o')&&(s[i-2]=='c')){mx[n]->x=cos(mx[n]->x);i-=3;break;}
		case'!':gthua=1;i--;break;
		default:i--;
		}
/*e1*/	  }
	for(j=n;j>=0;j--)
/*b2*/		if(mx[j]->c)
		  {switch(mx[j]->c)
		    {case 1:mx[j]->x=(mx[j+1]->x)*(mx[j]->x);mx[j]->c=mx[j+1]->c;break;
		    case 2:mx[j]->x=(mx[j+1]->x)/(mx[j]->x);mx[j]->c=mx[j+1]->c;break;
		    case 3:mx[j]->x=pow(mx[j+1]->x,mx[j]->x);mx[j]->c=mx[j+1]->c;break;
		    case 4:mx[j]->x=log(mx[j+1]->x)/log(mx[j]->x);mx[j]->c=mx[j+1]->c;break;
		    }
	      /*  free(mx[j+1]);*/
		  n--;
		  for (k=j+1;k<=n;k++){mx[k]=mx[k+1];}
/*e2*/            }
	for(j=n;j>=0;j--)
	gt=gt+mx[j]->x;
	return gt;
	}

	void input(char*ss)
	{char s[100];int i,l,k=0;
	gets(s);l=strlen(s);
	for(i=0;i<l;i++) {if (s[i]=='(') k++;if(s[i]==')') k--;}
	if(k==1) /*s[l]=')';s[l+1]=0;printf("\n%s",s);*/strcat(s,")");
	strcpy(ss,s);
	}

void main()
{char*ss;float f;float A=0,B=0,C=0;int menu;
for(;;)
{
printf("\npress 1 de nhap bieu thuc!");
printf("\npress 2 de chuyen gt bieu thuc truoc vao A");
printf("\npress 3 de nhap cho B!");
printf("\npress 4 de nhap cho C!");
printf("\npress 0 to quit!");
printf("\nyour press:");
scanf("%d",&menu);fflush(stdin);
switch (menu)
 {
 case 1:printf("bt:");fflush(stdin);
	input(ss);
	f=cal(ss,&A,&B,&C);
	printf("\n%s=%f\n",ss,f);
	break;
 case 2:{A=f;printf("\nA=%f",A);fflush(stdin);break;}
 case 3:{printf("\nB=:");
	scanf("%f",&B);fflush(stdin);break;}
 case 4:{printf("\nC=:");
	scanf("%f",&C);fflush(stdin);break;}
 }
if(menu==0)break;
}
}

