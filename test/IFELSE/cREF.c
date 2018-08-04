
#include <stdio.h>
int x=55;
int y=44;
int main()
{

    if(x<y){
        x=x*45;
    } else{
        y=y+5;
    }

    printf("%d\n", x);
    printf("%d\n", y);

    return 0;
}
