/*
 * stack
 *
 *  Created on: 2013-6-6
 *      Author: yaya
 */

#ifndef STACK_
#define STACK_

#include <stdio.h>
#include <stdlib.h>

class Stack {
public:
	Stack(size_t capacity_) :
		elements(NULL), top(0), capacity(capacity_) {
		elements = (unsigned*)malloc(capacity*sizeof(unsigned));
	}
	~Stack() {
		free(elements);
	}
	bool isFull() {
		return (top >= capacity);
	}
	void push(unsigned val) {
		if(isFull()){
			capacity = capacity*3/2;
			elements = (unsigned*)realloc(elements,capacity*sizeof(unsigned));
		}
		elements[top] = val;
		(top)++;
	}
	bool isEmpty() {
		return (top == 0);
	}
	unsigned pop() {
		if(capacity >4096 &&top < capacity/2){
			capacity = capacity*2/3;
			elements = (unsigned*)realloc(elements,capacity*sizeof(unsigned));
		}
		(top)--;
		return (elements[top]);
	}
	unsigned size() {
		return (top);
	}
	unsigned getCapacity() {
		return (capacity);
	}
	void print() {
		unsigned i = 0;
		if (top == 0) {
			printf("stack is empty.\n");
		} else {
			printf("stack contents: ");
			for (i = 0; i < top; i++) {
				printf("%d, ", elements[i]);
			}
			printf("\n");
		}
	}
private:
	unsigned* elements;
	unsigned top;
	size_t capacity;
};
#endif /* STACK_ */
