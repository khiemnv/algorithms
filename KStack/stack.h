#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#define S_MAX 100

class Kstack
{
public:
	Kstack();	
	Kstack(int);
	int push(int);
	int pop();
	bool isFull();
	bool isError();
	bool isEmpty();
protected:
private:
	int size;
	int top;
	int *stack;
	bool error;
};

Kstack::Kstack(){	
	size = S_MAX;
	stack = (int*)calloc(sizeof(int),size);
	top = 0;	
	error = false;	
}
Kstack::Kstack(int stack_size){
	if (stack_size > 0)	
		size = stack_size;
	else	
		size = S_MAX;
	stack = (int*)calloc(sizeof(int),size);
	top = 0;	
	error = false;	
}
int Kstack::push(int number){
	if (top < (size)){
		stack[top] = number;	//top = 0
		top ++;					//top = 1
		error = false;
		return number;
	}
	else {
		error = true;
		return (number - 1);
	}
}
int Kstack::pop(){
	if(top > 0){			//top = 1
		error = false;
		top --;				//top = 0
		return stack[top];
	}
	else{
		error = true;
		return 0;
	}
}
bool Kstack::isEmpty(){
	if(top > 0) return false;
	else	return true;
}
bool Kstack::isFull(){
	if(top < size) return false;
	else	return true;
}
bool Kstack::isError(){
	return error;
}