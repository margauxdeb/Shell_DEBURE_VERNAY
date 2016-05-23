#include <stdlib.h>
#include <stdio.h>

int main() {
	char c;
	while (c != EOF)
	{
		c = fgetc(stdin);
		printf("%c",c);	
	}
	return 0;
}
