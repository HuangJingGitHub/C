#include <stdio.h>
#include <math.h>

float Newton(float a, float b, float c, float d)
{  float x0 =1.5, x=1, f, h;
   f = a*x0*x0*x0 + b*x0*x0 + c*x0 + d;
   h = f/(3*a*x0*x0 + 2*b*x0 + c);
   x = x0 - h;
   for( ; fabs(x-x0) >= 1e-5; )             // fabs()��Ӧ���Ǹ���������abs()���ص���int�ͣ�Newton�����о����¾�x 
    {  x0 = x;                              // ֮��Ĳ�ֵ��carefully 
       f = a*x0*x0*x0 + b*x0*x0 + c*x0 + d;
	   h = f/(3*a*x0*x0 + 2*b*x0 + c);
       x = x0 - h;
     }
    return x;
 } 
 
 int main()
 {  float a, b, c, d, x0;
    printf("Input the cofficients of the equation:\n");
    scanf("%f %f %f %f", &a, &b, &c, &d);
    printf("The equation is: %fx^3+%fx^2+%fx+%f=0\n", a, b, c, d );
    x0 = Newton(a,b,c,d);
    printf("The approximated root is %f.\n", x0);
 }
