#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
pte_t *walk(pagetable_t pagetable, uint64 va, int alloc);

uint64 sys_exit(void) {
  int n;
  if (argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0; // not reached
}

uint64 sys_getpid(void) { return myproc()->pid; }

uint64 sys_fork(void) { return fork(); }

uint64 sys_wait(void) {
  uint64 p;
  if (argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64 sys_sbrk(void) {
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;

  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64 sys_sleep(void) {
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (myproc()->killed) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// #ifdef LAB_PGTBL
int sys_pgaccess(void) {
  // lab pgtbl: your code here.
  uint64 va;
  int num;
  uint64 abits;
  if (argaddr(0, &va) < 0)
    return -1;
  if (argint(0, &num) < 0)
    return -1;
  if (argaddr(0, &abits) < 0)
    return -1;
  pagetable_t pagetable = myproc()->pagetable;

  uint64 a = PGROUNDDOWN(va);
  uint64 va0 = a; // store the begin page
  uint64 last = PGROUNDDOWN(va + num * PGSIZE - 1);
  pte_t *pte;
  uint64 mask = 0;

  for (;;) {
    pte = walk(pagetable, a, 0);
    if (pte && (*pte & PTE_A)) { // walkaddr has set the pte_a
      mask = mask | (1 << ((a - va0) / PGSIZE));
      *pte = (*pte) & (0xffffffffffffffff & (~PTE_A));
    }
    if (a == last)
      break;
    a += PGSIZE;
  }

  printf("mask %x", mask);
  if (copyout(pagetable, abits, (char *)&mask, sizeof(mask)) <
      0) // ? why use copyout instead of writing directly
    return -1;
  return 0;
}
// #endif

uint64 sys_kill(void) {
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64 sys_uptime(void) {
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// // 1. pte if pte_a set set abits and clear
// uint64 sys_pgaccess(void) {

// }
