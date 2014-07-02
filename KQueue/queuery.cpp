#include "queuery.h"
int main()
{
	int select = 1;
	int temp;
	Kqueuery my_q = Kqueuery(5);
	while (select != 0)
	{
		printf("\n1:enqueue  number");
		printf("\n2:dequeue a number");
		printf("\n3:get last erro");
		printf("\n0:quit");
		printf("\n_>");
		fflush(stdin);
		scanf("%d",&select);
		switch(select)
		{
		case 1: 
			printf("\nnhap mot so int vao hang doi: ");
			fflush(stdin);
			scanf("%d",&temp);
			printf("\nret:%d",my_q.enqueue(temp));
			break;
		case 2:
			printf("\nlay mot so int tu hang doi: ");
			printf("%d",my_q.dequeue());
			break;
		case 3:
			if(my_q.isError())
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