#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int Delete(char *p1, char *p2)
{  int i = 1, j = 1;
   printf(">>>%d %d<<<\n", p1, p2);  
   for( ; &p1[i] <= p2; i++)       //��p1��p2֮�����з��Ǻ�*Ԫ�ظ��Ƶ�p1���� 
   {  printf(">>>%d %d<<<\n", &p1[i], p2);  
      if(p1[i] != '*')            
      {  p1[j] = p1[i];
         j++;
	  }
   }
   printf(">>>NEW<<<\n");  
   for( ; *(p2+1) != '\0'; p2++)
   { p1[j]='*';                // ��p2���*���ӵ����� 
     j++;
   }
  p1[j]='\0';                 
}

int main()
{  
  char ori[100], *s, *e, *flag1, *flag2;
   int i = 0;
   printf("Please input a string composed of * and letters:\n");
   scanf("%s",ori);
   for(; ori[i]!='\0';i++)
   {  if(!(ori[i]=='*' || (ori[i]>='a'&&ori[i]<='z') || (ori[i]>='A'&&ori[i]<='Z')))
        { 
		  printf("Input Error!");
          exit(1);
        }
   }
   s = ori;
   e = ori + sizeof(char) * strlen(ori) - 1;
   printf(">>>%d %d<<<\n",  s, e);  
                  // ��ָ��e��λ���ַ��������һ���ַ�(��'\0')�������ķ�ʽ�� 
                  // e=ori; while(*e) e++; e--,��������ʱĪ��bug 
   for( ;*s == '*' || *e == '*'; )
   {  if(*s == '*')
      { printf("Case1\n");
        s++;
        printf("Case1\n");
	  }
      if(*e == '*')
        e--;
   }
   
   Delete(s, e);
   printf("The new string is:\n%s\n",ori);
} 
