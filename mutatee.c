#include <stdio.h>
#include <unistd.h>

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

void quebec(int a)
{
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
		quebec(18);
		awesome("Good Morning", "Good Night");
		beauce(24,"Life is like a box of chocolate");
		montreal(24,"Life is like a box of chocolate", 'p');
		sherbrook(24,"Life is like a box of chocolate", 'p', &var);

	}
	return 0;
}
