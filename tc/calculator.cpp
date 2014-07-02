
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <STDLIB.H>
#include<STRING.H>
typedef struct mystruct{float x;int c;} ra;

 float cal(char*s,float*A,float*B,float*C)
   {float gtdem,gt=0;float mx[10];int mc[10];
   int l,i,j,k;int n=-1;int ll,kk=1; float gtgt;char ss[100];ra * mm;

    l=strlen(s)-1;
    for(i=l;i>=0;)
    if(((s[i]<='9'&&s[i]>='0')||s[i]=='.')&&sscanf(s+i,"%f",gtdem))/*((s[i]<58&&s[i]>47)||s[i]==46)*/
	 {i--;
	 if(i<0)
	    {n++;sscanf(s+i+1,"%f",&mx[n]);mc[n]=0;}
	 }
    else

	 switch(s[i])
	{ case '+': {if(sscanf(s+i,"%f",&gtdem))
				{n++;mc[n]=0;sscanf(s+i,"%f",mx+n);
				#ifdef DEBUG
				printf("\n%f   %d",mx[n],n);
				#endif
				}
		     i--; break;
		     }
	  case'-': if(sscanf(s+i,"%f",&gtdem))
				{n++;mc[n]=0;sscanf(s+i,"%f",mx+n);
				#ifdef DEBUG
				printf("\n%f   %d",mx[n],n);
				#endif
				}
		      else  mx[n]=-mx[n];
		     i--;break;

	   case'n':if((*(s+i-1)=='i')&&(*(s+i-2)=='s'))
			  {if((s[i+1]<58&&s[i+1]>47)||s[i+1]==46)
				{n++;mc[n]=0;sscanf(s+i+1,"%f",mx+n);}
			  mx[n]=sin(mx[n]);
			  i-=3;break;
			  }
		   if ((*(s+i-1)=='a')&&(*(s+i-2)=='t'))
			  {if((s[i+1]<58&&s[i+1]>47)||s[i+1]==46)
				{n++;mc[n]=0;sscanf(s+i+1,"%f",mx+n);}
			  mx[n]=tan(mx[n]);
			  i-=3;break;
			  }
		   if(*(s+i-1)=='l')
			  {if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mc[n]=0;sscanf(s+i+1,"%f",mx+n);}
			  if(mx[n]<=0)
				{
				printf("\nkhong co: ln%f ",mx[n]);
				i=-1;n=-1;
				break;
				}
			  mx[n]=log(mx[n]);
			  i-=2;break;
			  }
		   break;
	   case'g':if((*(s+i-1)=='o')&&(*(s+i-2)=='l'))
			  {if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mc[n]=0;sscanf(s+i+1,"%f",mx+n);}

			   if(mx[n]<=0)
				{i=-1;n=-1;
				printf("\nco so phai >0");
				break;
				}
			   mc[n]=4;
			   i-=3;break;
			  }
		   if(*(s+i-1)=='l')
			  {if((s[i+1]<58&&s[i+1]>47)||s[i+1]==46)
				{n++;mc[n]=0;sscanf(s+i+1,"%f",mx+n);}
			   if(mx[n]<=0)
				{
				printf("\nkhong co: lg%f ",mx[n]);
				i=-1;n=-1;
				break;}
			  mx[n]=log10(mx[n]);
			   i-=2;
			  }
		   break;
	   case's':if((*(s+i-1)=='o')&&(*(s+i-2)=='c'))
			 {if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mc[n]=0;sscanf(s+i+1,"%f",mx+n);}
			  mx[n]=cos(mx[n]);
			   i-=3;
			 }
		   if((*(s+i-1)=='b')&&(*(s+i-2)=='a'))
			 {if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;mc[n]=0;sscanf(s+i+1,"%f",mx+n);
				#ifdef DEBUG
				printf("\n%f   %d",mx[n],n);
				#endif
				}
			  mx[n]=fabs(mx[n]);
			  i-=3;
			 }
		   break;
	   case'p':if((*(s+i-1)=='x')&&(*(s+i-2)=='e'))
		       {if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;sscanf(s+i+1,"%f",mx+n);mx[n]=pow(10,mx[n]);
				#ifdef DEBUG
				printf("\n%f   %d",mx[n],n);
				#endif
				}
			i-=3;mc[n]=1;
		       }
		   break;

	   case'*':if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;sscanf(s+i+1,"%f",mx+n);
				#ifdef DEBUG
				printf("\n%f   %d",mx[n],n);
				#endif
				}
			    i--; mc[n]=1;
		   break;
	   case'/':if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;sscanf(s+i+1,"%f",mx+n);

				if (mx[n]==0){i=-1;n=-1;
					#ifdef DEBUG
					printf("\nloi so bi chia");
					#endif
					     break;
					     }
				}
			    i--;mc[n]=2;
		   break;

	   case'^':if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
				{n++;sscanf(s+i+1,"%f",mx+n);
				#ifdef DEBUG
				printf("\n%f   %d",mx[n],n);
				#endif
				}

			    i--;mc[n]=3;
		   break;
	   case')':
		    n++;{
			for(ll=i-1;ll>=0;ll--) { if(s[ll]=='(')kk--;
						 if(s[ll]==')')k++;
						 if(kk==0) break;
						}
			strncpy(ss,s+ll+1,(i-ll-1));ss[i-ll-1]=0;
			i=ll-1;
			mx[n]=cal(ss,A,B,C);mc[n]=0;
			}
		     break;

	   case'A':
		    if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
			  {n++;sscanf(s+i+1,"%f",mx+n);mc[n]=1;n++;mx[n]=*A;}
			  else if(s[i+1]<='C'&&s[i+1]>='A')
			   {n++;mx[n]=*A;mc[n-1]=1;}
			   else {n++;mx[n]=*A;}
		     i--;if(i<0)mc[n]=0;
		     break;

	   case'B':
		    if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
			  {n++;sscanf(s+i+1,"%f",mx+n);mc[n]=1;n++;mx[n]=*B;
			  #ifdef DEBUG
			  printf("\n%f   %d",mx[n],n);
			  #endif
			  }
			  else if(s[i+1]<='C'&&s[i+1]>='A')
			   {n++;mx[n]=*B;mc[n-1]=1;
			   #ifdef DEBUG
			   printf("\n%f   %d",mx[n],n);
			   #endif
			   }
			   else {n++;mx[n]=*B;
				#ifdef DEBUG
				printf("\n%f   %d",mx[n],n);
				#endif
				}
		     i--; if(i<0)mc[n]=0;
		     break;
	   case'C':
		    if(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",gtdem))
			  {n++;sscanf(s+i+1,"%f",mx+n);mc[n]=1;n++;mx[n]=*C;}
			  else if(s[i+1]<='C'&&s[i+1]>='A')
			   {n++;mx[n]=*C;mc[n-1]=1;
			   }
			   else {n++;mx[n]=*B;}
		     i--;if(i<0)mc[n]=0;
		     break;

	   default:{strcpy(s+i,s+i+1);
		   i--;}
	}
    for(j=n;j>=0;j--) {
    /*b11*/   {
	  if(mc[j]==1)
	    {mx[j]=mx[j]*mx[j+1];
	    mc[j]=mc[j+1];
	    n--;
	    for (k=j+1;k<=n;k++)
	    {mx[k]=mx[k+1];mc[k]=mc[k+1];}
	    }
	  if(mc[j]==2)
	    {mx[j]=mx[j+1]/mx[j];
	    mc[j]=mc[j+1];
	    n--;
	    for (k=j+1;k<=n;k++){mx[k]=mx[k+1];mc[k]=mc[k+1];}
	    }
	  if(mc[j]==3)
	    {mx[j]=pow(mx[j+1],mx[j]);
	    mc[j]=mc[j+1];
	    n--;
	    for (k=j+1;k<=n;k++){mx[k]=mx[k+1];mc[k]=mc[k+1];}
	    }
	  if(mc[j]==4)
	    if(mx[j+1]>0)
	      {mx[j]=log(mx[j+1])/log(mx[j]);
	      mc[j]=mc[j+1];
	      n--;
	      for (k=j+1;k<=n;k++){mx[k]=mx[k+1];mc[k]=mc[k+1];}
	      }
	    else {printf("\nkhong co:%flog%f",mx[j+1],mx[j]);return 0;}
/*e11*/	  }

			}
    for(j=n;j>=0;j--)
    gt=gt+mx[j];
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