/*Function: ���ַ���������ǰ����$*/ 
#include <stdio.h>

int Adder(char *s)
{
   int i, j;      // �ַ����ĸ��ƱȽ�˼· 
   char t[100];
   for(i=0; s[i]!='\0'; i++)
     t[i]=s[i];
	 t[i]='\0'; 
   for(i=0, j=0; t[i]!='\0'; i++,j++)
   {  if(t[i]>='0' && t[i]<='9')
      {  s[j]='$';
         j++;
         s[j]=t[i];
	   }
	  else
	    s[j]=t[i];
   }
   return 0;
}

int main()
{  
   char ori[100];
   printf("Please input the string:\n");
   scanf("%s",ori);
   Adder(ori);
   printf("The new string is: %s\n",ori);
   
}
