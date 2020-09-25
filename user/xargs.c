#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

/*
 * 管道stdin以空格为分隔，遇到\n进行下一次操作
 */

int
main(int argc,char* argv[])
{
  if(argc<2){
    fprintf(2,"The args nums are wrong\n");
    exit(0);
  }
  
  char *_argv[MAXARG+1];
  memset(_argv,0,MAXARG);
  int pid;
  char *pathname = argv[1];
  for(int i=1;i!=argc;i++)
    _argv[i-1] = argv[i];
  
  char c[128];

  while(gets(c,128)){
    int len = strlen(c);
    if(len<1)break;
    c[len-1] = 0;
    int _argc = argc-1;

    for(int i=0;i<len-1;){
      if(_argc==MAXARG){
        printf("Too many args\n");
        exit(0);
      }
      if(c[i]==' '){
        c[i] = 0;
        i++;
        continue;
      }
      _argv[_argc++] = &c[i];
      while(i<=(len-1)&&c[i]!=' ')i++;
    }
    _argv[_argc] = 0;
    
    if((pid=fork())<0){
      fprintf(2,"Fork wrongly\n");
      exit(0);
    }
    if(pid==0){
      if(exec(pathname,_argv)<0){
        fprintf(2,"Execve wrongly!\n");
        exit(0);
      }
    }
    if(pid>0)
      wait((void*)0);
  }
  exit(0);
}
