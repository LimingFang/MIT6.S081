# 阅读xv6手册以及上课的一些记录

## Chp1：操作系统接口

### 前言

操作系统功能在于管理硬件并向上层应用提供服务。操作系统将底层硬件进行管理、抽象

操作系统通过接口向上层提供服务

学习xv6可以帮助理解类unix系统接口设计

xv6采用传统的内部结构，即内核是一个特殊的程序，进程通过系统调用陷入内核，运行完成后返回



第一章主要介绍xv6提供了哪些服务

#### 进程与内存

进程包括用户内存地址空间以及进程状态，进程间的切换是等时的，其他包括`fork`等进程操控原语在CSAPP里学过一次，这里就不提了

#### I/O、文件描述符

文件描述符是一个整数，进程可以通过其写入或者读出数据，文件描述可以是文件的，也可以是管道、设备或者其他文件描述符，这里做了一层非常好的抽象

更底层的说，文件描述符是访问文件描述符表的index，每个表项纪录某个文件的offset等信息

#### Pipes

管道以文件描述符形式提供给进程使用，其实是一块缓冲区，一端写入一端读出

#### 文件系统

xv6实现的文件系统包括普通文件（字节流）、文件夹以及对应的操作函数

#### real world

Unix系统调用被POSIX标准化，xv6不属于该标准，只求简洁并尽量像unix



#### why

hard && interesting

CS的核心课，设计操作系统兼顾抽象简洁安全与高效灵活

有助于写更高效的程序



#### 课程学习

- 视频
- Labs
- Reading





## Chp2: Operating system organization

操作系统三个基本要求：

- 同时能运行一些程序，并且每个程序都应该有机会运行（复用）
- 程序运行环境应当是隔离的
- 程序应当有方式进行通信

`*monolithic kernel*`->主流实现满足上述要求

### 抽象硬件资源

OS的意义？当上层程序直接使用硬件资源时，对上层程序提出很高的要求，因此利用OS抽象硬件资源非常重要。

如果直接使用，需要考虑潜在出错的可能以及CPU复用。而OS提供了隔离的运行环境，同时提供的服务也简化了程序的编写。

### 模式

操作系统需要建立不同的模式，使得isolation成为可能，因为不能让某个应用程序毁坏整个环境

CPU硬件层面提供了可能（RISC-V）

- machine mode（主要用于启动时的初始化）
- supervisor mode（特权指令）
- user mode

在supervisor mode和user mode之间通过指令切换

### Kernel org

> 已经提出了不同的CPU运行模式，需要了解操作系统的哪部分应当运行在supervisor mode

`monolithic mode`是整个操作系统内核都运行在supervisor mode下

优点：系统内部享受所有特权指令，内部合作方便，等

缺点：比较复杂，易出错

### xv6 org

> 内部模块接口定义在kernel/defs.h中

整体定义在kernel文件夹中，模块化的形式

### 进程

进程提供了程序独立运行的环境，进程甚至为程序抽象出了独立地址空间、独立逻辑控制流

内核需要维护每个进程的状态，在`/kernel/proc.h`中

每个进程有两个stack：user and kernel stack

> 如何调用系统调用的：通过ecall指令提高特权级别，改变PC，切换到kernel stack,运行kernel ins

### Starting

> 简单的描述内核是如何启动并运行第一个进程的，后续章节描述细节

**配合xv6源代码理解启动过程**

- 首先在machine mode下运行
  - 启动后最先运行的是`kernel/entry.S`：主要是为内核代码建立stack，然后跳转到`kernel/start.c`
  - **start**负责建立时钟中断、进行部分初始化，最后跳转到supervisor mode的`kernel/main.c`注：其跳转主要是依靠`mret`指令
    - `mret`指令正常情况下是负责从supervisor mode->machie mode
    - 但是这里首先在`mstatus`寄存器中存放supervisor mode，在`mepc`寄存器中存放`main`函数的地址，然后返回到`main`函数
- 此时是supervisor mode
  - `kernel/main.c`：此时需要完成设备和一些子系统的初始化配置，例如分页机制、中断向量表、文件表、设备中断配置。最后跳转到`kernel/proc.c`中的`userinit()`函数为转向user space做准备----建立第一个进程
  - `kernel/proc.c:userinit()`：生成第一个进程，配置proc结构体，函数结束后以某种方式跳转到了`kernel/initcode.S`
  - `kernel/initcode.S`：调用exec系统调用重新陷入内核变成用户space的第一个init进程
  - init进程：在user space下运行，配置文件描述符，建立终端console，完成启动
  - 





## Chp3:Page tables

### 简介

页表机制帮助每个进程实现拥有独立的内存地址空间，即虚拟地址（VA)

虚拟地址的页表不仅负责VA->PA的翻译，还负责管理PA的访问控制，同时，页表同样使得不同进程的VA映射到相同的PA实现共享

这一章介绍页表机制以及如何使用



### Paging hardware

xv6在Sv39 RISC-V上运行：地址64位，只用了底部的39位，每一页是4KB，因此VPN占了27位，VPO占了12位，一共有$2^{27}$个页表项（PTE)，PPN一共有44位，加上12位的PPO，物理地址一共是56位

翻译的过程是：将VA划分成25+27+12三部分，高25位没有用到（目前），中27位作为VPN(index)到page table中查找对应的PTE，得到PPN，配合低12位生成VA（56位）

