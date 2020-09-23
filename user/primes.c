#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXNUM 35

int main()
{
  int first_fd[2];
  int second_fd[2];
  int *read_pipe;
  int *write_pipe;
  int nums[MAXNUM-1];
  
  pipe(first_fd);
  if(fork()>0){
    close(first_fd[0]);
    for(int i=0;i!=MAXNUM-1;i++){
      nums[i] = i+2;
      write(first_fd[1],&nums[i],sizeof(int));
    }
    close(first_fd[1]);
    wait((void*)0);
  }
  else{
    //正式的流水线部分
    int idx;
    int data;
    read_pipe = first_fd;
    write_pipe = second_fd;
    while(1){
      //与右边的建立一个管道
      pipe(write_pipe);
      close(read_pipe[1]);
      if(read(read_pipe[0],&idx,sizeof(int))){
        //第一次读到了数据，为自己赋值
        printf("prime %d\n",idx);
      }
      else{
        break;
      }

      if(fork()>0){
        //靠左的进程
        close(write_pipe[0]);
        while(read(read_pipe[0],&data,sizeof(int))){
          if(data%idx==0)
            continue;
          write(write_pipe[1],&data,sizeof(int));
        }
        //数据传输完毕
        close(write_pipe[1]);
        close(read_pipe[0]);
        wait((void*)0);
        break;
      }
      else{
        //新生成的进程
        //假设此时是第三个进程，idx=3;
        //此时它的read_pipe是连接到Idx=1的，write_pipe是没意义的
        //需要把write_pipe放到read_pipe上
        close(read_pipe[0]);
        int *tmp;
        tmp = write_pipe;
        write_pipe = read_pipe;
        read_pipe = tmp;
      }
    }
    exit(0);
  }
  exit(0);
}

