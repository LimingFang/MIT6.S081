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

> 简单的描述内核是如何启动并运行第一个进程的

- boot loader（ROM）：将Kernel载入内存，在machine mode下从_entry开始运行（不分页）
  - 初始化内核stack
  - 调用start函数（kernel/start.c):进行一些初始化设置并设置好时钟中断，然后调用`mret`返回supervisor mode（正常来说`mret`用于从之前的监管模式返回机器模式，但是这里只是提前虚拟设置好监管模式，然后返回）。具体来说，通过设置`mstatus`寄存器，同时在`mepc`中存放main函数地址
- main：初始化一些设备、子系统。开始创建第一个进程（调用`userinit`），在`user/initcode.S`中以汇编语言调用`exec`返回内核，形成`init`进程，最终生成console



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



