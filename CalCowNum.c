// Tencent��һ����Ŀ����ʼһͷ��ţ���������꿪ʼ���̣�֮��ÿ���һ�̣�����n���һ���ж�����ţ 
#include <stdio.h>

int calCow(int year)
{  int cowNum = 1, i;
   for(i=1; i<= year; i++)
   {  if(i<4)
      cowNum = 1;
      else
      {  cowNum = cowNum + calCow(year-i);    // ʹ�õݹ���������ݵĽǶ� 
	  }
   }
   return cowNum;
}

int main()
{  int year, flag = 0;
   
   printf("Input the year:\n");
   while(flag == 0)
   {  scanf("%d",&year);
      if(year>=1)
      flag = 1;
      else
      printf("Invalid input. Please input the year:\n");
   }

   printf("Number of cows: %d\n", calCow(year));
}
