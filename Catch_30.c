#include <stdio.h>    // catch 30����Ϸ�������������������׷��������һ�ο�ѡ1��2��֮��������Լ�1���2�������������3��������������� 
#include <stdlib.h>   // �ȴﵽ30�߻�ʤ�� �ɵ��ƿ�֪����ʤ�����ǰ���ס3�ı��� 
#include <time.h>
#include <math.h>

int input(int cs);  // cs:current sum
int computer_put(int cs);
int judge_series(int *a, int t1, int t2);

int dif[2] = {0, 0};  // ������¼�������ȫ�ֱ��� 

int main()
{  int sum = 0;
   srand((unsigned)time(NULL));  // ע��C��ʵ�����������������ǣ�stdlib.h time.h srand((unsigned)time(NULL)) rand() ��rand()ÿ��������� 
   if(rand()%2)                  // ���������0��1��1������ȳ��� 
   {  sum = input(sum);
      dif[0] = sum - 0;          // ��ʼ����¼�������ȫ�ֱ��� 
	} 
     
   while(sum!=30)               // ע����ǰ�����֮����߼�������˭�ȷ���������ʵ���������� 
     if((sum=computer_put(sum))==30)
       printf("*****Sorry You Lose The Game!*****\n");
     else if((sum=input(sum))==30)
       printf("*****Congratulations�� You Win The Game!*****");
     
 }

int judge_series(int *a, int t1, int t2)
{  *a = *(a+1);
   *(a+1) = t2 - t1;
   printf("***dif1 = %d dif2=%d***\n",*a,*(a+1));
   if(*a==1 && *(a+1)==1)
     return 1;
   else
     return 0;
}

int input(int cs)
{  int temp, flag;
   printf("Please input the number:\n");
   scanf("%d",&temp);
   flag = judge_series(dif, cs, temp);
   printf("***flag = %d***\n",flag);
   while((temp-cs)>2 || (temp-cs)<1 || flag || temp>30 )
   {  printf("Invalid input! Please input again:\n");
      scanf("%d",&temp);
      flag = judge_series(dif, cs, temp);    // �ж�ͬʱ��judge_series()�л��¼���³����� 
   }
   printf("You Count: %d \n", temp);
   return temp;
 } 
 
int computer_put(int cs)
{  int temp;
   if(cs%3==1)
   {  cs+=2;
      dif[0] = dif[1];                      // ���³����� 
      dif[1] = 2; 
      printf("Computer Counts: %d \n", cs);
   }
   else if(cs%3==2)
   {  cs = cs + 1;
      dif[0] = dif[1];
      dif[1] = 1; 
      printf("Computer Counts: %d \n", cs);
   }
   else
   {  temp = rand()%2 + 1;
      cs = cs + temp;
      dif[0] = dif[1];
      dif[1] = temp; 
      printf("Computer Counts: %d \n", cs);
   }

   return cs;
     
}

