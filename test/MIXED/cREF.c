
#include <stdio.h>
int x=4;
int y=5;

int main()
{
    while(x){
        if(x<y){
            y=100-y;
            if(5){ printf("%d\n", y);}
        }
        x=x-1;
        printf("%d\n", x);
    }
    return 0;
}
