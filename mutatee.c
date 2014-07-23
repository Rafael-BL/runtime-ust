#include <stdio.h>
#include <unistd.h>
#include "../buche/buche.h"

void print(int a, short b,char c, char *d, int *e)
{
	printf("%d, %d, %c, %s, %d\n",a ,b ,c ,d, *e);
}

int getAnswer(int a, char b)
{
	return 42;
}

void awesome(char *a, char *b)
{
	return;
}

void toronto(int a, int b, char c, int d)
{
	printf("%s, %d, %d, %c, %d\n", __func__, a, b, c, d);
	return;
}
void quebec(int a)
{
	printf("%s, %d\n", __func__, a);
	return;
}
void beauce(int a, char *b)
{
	return;
}
void montreal(int a, char *b, char c)
{
	return;
}
void sherbrook(int a, char *b, char c, void *d)
{
	return;
}

int main()
{
	int i, var = 18;
	for(i = 0; i < 5; ++i)
	{
		//print(1337, 86,'c', "Hello World",&var );
		sleep(1);
		getAnswer(18, 'a');
		//quebec(18);
		toronto(18, 1337, 'M', -1000);
		awesome("Good Morning", "Good Night");
		beauce(24,"Life is like a box of chocolate");
		montreal(24,"Life is like a box of chocolate", 'p');
		sherbrook(24,"Life is like a box of chocolate", 'p', &var);

	}
	return 0;
}
