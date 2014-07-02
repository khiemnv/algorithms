
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <STDLIB.H>
#include<STRING.H>
typedef struct aaa {float x; int c;} ra;

 float cal(char*s,float*A,float*B,float*C)
   {float gtdem,gt=0;/*float mx[10];int mc[10];*/
   int l,i,j,k;int n=-1;int ll,kk=1; float gtgt;char ss[100];ra ** mm;

    l=strlen(s)-1;
    for(i=l;i>=0;)
    if(((s[i]<='9'&&s[i]>='0')||s[i]=='.')&&sscanf(s+i,"%f",gtdem))/*((s[i]<58&&s[i]>47)||s[i]==46)*/
	 {i--;
	 if(i<0)
	    {n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
	    sscanf(s+i+1,"%f",&(mm[n]->x));mm[n]->c=0;}
	 }
    else

	 switch(s[i])
	{ case '+': {if(sscanf(s+i,"%f",&gtdem))
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				mm[n]->c=0;sscanf(s+i,"%f",&(mm[n]->x));
				#ifdef DEBUG
				printf("\n%f   %d",(mm[n]->x),n);
				#endif
				}
		     i--; break;
		     }
	  case'-': if(sscanf(s+i,"%f",&gtdem))
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				mm[n]->c=0;sscanf(s+i,"%f",&(mm[n]->x));
				#ifdef DEBUG
				printf("\n%f   %d",(mm[n]->x),n);
				#endif
				}
		      else  (mm[n]->x)=(mm[n]->x);
		     i--;break;

	   case'n':if((*(s+i-1)=='i')&&(*(s+i-2)=='s'))
			  {if((s[i+1]<58&&s[i+1]>47)||s[i+1]==46)
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				mm[n]->c=0;sscanf(s+i+1,"%f",&(mm[n]->x));}
			  (mm[n]->x)=sin(mm[n]->x);
			  i-=3;break;
			  }
		   if ((*(s+i-1)=='a')&&(*(s+i-2)=='t'))
			  {if((s[i+1]<58&&s[i+1]>47)||s[i+1]==46)
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				mm[n]->c=0;sscanf(s+i+1,"%f",&(mm[n]->x));}
			  (mm[n]->x)=tan(mm[n]->x);
			  i-=3;break;
			  }
		   if(*(s+i-1)=='l')
			  {if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				mm[n]->c=0;sscanf(s+i+1,"%f",&(mm[n]->x));}
			  if((mm[n]->x)<=0)
				{
				printf("\nkhong co: ln%f ",(mm[n]->x));
				i=-1;n=-1;
				break;
				}
			  (mm[n]->x)=log(mm[n]->x);
			  i-=2;break;
			  }
		   break;
	   case'g':if((*(s+i-1)=='o')&&(*(s+i-2)=='l'))
			  {if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				mm[n]->c=0;sscanf(s+i+1,"%f",&(mm[n]->x));}

			   if((mm[n]->x)<=0)
				{i=-1;n=-1;
				printf("\nco so phai >0");
				break;
				}
			   mm[n]->c=4;
			   i-=3;break;
			  }
		   if(*(s+i-1)=='l')
			  {if((s[i+1]<58&&s[i+1]>47)||s[i+1]==46)
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				mm[n]->c=0;sscanf(s+i+1,"%f",&(mm[n]->x));}
			   if((mm[n]->x)<=0)
				{
				printf("\nkhong co: lg%f ",(mm[n]->x));
				i=-1;n=-1;
				break;}
			  (mm[n]->x)=log10(mm[n]->x);
			   i-=2;
			  }
		   break;
	   case's':if((*(s+i-1)=='o')&&(*(s+i-2)=='c'))
			 {if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				mm[n]->c=0;sscanf(s+i+1,"%f",&(mm[n]->x));}
			  (mm[n]->x)=cos((mm[n]->x));
			   i-=3;
			 }
		   if((*(s+i-1)=='b')&&(*(s+i-2)=='a'))
			 {if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				mm[n]->c=0;sscanf(s+i+1,"%f",&(mm[n]->x));
				#ifdef DEBUG
				printf("\n%f   %d",(mm[n]->x),n);
				#endif
				}
			  (mm[n]->x)=fabs((mm[n]->x));
			  i-=3;
			 }
		   break;
	   case'p':if((*(s+i-1)=='x')&&(*(s+i-2)=='e'))
		       {if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				sscanf(s+i+1,"%f",&(mm[n]->x));(mm[n]->x)=pow(10,(mm[n]->x));
				#ifdef DEBUG
				printf("\n%f   %d",(mm[n]->x),n);
				#endif
				}
			i-=3;mm[n]->c=1;
		       }
		   break;

	   case'*':if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				sscanf(s+i+1,"%f",&(mm[n]->x));
				#ifdef DEBUG
				printf("\n%f   %d",(mm[n]->x),n);
				#endif
				}
			    i--; mm[n]->c=1;
		   break;
	   case'/':if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				sscanf(s+i+1,"%f",&(mm[n]->x));

				if ((mm[n]->x)==0){i=-1;n=-1;
					#ifdef DEBUG
					printf("\nloi so bi chia");
					#endif
					     break;
					     }
				}
			    i--;mm[n]->c=2;
		   break;

	   case'^':if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				sscanf(s+i+1,"%f",&(mm[n]->x));
				#ifdef DEBUG
				printf("\n%f   %d",(mm[n]->x),n);
				#endif
				}

			    i--;mm[n]->c=3;
		   break;
	   case')':
				n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
			{
			for(ll=i-1;ll>=0;ll--) { if(s[ll]=='(')kk--;
						 if(s[ll]==')')k++;
						 if(kk==0) break;
						}
			strncpy(ss,s+ll+1,(i-ll-1));ss[i-ll-1]=0;
			i=ll-1;
			(mm[n]->x)=cal(ss,A,B,C);mm[n]->c=0;
			}
		     break;

	   case'A':
		    if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
			  {n++; mm[n]=(ra*)calloc(1,sizeof (ra *));
			  sscanf(s+i+1,"%f",&(mm[n]->x));mm[n]->c=1;
			  n++;  mm[n]=(ra*)calloc(1,sizeof (ra *));
			  (mm[n]->x)=*A;}
			  else if(s[i+1]<='C'&&s[i+1]>='A')
			   {n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
			   (mm[n]->x)=*A;mm[n-1]->c=1;}
			   else {n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				(mm[n]->x)=*A;}
		     i--;if(i<0)mm[n]->c=0;
		     break;

	   case'B':
		    if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
			  {n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
			  sscanf(s+i+1,"%f",&(mm[n]->x));mm[n]->c=1;
			  n++; mm[n]=(ra*)calloc(1,sizeof (ra *));
			  (mm[n]->x)=*B;
			  #ifdef DEBUG
			  printf("\n%f   %d",(mm[n]->x),n);
			  #endif
			  }
			  else if(s[i+1]<='C'&&s[i+1]>='A')
			   {n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
			   (mm[n]->x)=*B;mm[n-1]->c=1;
			   #ifdef DEBUG
			   printf("\n%f   %d",(mm[n]->x),n);
			   #endif
			   }
			   else {n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				(mm[n]->x)=*B;
				#ifdef DEBUG
				printf("\n%f   %d",(mm[n]->x),n);
				#endif
				}
		     i--; if(i<0)mm[n]->c=0;
		     break;
	   case'C':
		    if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
			  {n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
			  sscanf(s+i+1,"%f",&(mm[n]->x));mm[n]->c=1;
			  n++; mm[n]=(ra*)calloc(1,sizeof (ra *));
			  (mm[n]->x)=*C;}
			  else if(s[i+1]<='C'&&s[i+1]>='A')
			   {n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
			   (mm[n]->x)=*C;mm[n-1]->c=1;
			   }
			   else {n++;mm[n]=(ra*)calloc(1,sizeof (ra *));
				(mm[n]->x)=*B;}
		     i--;if(i<0)mm[n]->c=0;
		     break;

	   default:{strcpy(s+i,s+i+1);
		   i--;}
	}
    for(j=n;j>=0;j--) {
    /*b11*/   {
	  if(mm[j]->c==1)
	    {mm[j]->x=mm[j]->x*mm[j+1]->x;
	    mm[j]->x=mm[j+1]->x;
	    n--;
	    for (k=j+1;k<=n;k++)
	    {mm[k]->x=mm[k+1]->x;mm[k]->c=mm[k+1]->c;}
	    }
	  if(mm[j]->c==2)
	    {mm[j]->x=mm[j+1]->x/mm[j]->x;
	    mm[j]->c=mm[j+1]->c;
	    n--;
	    for (k=j+1;k<=n;k++){mm[k]->x=mm[k+1]->x;mm[k]->c=mm[k+1]->c;}
	    }
	  if(mm[j]->c==3)
	    {mm[j]->x=pow(mm[j+1]->x,mm[j]->x);
	    mm[j]->c=mm[j+1]->c;
	    n--;
	    for (k=j+1;k<=n;k++){mm[k]->x=mm[k+1]->x;mm[k]->c=mm[k+1]->c;}
	    }
	  if(mm[j]->c==4)
	    if(mm[j+1]->x>0)
	      {mm[j]->x=log(mm[j+1]->x)/log(mm[j]->x);
	      mm[j]->c=mm[j+1]->c;
	      n--;
	      for (k=j+1;k<=n;k++){mm[k]->x=mm[k+1]->x;mm[k]->c=mm[k+1]->c;}
	      free(mm[n+1]);
	      }
	    else {printf("\nkhong co:%flog%f",mm[j+1]->x,mm[j]->x);return 0;}
/*e11*/	  }

			}
    for(j=n;j>=0;j--)
    gt=gt+mm[j]->x;
    free(mm[0]);
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