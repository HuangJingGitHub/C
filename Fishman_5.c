#include <stdio.h>

int main()
{  int x[5], i=1, flag = 0;    // ������˷������⣬�ýⷨ���У��������������ӣ��򲻿���д���if��� 
                               // ��Ӧʹ�õݹ麯����� 
   for(;flag == 0; i++) 
   {  flag = 1;
      x[0] = i*5+1;
      if(x[0]%4 != 0)
      {
	     flag = 0;
	     continue;
	  }
	  x[1] = x[0]/4*5+1;
	  if(x[1]%4 != 0)
      {
	     flag = 0;
	     continue;
	  }
	  
	  x[2] = x[1]/4*5+1;
	  if(x[2]%4 !=0)
      {
	     flag = 0;
	     continue;
	  }
	  
	  x[3] = x[2]/4*5+1;
	  if(x[3]%4 !=0)
      {
	     flag = 0;
	     continue;
	  }
	  
	  x[4] = x[3]/4*5+1;
   } 
    printf("The minimum numer of fish is %d:\n",x[4]);
}
