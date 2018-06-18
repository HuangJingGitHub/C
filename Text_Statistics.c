// ��������C�����������ṩ�Ĵ��룬�߼��жϲ������в�ͬ��ͳ��һ���ı�
// ����Ϣ������ע��ע�͵�ϸ�� 
#include <stdio.h>
#include <string.h>

int *getCharNum(char *filename, int *totalNum);

int main()
{  char filename[30];
   int totalNum[3] = {0,0,0};
   
   printf("Input the file name:\n");
   scanf("%s",filename);
   
   if(getCharNum(filename, totalNum))
     printf("Total: %d line(s);  %d word(s);  %d char(s)", totalNum[0], totalNum[1], totalNum[2]);
   else
     printf("Error��\n");
    
    return 0;
}

int *getCharNum(char *filename, int *totalNum)
{  FILE *fp;
   char buffer[1003], c;
   int bufferlen, i,lineNum = 0, isLastBlank = 1, charNum = 0, wordNum = 0;
   
   if((fp=fopen(filename, "rb")) == NULL)
   {  perror(filename);
      return NULL;
   }
   
   printf("Line Words Chars\n");
   while(fgets(buffer, 1003, fp)!= NULL)         // ����fgets()���������� 
   {  bufferlen = strlen(buffer);
      for(i=0; i<bufferlen; i++)
      {  c = buffer[i];
         if(c != ' ' && c !='\n' && c !='\r')   // ע��Windowsϵͳ���ı�һ�н�����־��\n\r, ���мӹ���ƶ������� 
         {  charNum++;                          // �ǿո񣬷ǻ��У����ַ�����1 
            isLastBlank = 0;
         } 
         else if(c == ' ' && (!isLastBlank))
         {  wordNum++;
            isLastBlank = 1;
		 }
	  }
      
      !isLastBlank && wordNum++;             // �ж���β�Ƿ��Կո�����������Կո�����򵥴���Ҫ��1
	                                         // ͬʱ��a && b����������൱�� if(!a) b, 
	  totalNum[0] ++;
	  totalNum[1] += wordNum;
	  totalNum[2] += charNum;
	  printf("%-7d%-7d%d\n", totalNum[0], wordNum, charNum);
	  
	  charNum = 0;                          // ���³�ʼ�� 
	  wordNum = 0;
	  isLastBlank = 1;
   }
   return totalNum;
}
