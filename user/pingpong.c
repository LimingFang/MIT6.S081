#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc,char *argv[])
{
  /* 
   * the parent process send a byte to child, the child print a message, and write it back and exit
   * then parent read the byte then print the message
   */
  int pid;
  int par_fd[2];//parent write
  int chd_fd[2];//child write

  
  pipe(par_fd);
  pipe(chd_fd);
  if((pid=fork())<0){
    printf("something wrong with fork\n");
    exit(1);
  }
  if(pid==0){
    //child process
    char b[1];
    if(read(par_fd[0],b,1)==1){
      printf("%d: received ping\n",getpid());
      write(chd_fd[1],b,1);
      exit(0);
    }
  }
  else{
    char b[1] = {"a"};
    write(par_fd[1],b,1);
    if(read(chd_fd[0],b,1)==1){
      printf("%d: received pong\n",getpid());
      exit(0);
    }
  }
  exit(0);
}

