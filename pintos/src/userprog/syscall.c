#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
static void halt (void); 
static void exit (int status);
static int exec (const char *cmd_line);
static int wait (pid_t pid);
static bool create (const char *file, unsigned initial_size);
static bool remove (const char *file);
static int open (const char *file);
static int filesize (int fd);
static int read (int fd, void *buffer, unsigned size);
static int write (int fd, const void *buffer, unsigned size);
static void seek (int fd, unsigned position);
static unsigned tell (int fd);
static void close (int fd);
static void get_args(int *args, struct intr_frame *f, int numArgs);
/* Copies SIZE bytes from user address USRC to kernel address
 *    DST.
 *       Call thread_exit() if any of the user accesses are invalid. */
static void
copy_in (void *dst_, const void *usrc_, size_t size) 
{
  uint8_t *dst = dst_;
  const uint8_t *usrc = usrc_;
 
  for (; size > 0; size--, dst++, usrc++) 
    if (usrc >= (uint8_t *) PHYS_BASE || !get_user (dst, usrc)) 
      thread_exit ();
}

/* Creates a copy of user string US in kernel memory
 *    and returns it as a page that must be freed with
 *       palloc_free_page().
 *          Truncates the string at PGSIZE bytes in size.
 *             Call thread_exit() if any of the user accesses are invalid. */
static char *
copy_in_string (const char *us) 
{
  char *ks;
  size_t length;
 
  ks = palloc_get_page (0);
  if (ks == NULL) 
    thread_exit ();
 
  for (length = 0; length < PGSIZE; length++)
    {
      if (us >= (char *) PHYS_BASE || !get_user (ks + length, us++)) 
        {
          palloc_free_page (ks);
          thread_exit (); 
        }
       
      if (ks[length] == '\0')
        return ks;
    }
  ks[PGSIZE - 1] = '\0';
  return ks;
}


/* Copies a byte from user address USRC to kernel address DST.
 *    USRC must be below PHYS_BASE.
 *       Returns true if successful, false if a segfault occurred. */
static inline bool
get_user (uint8_t *dst, const uint8_t *usrc)
{
  int eax;
  asm ("movl $1f, %%eax; movb %2, %%al; movb %%al, %0; 1:"
       : "=m" (*dst), "=&a" (eax) : "m" (*usrc));
  return eax != 0;
}




/* Returns true if UADDR is a valid, mapped user address,
 *    false otherwise. */
static bool
verify_user (const void *uaddr) 
{
  return (uaddr < PHYS_BASE
          && pagedir_get_page (thread_current ()->pagedir, uaddr) != NULL);
}

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

  //copy_in(&callNum, f->esp, sizeof callNum);

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
  int i;
  for (i = 0; i <= numArgs; i++)
  {
    ptr = f->esp + i;
    args[i] = *ptr; 
  }
}
