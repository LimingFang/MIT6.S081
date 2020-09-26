# Lab syscall

## 简介

该lab核心还是设计新的系统调用：`trace`和`sysinfo`，但是不同于util lab里直接使用现成的C函数库，它需要阅读部分kernel code，然后修改kernel code，算是第一次对kernel有了具体的认识



## trace

该系统调用用于后续Lab的Debug，函数原型是

`int trace(int mask)`

他接受一个mask参数，int一共32位，设置成1的那一位表示是需要trace的系统调用，具体系统调用的编号，在`kernel/syscall.h`中有。一旦设置成1，在每次调用该系统调用时，函数返回前会打印一条信息，包括

- 进程号
- 系统调用名称
- 返回值

为了完成trace函数，需要了解系统调用具体是如何运行的

### 系统调用

xv6系统部分模块结构为：

- `user/user.h`和`user/usys.pl`：前者是用户空间系统调用的函数原型，后者是perl文件，在操作系统编译时会自动转换成`usys.S`,是系统调用的汇编代码文件。具体来说，在用户调用系统调用时，相应的参数会以规定的顺序放在相应寄存器中，汇编代码只是把系统调用的编号放在`a7`寄存器中，用来通知后续代码是何种系统调用。在最后会统一调用`ecall`指令陷入内核
- `kernel/syscall.h`和`kernel/syscall.c`：作为kernel中负责系统调用“转发”的部分。头文件定义了不同系统调用的编号，例如`fork`是1；源文件中`syscall`函数根据`a7`寄存器调用不同真正的系统调用函数，完成后返回用户层
  - `syscall.c`和`usys.S`功能并不重复，因为一个在user space,一个在kernel space，特权级别不一样
  - **参数获取**：int、pointer、file discriptor。参数最初是存放在寄存器中的，`syscall.c`定义了`argint`, `argaddr`和`argsstr`，可以根据参数序号获取参数

- `kernel/proc.h`和`kernel/proc.c`：头文件定义了和进程相关的结构体`proc`以及CPU相关结构体`cpu`，`proc`包含了诸如当前进程号等信息，源文件定义了操作进程的一些函数.
  - trace系统调用需要在系统进程级别跟踪其他系统调用，因此需要在进程结构体中添加一个int变量（即mask），可以命名为`traced_syscall`
- `kernel/sysproc.h`和`kernel/sysproc.c`：系统调用源代码,例如`sys_exit`，不过大部分也调用了其他模块的函数

### sys_trace

`kernel/proc.c`中定义了获取当前进程结构体的函数`myproc`直接获取进程，将mask赋值即可。（先利用`argint`获取mask）



## sys_info

该系统调用记录当前的空闲内存和运行进程数

复杂之处在于完成结构体在user space和kernel space的传递

- the address maybe buggy
- kernel's mapping from vm to physic addr is different from user's.

不过xv6已经写好了工具函数，有封装过的，也有更底层的

- `copyinstr`
- `copyin`
- `fetchstr`
- `copyout`

如果是获取user space的内容，需要指定内核的dstva以及进程的pagetable号以及srcva，以及bytes数

如果是返回至user space，需要指定内核的srcva以及进程的pagetable号以及dstva，以及byte数

user space的va是uint64的类型



另一个麻烦的地方在于统计当前空闲的内存，在`kernel/kalloc.c`中定义了一些操作分配器的函数，最小单位是Page，在`kmem`结构体中定义了free page的地址，统计个数乘pagesize即可

至于进程个数，有`proc`结构体数组，循环统计不是UNUSED状态的即可



## 完结

总体来说，阅读了一些kernel code，感觉认知具体了一些