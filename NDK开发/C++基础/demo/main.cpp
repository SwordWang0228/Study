#include<iostream>
#include "func.h"
using namespace std;

// void* ������ָ�룬ָ�����Ͳ���Ҫ���� 
int* addFunc(void* param) {
	cout << "ָ�뺯��" << endl;
	int a = 10;
	int *b = &a;
	return b;
}

int main() {
	int a = 10; 
	addFunc(&a);
	return 0;
} 

int func(){
	printf("����");
	return -1;
}

// �������� 
int func(int num) {
	printf("����");
	return -1;
}

