# Lab COW

- COW是什么
- 如何实现



## COW

copy-on-write是指在`fork`时，子进程并不将父进程的进程地址空间的所有内存进行拷贝，而是先只复制页表和进程描述符。当父子进程中某一方需要修改内存内容时，在将那一页进行真正的拷贝



## 如何实现

- `fork`的时候调用`uvmcopy`，修改`uvmcopy`
- `kalloc`的时候需要记录ref_count，数组定义在`kalloc.c`
- `kinit`中的`freerange`在初始化时调用，需要对数组初始化：
- `kfree`中在释放物理内存时需要考虑ref_count，先对ref_count减1，减到0在进行真正的释放

- `usertrap`中断时，需要识别COW page fault，统一写一个`vm.c/uvmcow`
  - COW page fault：pte有PTE_COW并且不可写，

- `copyout`从内核空间向用户空间复制，有可能用户空间对应的page是COW page，要么在kernel内部产生中断，要不就直接修改`copyout`，【选择后者】



整体来看，一共有下面几处地方需要修改，以实现COW fork的正常工作：

- mem_rf数组初始化

- fork时调用`uvmcopy`，需要对mem_rf数组进行修改
- `usertrap`中，即发生page fault时，需要判断是否是COW page fault，这涉及到对mem_ref数组以及kmem.freelist的修改。即需要查看指向的pa有多少引用计数
- 在`kfree`中，释放pa需要考虑引用计数，即mem_ref数组
- 在`kalloc`中，需在真实分配，而是修改mem_ref数组
- 在`copyout`中，从内核物理地址pa拷贝到用户空间va所对应的物理地址处，有可能需要写的va对应的PTE是COW page的PTE，因此需要做和`usertrap`中差不多的处理，因为需要写的地址是COW page。



