#include <stdio.h>
int x=33;
int y=44;

int main()
{
    while (x<y){
        x=x+1;
        y=y-4;
        printf("%d\n", x);
        printf("%d\n", y);
    }
    return 0;
}
