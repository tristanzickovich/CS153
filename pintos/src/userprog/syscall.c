#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  unsigned callNum;
  int args[3];
  int numOfArgs;

  copy_in(&callNum, f->esp, sizeof callNum);

  switch (callNum)
  {
    case SYS_HALT: /* Halt the operating system. */
      //halt();
      break;
    case SYS_EXIT: /* Terminate this process. */
      get_args(args, f, 1);
      break;
    case SYS_EXEC: /* Start another process. */
      get_args(args, f, 1);
      break;
    case SYS_WAIT: /* Wait for a child process to die. */
      get_args(args, f, 1);
      break;
    case SYS_CREATE: /* Create a file. */
      get_args(args, f, 2);
      break;
    case SYS_REMOVE: /* Delete a file. */
      get_args(args, f, 1);
      break;
    case SYS_OPEN: /* Open a file. */
      get_args(args, f, 1);
      break;
    case SYS_FILESIZE: /* Obtain a file's size. */
      get_args(args, f, 1);
      break;
    case SYS_READ: /* Read from a file. */
      get_args(args, f, 3);
      break;
    case SYS_WRITE: /* Write to a file. */
      get_args(args, f, 3);
      break;
    case SYS_SEEK: /* Change position in a file. */
      get_args(args, f, 2);
      break;
    case SYS_TELL: /* Report current position in a file. */
      get_args(args, f, 1);
      break;
    case SYS_CLOSE: /* Close a file. */
      get_args(args, f, 1);
      break;  
  }
  thread_exit ();
}

static void
get_args(int *args, struct intr_frame *f, int numArgs)
{
  int *ptr;
  for (int i = 1; i <= numArgs; i++)
  {
    ptr = f->esp + i;
    args[i] = *ptr; 
  }
}
