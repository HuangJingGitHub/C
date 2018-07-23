#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float det(float A[20][20], int n)       // ������ʽ�ĺ���Դ��Matrix_det.c 
{  float k, det=1.0;
   int r, i, j, flag = 0;
   
   for(r=0; r<=n-1; r++)
   {  if(A[r][r]==0)
      { for(i=r; i<=n-1; i++)
        {  if(A[i][r]!=0)               // ��������Խ����ϵ�Ԫ��Ϊ��������Ҫ���д������� 
           {  flag = 1;                 // ��������Ԫ�ز�Ϊ����У�����Ϊ���У����ı�����ʽ��ֵ 
		      for(j=r; j<=n-1; j++)
              A[r][j] = A[r][j] + A[i][j];
              break;    
           }
	    }
	   if(flag==0)
	   {  det = 0;
	      return det;
	   }
      }
      
     for(i=r+1; i<=n-1; i++)                // �Խǻ�����ע��ϸ�ڣ�����һ��Ҫ�� k ��ϵ�����£�����֮���õ� 
     { k = -A[i][r] / A[r][r];                // ϵ������0 
	   for(j=r; j<=n-1; j++)
		     A[i][j] = A[i][j] + k*A[r][j];
     }
   det = det * A[r][r];
   }
   return det;
}

int main()
{
  float a[20][20], 
        b[20][20], 
        temp[20][20], 
		inv[20][20] = {0}, 
		det_a;
   int n, i, j, i1, j1, i2, j2, 
       flag1 = 0, 
	   flag2 = 0;
   
   printf("Please input the degree of the matrix:\n");
   scanf("%d",&n);
   printf("Please input the entries row by row:\n");
   
   for(i=0, j=0; i<n; i++, j=0)        // ע���������ķ��� 
   {  do
      {  scanf("%f",&a[i][j]);         // ���������aʱ��ͬʱ����a��b����Ϊ����������ʽʱ�Ὣ����Խǻ����滻ԭ���� 
         b[i][j] = a[i][j];            // ��������ʽʱ��Ҫ�õ�ԭ�������Ը���a(�Կռ任ʱ�䣩;����Ҳ���������������� 
         j++;                          // ֮���ټ���a������ʽ������ʱ���det(a) == 0����������û���ã��˷Ѽ���ʱ�� 
         if(j>n)
         {  printf("INPUT DIMENSION ERROR !");
            exit(1);
         }
      }
      while((getchar())!='\n');
   }
   
   det_a = det(b,n);      
   if(det_a == 0)
   {  printf("The matrix is singular!");
      exit(0);
   }
   
   for(i = 0; i<n; i++)
     for(j = 0; j<n; j++, flag1 = 0)          // ��������flag֮��һ��Ҫע���Ƿ���Ҫѭ����ʼ��,��ʱ���ײ�� 
     {  for(i1 = 0; i1<n; i1++, flag2 = 0)
		   if(i1==i)
           { flag1 = 1;
               continue;
		   }
           else
             for(j1 = 0; j1<n; j1++)       
             {
			    if(j1==j) 
                { flag2 = 1;
                  continue;            
				}                      
				                       
				if((!flag1) && (!flag2))      // ����ʽ��ԭ����֮��Ľű��Ӧ���� 
				  temp[i1][j1] = a[i1][j1];
				else if((flag1) && (!flag2))
				  temp[i1-1][j1] = a[i1][j1];
				else if((!flag1) && (flag2))
				  temp[i1][j1-1] = a[i1][j1];
				else
				  temp[i1-1][j1-1] = a[i1][j1];
				  
			 }    
		inv[j][i] = pow(-1,i+j) * det(temp, n-1);  
    }
    
  printf("The inverse is:\n");
  for(i=0; i<n; i++)
    for(j=0; j<n; j++)
      if(j == n-1)
      printf("%-5.3f\n", inv[i][j] / det_a);
      else
      printf("%-5.3f  ", inv[i][j] / det_a);
}
 
 
