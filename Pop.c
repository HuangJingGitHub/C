#include <stdio.h>
int main()
{  int input[50] = {0}, 
       num = 0,
	   isOrder = 0,
	   i, j, temp;
	   
   printf("Please input the array:\n");
   do
     {  scanf("%d",&input[num]);
        num++;
	 }
	 while ((getchar())!='\n');    // !!! ע����do-while �ṹ���������룬������һ�����ݾͿ��Ա��������飬��while����ǰ�Ļ�
	                               // ��һ�����ݱ���Ϊgetchar()���벻�ܱ�������ա� 
	 
	printf("The  original array is:\n"); 
    for(i=0; i<num; i++)
	printf("%d  ",input[i]);
	printf("\n");
	
	for(j=1; j<num; j++)
	{ if (isOrder == 0)
  	  {
	    for(i=0; i<num-j; i++)
	      {  isOrder = 1;          // Ϊ���ð�ݳ���Ч�ʣ������жϣ��޽������������Ѿ���˳��ṹ�������ٽ��бȽϣ�ÿ�ζ�Ҫ��ʼ�� 
		     if (input[i]>input[i+1])
	           {  temp = input[i+1];
	              input[i+1] = input[i];
	              input[i] = temp;
	              isOrder = 0;     // �в���������������˳��ṹ 
	           } 
	    }
	  }
	  else
	    break;
	}
	
	printf("The array in ascending order is:\n");
    for(i=0; i<num; i++)
	printf("%d  ",input[i]);
	printf("\n");
		
}
