# Lab pgtbl

## 背景

该Lab与Chp3内容相关，主要介绍xv6的分页机制，内核地址空间说明及创建，进程地址空间介绍以及exec创建进程



## vmprint

**目标**：给定页表基地址，打印出该页表及随后两级页表的vaild PTE以及对应的physic address

> 比较简单，pass





### kernel page table per process

---

**背景**：内核需要访问进程地址空间的内容，但是最开始是没有进程的页表的，因此内核是无法了解进程的va->pa的映射的。

**目标**：令每个进程拥有内核页表备份

内核通过`allocproc`生成进程proc结构体，通过在proc结构体中增加`pagetable_t kernel_pagetable`，在生成进程结构体时，重新生成一份一模一样的内核页表，然后赋值给struct proc即可。

在`kernel/proc.c`中定义函数`proc_pagetable`完成复制内核页表的任务



### copyin/copyinstr

---

#### copyin工作原理

`walk`：在给定页表中查找虚拟地址的PTE，并且允许创建PTE，if needed

`walkaddr`：在给定页表中，查询va对应的pa，只能查询用户能访问的page

`memmove`：从src->dst复制移动n字节；**注**：CPU发送出的全是虚拟地址，是要经过页表转换的，之所以在`copyin`中用pa，是因为内核的页表几乎都是direct map的

具体来说，copy-paste得一页一页的进行，每次都要去查找对应的pa

现在可以把user‘s mapping直接存入每一个进程的kernel pagetable中，这样直接操作虚拟地址即可，不需要翻译。但是这样做有个限制，即用户进程虚拟地址空间不能高于内核使用的最低地址例如代码和数据，在该部分来说，不能高于0x0C000000，即192MB。

在回看一下用户进程生成时的地址映射情况：用户地址是从0开始，自下而上增长的（除了MAXVA附近两个page，但那个不是在用户态使用的，因此不用考虑。例如在exec函数中，将程序段依次载入，然后在分配两页，一个是guard page，一个是stack page。

因此要将用户进程的mapping放入process kernel pagetable，需要在进程首次创建、进程fork、进程内存地址增长三种情况及时更新process kernel pagetable。









