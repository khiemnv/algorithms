#include <stdio.h>
#include <conio.h>
#define Q_MAX 100
#include <stdlib.h>


class Kqueuery{

public:
	Kqueuery(){
		Q = (int*)calloc(sizeof(int),Q_MAX);
		Qsize = Q_MAX;
		erro = false;
		first = 0;
		last  = -1;
		n = 0;
	}
	Kqueuery(int size){
		if(size > 0) Qsize = size;
		else	Qsize = Q_MAX;
		Q = (int*)calloc(sizeof(int),Qsize);
		erro = false;
		first = 0;
		last  = -1;
		n = 0;
	}

private:
	int Qsize;
	int first;
	int last;
	int n;
	bool erro;
	int *Q;

public:
	int enqueue(int number){
		if(n < Qsize) {			//neu hang doi ko day
			last ++;			//last = 0 + 1
			last = last % Qsize;//last = 1;
			Q[last] = number;
			n++;			//n = 0 + 1
			erro = false;
			return number;
		}
		else {
			erro = true;
			return (number - 1);
		}
	}
	int dequeue(){
		if (n > 0){				//neu hang doi ko rong
			n --;				// n = 1 -- = 0
			int temp = Q[first];
			first ++;			//
			first = first % Qsize;
			erro = false;
			return temp;
		}
		else{
			erro = true;
			return 0;
		}
	}
	bool isEmpty(){
		if (n > 0) return false;
		else	return true;
	}
	bool isFull(){
		if (n < Qsize) return false;
		else	return true;
	}
	bool isError(){
		return erro;
	}
};