#include <stdio.h>
#include <stdlib.h>

float *matrix_multip(float a[20][20], float b[20][20], int m, int n, int q)
{  static float p[20][20] = {0};
   int i, j, k;
   for(i = 0; i < m; i++)
     for(j = 0; j < q; j++)
       for(k = 0; k < n; k++)
          {  printf("%f %f\n",a[i][k], b[k][j]);
             p[i][j] += a[i][k] * b[k][j];
		  }
    
//	printf("%f  %f \n%f  %f", p[0][0], p[0][1], p[1][0], p[1][1]);
    return p;
 } 
 
int main()
{  int Ar = 0, Ac = 0, Br = 0, Bc = 0,
       Ac_1, Ac_2, Bc_1, Bc_2,
       flagA = 0, flagB = 0;
   static float  *ans;
   
   float A[20][20] = {0},
         B[20][20] = {0};
    
   printf("Please input matrix A (use ; to donote a row and space to denote an entry):\n");
   do
   { do 
     {  scanf("%f",&A[Ar][Ac]);
        Ac++;
     }while((getchar()) != ';');     //�����漰��scanf()��getchar()��ϸ�ڣ�scanf("%f",&a)ֻ��ȡ���ֲ��� 
	                                 // ��������78kj����78; ����a=78. һ��getchar()��Ӧһ����ǰ����
		Ar++;                        
	    Ac_2 = Ac_1; 
	    Ac_1 = Ac;
	    Ac = 0;
	  
	 if(Ac_1 != Ac_2 && Ar != 1)
	   flagA = 1;
    }while((getchar()) != '\n');
    
    if(flagA)
    {  printf("Invalid matrix input!");
       exit(1);
	}

   printf("Please input matrix B (use ; to donate a row):\n");
   do
   { do
     {  scanf("%f",&B[Br][Bc]);
        Bc++;
     }while((getchar()) != ';');
	 
	    Br++;
	    Bc_2 = Bc_1; 
	    Bc_1 = Bc;
	    Bc = 0;
	  
	 if(Bc_1 != Bc_2 && Br != 1)
	   flagB = 1;
    }while((getchar()) != '\n');
    
    if(flagB)
    {  printf("Invalid matrix input!");
       exit(1);
	}
   
   ans = matrix_multip(A, B, Ar, Ac_1, Br);
   printf("\n>>>%f  %f \n%f  %f", *ans, *(ans+1), *(ans+20), *(ans+21)); // ��ʹ float p[20][20], *a; ��a����p��a = p 
                                                                          // a[0][0]��ά��ʽ������Ҳ�ǲ��Ϸ���,a[0]һά�����ǿ��Ե� 
}
