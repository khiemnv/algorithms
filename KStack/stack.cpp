#include "stack.h"
int main()
{
	int select = 1;
	int temp;
	Kstack my_s = Kstack(5);
	while (select != 0)
	{
		printf("\n1:push a number");
		printf("\n2:pop a number");
		printf("\n3:get last erro");
		printf("\n0:quit");
		printf("\n_>");
		fflush(stdin);
		scanf("%d",&select);
		switch(select)
		{
		case 1: 
			printf("\npush mot so int vao stack: ");
			fflush(stdin);
			scanf("%d",&temp);
			printf("\nret:%d",my_s.push(temp));
			break;
		case 2:
			printf("\npop mot so int tu stack: ");
			printf("%d",my_s.pop());
			break;
		case 3:
			if(my_s.isError())
				printf("\nhas error occur");
			else 
				printf("\nhas no error occur");
			break;
		case 0:
			break;
		default:
			printf("\nu select wrong number!");
		}
		
	}
	
	return 0;
}