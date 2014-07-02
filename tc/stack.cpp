#include<stdio.h>
#include<conio.h>

  struct stack {int a[1000];int n;};
  void push(int i,struct stack*st)
  {int n=st->n;
  (*st).a[n]=i;
  (st->n)=n+1;
  }

  int pop(struct stack*st)
  {(st->n)=(st->n)-1;
  return (st->a)[st->n];
  }

  int full(struct stack st)
  {if (st.n==1000)return 1;
  else return 0;
  }

  int empty(struct stack st)
  {if (st.n==0) return 1;
  else return 0;
  }

  void input(int*n)
  {printf("nhap mot so nguyen: ");
  scanf("\n%d",n);
  }

  void output(int n)
  {struct stack st;int i;
  st.n=0;
  while (n!=0 && full(st)<1)
   {i=n%2;push(i,&st);n=n/2;
   }
  while (empty(st)<1)
   {printf("%d",pop(&st));
   }
  }
void main()
{int n;
clrscr();
input(&n);
output(n);
getch();
}