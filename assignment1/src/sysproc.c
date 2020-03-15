#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
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
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

char *calls_name[] = {"sys_call0", "sys_fork", "sys_exit", "sys_wait", "sys_pipe", "sys_read", "sys_kill", "sys_exec" , "sys_fstat", "sys_chdir", "sys_dup", "sys_getpid", "sys_sbrk", "sys_sleep", "sys_uptime", "sys_open","sys_write", "sys_mknod", "sys_unlink", "sys_link", "sys_mkdir", "sys_close", "sys_print_count", "sys_toggle", "sys_add", "sys_ps", "sys_send", "sys_recv", "sys_send_multi"};

extern int calls_count[syscall_len];

int
sys_print_count(void)
{
  for(int i=1; i<syscall_len; ++i)
  {
    if(calls_count[i]>0)
    {
      cprintf("%s %d\n", calls_name[i], calls_count[i]);
    }
  }
  return 0;
}

int toggle_on = 0;

int sys_toggle(void)
{
  if(toggle_on == 0)
  {
    for(int i=0; i<syscall_len;++i)
    {
      calls_count[i] = 0;
    }
    toggle_on = 1;
  }
  else
  {
    toggle_on = 0;
  }
  return 0;
}

int
sys_add(int a, int b)
{
  argint(0, &a);
  argint(1, &b);
  return (a+b);
}

extern void print_pid(void);

int
sys_ps(void)
{
  print_pid();
  return 0;
}

int send_arr[buf_len];
int recv_arr[buf_len];
char msg_arr[buf_len][MSGSIZE];

int lock = 0;

int
sys_send(int sender_pid, int rec_pid, void* msg)
{
  while(lock > 0)
  {

  }
  lock = 1;
  char *message = (char *)msg;
  argint(0, &sender_pid);
  argint(1, &rec_pid);
  argptr(2, &message, MSGSIZE);
  int loc = -1;
  for(int i=0; i<buf_len; ++i)
  {
    if(send_arr[i] == 0)
    {
      loc = i;
      break;
    }
  }
  if(loc<0)
  {
    lock = 0;
    return -1;
  }
  for(int i=0; i<MSGSIZE; ++i)
  {
    if(message[i] == '\0')
    {
      msg_arr[loc][i] = '\0';
      break;
    }
    msg_arr[loc][i] = message[i];
  }
  send_arr[loc] = sender_pid;
  recv_arr[loc] = rec_pid;
  //cprintf("%s %d %d %s\n", "SENDING PROCESS", send_arr[loc], recv_arr[loc], msg);
  //cprintf("%s \n", "Done Sending");
  lock = 0;
  return 0;
}

int
sys_recv(void *msg)
{
  char *message = (char *)msg;
  argptr(0, &message, MSGSIZE);
  int loc = -1;
  int my_id = sys_getpid();
  //cprintf("%s %d\n", "RECIVING PROCESS", my_id);
  while(loc < 0)
  {
    for(int i=0; i<buf_len; ++i)
    {
      if(recv_arr[i] == my_id)
      {
        loc = i;
        break;
      }
    }
  }
  if(loc<0)
  {
    lock = 0;
    return -1;
  }
    while(lock > 0)
  {

  }
  lock = 1;
  for(int i=0; i<MSGSIZE; ++i)
  {
    if(msg_arr[loc][i] == '\0')
    {
      message[i] = '\0';
      break;
    }
     message[i] = msg_arr[loc][i];
  }
  send_arr[loc] = 0;
  recv_arr[loc] = 0;
  lock = 0;
  return 0;
}

int
sys_send_multi(int sender_pid, void *rec_pid, void *msg, int len)
{
  char *message = (char *)msg;
  argint(0, &sender_pid);
  argptr(2, &message, MSGSIZE);
  argint(3, &len);
  int* rec_multi = (int *)rec_pid;
  argintptr(1, &rec_multi,len);

  while(lock > 0)
  {

  }
  for(int i=0; i<len; ++i)
  {
    int loc = -1;
    for(int j=0; j<buf_len; ++j)
    {
      if(send_arr[j] == 0)
      {
        loc = j;
        break;
      }
    }

    if(loc<0)
    {
      lock = 0;
      return -1;
    }

    send_arr[loc] = sender_pid;
    recv_arr[loc] = rec_multi[i];
    for(int j=0; j<MSGSIZE; ++j)
    {
      if(*(message + j) == '\0')
      {
        msg_arr[loc][j] = '\0';
        break;
      }
      else
      {
        msg_arr[loc][j] = *(message + j);
      }
    }
  }
  lock = 0;
  return 0;
}

