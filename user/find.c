#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path,char *p,char *filename){
  //递归实现
  int fd;
  struct stat st;
  char buf[512];
  struct dirent de;
  
  if((fd=open(path,0))<0){
      fprintf(2,"find: cannot open %s\n",path);
      return;
  }
  if(fstat(fd,&st)<0){
    fprintf(2,"find: cannot stat %s\n",path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_DIR:
    //先判断是不是.和..,不是的话继续while->find
    strcpy(buf,path);
    p = buf+strlen(buf);
    *p++ = '/';
    //p指向的是文件(夹)名字的第一个字符
    while(read(fd,&de,sizeof(de))==sizeof(de)){
      if(de.inum==0)
        continue;
      memmove(p,de.name,DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      if((strcmp(p,".")*strcmp(p,".."))!=0)
        find(buf,p,filename);
    }
    break;
  case T_FILE:
    //判断有没有找到文件
    if(strcmp(p,filename)==0){
      printf("%s\n",path);
      return;
    }
    break;
  case T_DEVICE:
    //PASS
  default:
    break;
  }
  close(fd);
}

int
main(int argc,char *argv[])
{
  char file2find[512];
  char* path = argv[1];
  
  strcpy(file2find,argv[2]);
  find(path,path,file2find);
  exit(0);
}

