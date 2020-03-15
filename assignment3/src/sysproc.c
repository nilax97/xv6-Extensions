#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_fork(void)
{
  return fork();
}

int sys_exit(void)
{
  exit();
  return 0; // not reached
}

int sys_wait(void)
{
  return wait();
}

int sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void)
{
  return myproc()->pid;
}

int sys_sbrk(void)
{
  int addr;
  int n;

  // cprintf("sbrk called %d\n",myproc()->pid);

  if (argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

int mem_log = 0;

void make_pt_entry(int n)
{
  int pid = myproc()->pid;
  static int i = 0;
  int address = 4096 * i;
  i++;
  if (mem_log == 1)
  {
    cprintf("%d -> %d%d\n", address, n, pid);
  }
}

int sys_make_page_table_entry(void)
{
  int n;
  if (argint(0, &n) < 0)
    return -1;
  make_pt_entry(n);
  return 0;
}

int sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_scheduler_log(void)
{
  int n;
  if (argint(0, &n) < 0)
    return -1;
  scheduler_log(n);
  return 0;
}

int memory_log(int n)
{
  mem_log = n > 0;
  return mem_log;
}

int sys_memory_log(void)
{
  int n;
  if (argint(0, &n) < 0)
    return -1;
  memory_log(n);
  return 0;
}

int sys_get_container_id(void)
{
  return myproc()->container_id;
}