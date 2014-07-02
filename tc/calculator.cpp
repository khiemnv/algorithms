
							//e:end
#include <stdio.h>					//b:begin
#include <conio.h>					//gthua:gai thua
#include <math.h>
#include <STDLIB.H>
#include<STRING.H>
typedef struct aaa{int c; float x;int d;}ra;
float A=0,B=0,C=0;
int mode=3,loi=0;

	void error(int*aan,int*aai)
	{loi=1+(*aai);*aai=-1;*aan=-1;}

	void addnew(int*aagthua,float*aagtdem,int*aagtdem2,int*aan,ra ** aamx,int*aai)
	{ int gthua,n,i,gtdem2;float gtdem,gtdem3;ra ** mx;
	gthua=*aagthua;gtdem=*aagtdem;mx=aamx;n=*aan;gtdem2=*aagtdem2;i=*aai;
/*gth=1*/ if(gthua==1) 	if(gtdem>=0&&(gtdem==(int)gtdem))
			{gtdem3=1;
			while(gtdem>1) {gtdem3*=gtdem;gtdem--;}
			gtdem=gtdem3;
			gthua=0;
/*egth=1*/		}
			else {i++;error(&n,&i);gthua=0;}
	  if(gtdem2==1) {n++;mx[n]=(ra *)calloc(1,sizeof (ra));
			mx[n]->x=gtdem;mx[n]->c=1;mx[n]->d=0;gtdem2=0;
			}
	  *aagthua=gthua;*aagtdem=gtdem;aamx=mx;*aan=n;*aagtdem2=gtdem2;*aai=i;
	}

	int size(char*s)
	{int i,n=0;float a;
	for(i=0;i<strlen(s);)
	  {if  (((s[i]<='9'&&s[i]>='0')||s[i]=='.') && sscanf(s+i,"%f",&a))
	    if(!(((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')&&sscanf(s+i+1,"%f",&a))||s[i+1]==0)
	    {n++;i++;}
	  if(s[i]=='('){n++;while(s[i]!=')')i++;i++;}
	  if((s[i]=='P'&&s[i+1]=='I')||(s[i]>='A'&&s[i]<='C'))n++;
	  i++;
	  }
	return n;
	}

/*cal*/	float cal(char*s)		//ham tinh gt bieu thuc
	{int i,l,j,k,n=-1,gthua=0,gtdem2=0;
	int ll,kk=1;
	char ss[1024];
	float gtdem,gt=0;
	ra **mx;

	mx=(ra**)calloc(size(s),sizeof (ra*));
	l=strlen(s)-1;
	for(i=l;i>=0;)
	  if(((s[i]<='9'&&s[i]>='0')||s[i]=='.')&&sscanf(s+i,"%f",&gtdem))
		{i--;
		if(i<0) {sscanf(s+i+1,"%f",&gtdem);gtdem2=1;
			addnew(&gthua,&gtdem,&gtdem2,&n,mx,&i);}
		}
	  else
/*b1*/	  {
	  if((s[i+1]<='9'&&s[i+1]>='0')||s[i+1]=='.')
			{sscanf(s+i+1,"%f",&gtdem);gtdem2=1;
			addnew(&gthua,&gtdem,&gtdem2,&n,mx,&i);
			}
	  while(((s[i]>='A'&&s[i]<='C')||s[i]==')'||s[i]=='I')&&i>=0)
		{if(s[i]=='A') {gtdem=A;gtdem2=1;i--;}
		if(!gtdem2)if(s[i]=='B') {gtdem=B;gtdem2=1;i--;}
		if(!gtdem2)if(s[i]=='C') {gtdem=C;gtdem2=1;i--;}
		if(!gtdem2)if(s[i]=='I'&&s[i-1]=='P')
			{gtdem=2*asin(1);gtdem2=1;i-=2;}
		if(!gtdem2)if(s[i]==')') {
			for(ll=i-1;ll>=0;ll--)
				{
				if(s[ll]=='('){kk--;}
				if(s[ll]==')'){kk++;}
				if(kk==0) {kk=1;break;}
				}
			strncpy(ss,s+ll+1,(i-ll-1));ss[i-ll-1]=0;
			i=ll;
			gtdem=cal(ss);
			if(loi!=0)
				loi+=i;
			gtdem2=1;i--;
			}
		addnew(&gthua,&gtdem,&gtdem2,&n,mx,&i);
		}
	  if(((s[i]<='9'&&s[i]>='0')||s[i]=='.')&&sscanf(s+i,"%f",&gtdem)) continue;
	  if(i<0)break;
	  switch(s[i])
		{case'+':if((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->c=0;i--;break;}
				else {error(&n,&i);break;}
		case'*':if((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->c=11;i--;break;}
				else {error(&n,&i);break;}
		case'/':if((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->c=2;i--;break;}
				else {error(&n,&i);break;}
		case'^':if((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->c=3;mx[n]->d=i;i--;break;}
				else {error(&n,&i);break;}
		case'-':if((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->c=0;mx[n]->x=-1*mx[n]->x;i--;break;}
				else {error(&n,&i);break;}
		case'!':if(gthua==0)
			       {gthua=1;i--;break;}
			       else {error(&n,&i);break;}
		case'g':if(s[i-1]=='o'&&s[i-2]=='l')
				if(mx[n]->x>1
				&&((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0))
				{mx[n]->c=4;mx[n]->d=i;i-=3;break;}
				else{error(&n,&i);break;}
			if(s[i-1]=='l')
				if(mx[n]->x>0
				&&((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0))
				{mx[n]->x=log10(mx[n]->x);i-=2;break;}
				else {error(&n,&i);break;}
			if(mode==3){error(&n,&i);break;}
			else{s[i]=' ';i--;break;}
		case'n':if(s[i-1]=='l')
				if(mx[n]->x>0
				&&((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0))
				{mx[n]->x=log(mx[n]->x);i-=2;break;}
				else {error(&n,&i);break;}
			if((s[i-1]=='i')&&(s[i-2]=='s')&&s[i-3]=='a')
				if(mx[n]->x<=1&&mx[n]->x>=-1
				&&((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0))
				{mx[n]->x=asin(mx[n]->x);i-=4;break;}
				else {error(&n,&i);break;}
			if((s[i-1]=='a')&&(s[i-2]=='t')&&s[i-3]=='a')
				if((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->x=atan(mx[n]->x);i-=4;break;}
				else {error(&n,&i);break;}
			if((s[i-1]=='i')&&(s[i-2]=='s'))
				if((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->x=sin(mx[n]->x);i-=3;break;}
				else {error(&n,&i);break;}
			if((s[i-1]=='a')&&(s[i-2]=='t'))
				if((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->x=tan(mx[n]->x);i-=3;break;}
				else {error(&n,&i);break;}
			if(mode==3){error(&n,&i);break;}
			else{s[i]=' ';i--;break;}
		case'p':if(s[i-1]=='x'&&s[i-2]=='e')
				if(mx[n]->x>0
				&&(mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->x=exp(mx[n]->x);i-=3;break;}
				else{error(&n,&i);break;}
			if(mode==3){error(&n,&i);break;}
			else{s[i]=' ';i--;break;}
		case's':if((s[i-1]=='o')&&(s[i-2]=='c')&&s[i-3]=='a')
				if(mx[n]->x>=-1&&mx[n]->x<=1
				&&((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0))
				{mx[n]->x=acos(mx[n]->x);i-=4;break;}
				else {error(&n,&i);break;}
			if((s[i-1]=='o')&&(s[i-2]=='c'))
				if((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->x=cos(mx[n]->x);i-=3;break;}
				else {error(&n,&i);break;}
			if((s[i-1]=='b')&&(s[i-2]=='a'))
				if((mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->x=fabs(mx[n]->x);i-=3;break;}
				else {error(&n,&i);break;}
			if(mode==3){error(&n,&i);break;}
			else{s[i]=' ';i--;break;}
		case't':if(s[i-1]=='r'&&s[i-2]=='q'&&s[i-3]=='s')
				if(mx[n]->x>0
				&&(mx[n]->c==1||mx[n]->c==0)&&gthua==0&&n>=0)
				{mx[n]->x=sqrt(mx[n]->x);i-=4;break;}
				else {error(&n,&i);break;}
		default:if(mode==3){error(&n,&i);break;}
			else{s[i]=' ';i--;break;}
		}
/*e1*/	  }
	mx[n]->c=0;

	for(j=n;j>=0;j--)
		if(mx[j]->c==3)
			{ if(mx[j+1]->x>0)
			   {mx[j]->x=pow(mx[j+1]->x,mx[j]->x);mx[j]->c=mx[j+1]->c;}
			else {i=mx[j]->d;error(&n,&i);}
			free(mx[j+1]);
			n--;
			for (k=j+1;k<=n;k++){mx[k]=mx[k+1];}
			}

	for(j=n;j>=0;j--)
		if(mx[j]->c==4)
			{if(mx[j+1]->x>0)
			   {mx[j]->x=log(mx[j+1]->x)/log(mx[j]->x);
			   mx[j]->c=mx[j+1]->c;}
			else {i=mx[j]->d;error(&n,&i);}
			free(mx[j+1]);
			n--;
			for (k=j+1;k<=n;k++){mx[k]=mx[k+1];}
			}

	for(j=n;j>=0;j--)
/*b2*/		if(mx[j]->c)
		  {switch(mx[j]->c)
		    {case 11:
		    case 1:mx[j]->x=mx[j+1]->x*mx[j]->x;mx[j]->c=mx[j+1]->c;break;
		    case 2:if(mx[j]->x!=0)
			   {mx[j]->x=(mx[j+1]->x)/(mx[j]->x);mx[j]->c=mx[j+1]->c;}
			   else n=-1;
		    }
		  free(mx[j+1]);
		  n--;
		  for (k=j+1;k<=n;k++){mx[k]=mx[k+1];}
/*e2*/            }

	for(j=n;j>=0;j--)
	{gt=gt+mx[j]->x;free(mx[j]);}
	free(mx);
	return gt;
	}

	void input(char*ss)
	{char s[1024];int i,l,k=0;
	fflush(stdin);
	gets(s);l=strlen(s);
	for(i=0;i<l;i++){if (s[i]=='(') k++;if(s[i]==')') k--;}
	while(k>0){strcat(s,")");k--;}
	strcpy(ss,s);
	}

	char* xdc(char*s)
	{int i,l;
	l=strlen(s);
	for(i=0;i<l;)
	  if(s[i]==' ')
	    {if(((s[i+1]>='0'&&s[i+1]<='9')||s[i+1]=='.')
		&&((s[i-1]>='0'&&s[i-1]<='9')||s[i-1]=='.'))
		i++;
	    else {strcpy(s+i,s+i+1);l--;}
	    }
	  else i++;
	return s;
	}

	void store(char**ms,int*ans,char*ss)
	{int ns;
	ns=*ans;
	if(ns<4)ns++;else ns=0;
	ms[ns]=strdup(ss);
	*ans=ns;
	}
	void restore(char**ms,int*ans,char*ss)
	{int ns;
	ns=*ans;
	strcpy(ss,ms[ns]);
	if(ns>0)ns--;else ns=4;
	*ans=ns;
	}
	void frees(char**ms)
	{int i;
	for(i=0;i<5;i++)free(ms[i]);
	free(ms);
	}
void main()
{char ss[1024];float f;char menu;int t,h=1,ns=-1;char**ms;
ms=(char**)calloc(5,sizeof(char*));
for(;;)
  {if(h==1)
	{printf("\npress 1 de nhap bieu thuc");
	printf("\npress 2 de chuyen gt bieu thuc truoc vao A");
	printf("\npress 3 de nhap gt cho B");
	printf("\npress 4 de nhap gt cho C");
	printf("\npress c de xoa man hinh");
	printf("\npress h to hide/show menu");
	printf("\npress m de chon mode");
	printf("\npress p view bt da nhap");
	printf("\npress r de tinh bt da nhap");
	printf("\npress v to view all var");
	printf("\npress 0 to quit!");
	}
  printf("\nyou press:");
  fflush(stdin);
  scanf("%c",&menu);
  switch (menu)
	{case'1':printf("bt:");
		input(ss);store(ms,&ns,ss);
		f=cal(ss);
		if(mode!=1)
			if(loi!=0)
				{printf("\n %s\n",ss);
				while((loi--)>0){printf(" ");}
				printf("^");loi=0;
				}
			else printf("\n%s=%f",xdc(ss),f);
		else printf("\n%s=%f",xdc(ss),f);
		break;
	case'2':A=f;printf("\nA=%f",f);break;
	case'3':printf("\nB=:");input(ss);B=cal(ss);break;
	case'4':printf("\nC=:");input(ss);C=cal(ss);break;
	case'c':clrscr();break;
	case'h':h=!h;break;
	case'm':printf("\nmode:1,2,3 press key tuong ung:");
		scanf("%d",&t);if(t>=1||t<=3)mode=t;break;
	case'p':restore(ms,&ns,ss);printf("\n%s",ss);break;
	case'r':f=cal(ss);printf("\n%s=%f",ss,f);break;
	case'v':printf("A=%f  B=%f  C=%f",A,B,C);break;
	case'0':break;
	default:h=1;continue;
	}
  if(menu=='0'){frees(ms);break;}
  }
}