**多级页表**

如果只有一个页表，需要容纳$2^{39}$个PTE，显然是不显示的，因此引入多级页表，Linux使用的是4级页表，Xv6使用三级页表，将VPN划分成9+9+9的形式，一级页表配合PTBR查找到一级PTE，得到二级页表的基地址，配合第二个9bits查找，以此类推

更具体的说，可以将39位的虚拟地址分成$2^{27}$个page，每个page有4KB，同时每一级的页表管理512个PTE，也就是说一级页表将虚拟地址按照1GB进行切分，二级页表在每个大的1GB范围内按照2MB进行切分，三级页表在2MB范围内按照4KB进行切分。随着页表搜索的递进，虚拟地址搜索范围都大大减小

> **PTE：**每一个页表项内容包括44位的基地址和10位的flag信息，占用8bytes

一旦某个连续地址空间内都没有映射，则多级页表相应位置是NULL，后续就不会分配PTE，省内存

每个PTE的10位flag包含：

- PTE_V：PTE是否存在
- PTE_R：是否Readable
- PTE_W：是否Writeable
- PTE_X：是否可执行
- PTE_U：user mode下是否可执行

PTBR在xv6称为satp寄存器



#### Kernel address space

内核也需要将自身的VA映射到PA，在`kernel/memlayout.h`中有相关定义

QEMU模拟与设备的交互是通过读写相关内存地址完成的

首先运行的是物理地址0x80000000->0x86400000的内容，这段地址是直接映射的，即PA=VA，内容包括内核代码，内核数据，free memory

其他内容包括：

- trampoline page：位于VA顶部，被映射到0x80000000开始的地方（那地方被映射了两次，后面会解释）
- stack page：上下都有guard page（invalid），用来防止overflow



### 创建地址空间

`kernel/vm.c`中定义了操作地址空间和page的操作

`pagetable_t`定义的是指向root page-table（一级页表)的结构体指针，既可以作为用户进程页表，也可以作为kernel page-table使用。本身是一个指向uint64的指针

`walk`用来在页表中查询PTE，具体过程是：给定一个页表的基地址，va，以及alloc选项参数，通过shift等实现定义好的Macro，获取va的2，1，0级页表的Index，最终返回的是一个指向该页表项的64位整数指针。

`mappages`建立新的映射，即给定pa，va以及页表基地址及需要分配的大小，以及perm选项参数，将va->va+size的虚拟地址映射到以pa为起始的一段连续内存上，所谓建立映射，本质是建立在pagetable里设置PTE

kvm开头的函数操作kernel页表，uvm操作用户页表

`copyout`和`copyin`是工具函数，将数据在用户和内核地址空间之间传递，在系统调用时会用到



在boot时，运行到`kernel/main.c（`初始化一些子系统）时，就调用`kvminit`创建内核页表

具体来说，首先在PA中分配one page to hold root-table page，然后调用`kvmmap`对内核内存空间进行mapping。

> `kvmmap`调用`mappages`

`kvmmap`：调用mappage函数建立内核地址空间的映射

创建内核地址空间还包括为每个进程（实现定义好最大进程数）创建一个内核栈，即每次分配一个物理页与内核地址空间的指定va建立映射，最后更新SATP寄存器



### 物理内存分配

---

#### 初始化分配器

> 现在不用管自旋锁



### 进程地址空间

---

用户进程地址空间分布：（以page为单位）

- trampoline
- trapframe
- heap
- stack：只有一页大小
- guard page（为了防止stack overflow）
- data
- text



### sbrk

---

系统调用用于分配or缩减内存

Sbrk->growproc->uvmalloc+uvmdealloc

分配内存，即分配出空闲的物理地址，建立PTE，更新用户页表

#### 分配

`kalloc`分配物理地址，`mappages`建立PTE

#### 缩减

`walk`查找PTE,`kfree`释放物理地址



### exec（源码分析）

---

exec系统调用用于创建用户进程，关键一步在于替用户进程创建页表

- `proc_pagetable`：创建用户进程页表

  - 申请one page物理内存用来存放二级页表
    - `uvmcreate`：底层是调用`kalloc`
  - 建立映射
    - 将trampoline code映射到TRAMPOLINE的va处，用于进程的return，代码是实现写好的
    - 将trapframe映射到进程的trapframe，地址就在trampoline下一页
  - 映射建立完毕
  - 返回创建好的页表

- 检查ELF header

  - elf结构体是 `struct elfhdr`，后面跟着program section header `struct proghdr`
  - 首先检查ELF header是正确的，即核对magic number
  - 调用`proc_pagetable`创建用户进程页表

- load program

  - 将每个程序段放到正确的位置，建立映射

- 创建user stack

  - `uvmalloc` ：在给定的虚拟地址区间内，申请物理内存并建立映射

  - user stack只有one page size，但是申请two page size，在stack下方的一页是guard page防止stack overflow

  







## Chp4:Traps and System calls

### 系统调用

> 调用系统调用时发生了什么

在initcode.S运行时，先将参数放在对应的寄存器中，然后调用ecall指令陷入内核,调用uservec(trampoline.S),调用usertrap,然后调用syscall

syscall:取出系统调用编号，在syscalls中查找到对应函数，例如`SYS_fork`然后运行

> 参数如何传递

上层C函数参数放在user registers中，kernel的trap code将其转移到trap frame中

- argint
- argaddr
- argfd



