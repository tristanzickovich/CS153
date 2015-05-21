#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "userprog/process.h"
#include "devices/shutdown.h"

struct lock file_lock;

static void syscall_handler (struct intr_frame *);

static void copy_in (void *dst_, const void *usrc_, size_t size); 
static inline bool get_user (uint8_t *dst, const uint8_t *usrc);
static bool verify_user (const void *uaddr);

static void
halt (void)
{
  shutdown_power_off();
}

static void 
exit (int status)
{
  
  thread_current()->return_status = status;
  thread_current()->exit = true;
  thread_exit ();
}

static int 
exec (const char *cmd_line);

static int 
wait (int pid)
{
  process_wait(pid);
}

static bool 
create (const char *file, unsigned initial_size)
{
  lock_acquire(&file_lock);
  bool ret = filesys_create(file, initial_size);
  lock_release(&file_lock);
  return ret;
}

static bool 
remove (const char *file)
{
  lock_acquire(&file_lock);
  bool ret = filesys_remove(file);
  lock_release(&file_lock);
  return ret;
}
static int 
open (const char *file);

static int 
filesize (int fd);

static int 
read (int fd, void *buffer, unsigned size);

static int 
write (int fd, const void *buffer, unsigned size)
{
  if (fd == 1)
  {
    putbuf (buffer, size);
    return size;
  }
  return size;
}

static void 
seek (int fd, unsigned position);

static unsigned 
tell (int fd);

static void 
close (int fd);

void
syscall_init (void) 
{
  lock_init(&file_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  
  if (!verify_user (f->esp)) 
  {
    exit(-1);
  }
  unsigned callNum;
  int args[3];

  

  copy_in (&callNum, f->esp, sizeof callNum);
  
  
  switch(callNum)
  {
    case SYS_HALT: /* Halt the operating system. */
      halt();
      break;
    case SYS_EXIT: /* Terminate this process. */
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 1);
      exit(args[0]);
      break;
    case SYS_EXEC: /* Start another process. */
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 1);
      break;
    case SYS_WAIT: /* Wait for a child process to die. */
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 1);
      wait(args[0]);
      break;
    case SYS_CREATE: /* Create a file. */
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 2);
      create(args[0], args[1]);
      break;
    case SYS_REMOVE: /* Delete a file. */
      remove(args[0]);
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 1);
      break;
    case SYS_OPEN: /* Open a file. */
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 1);
      break;
    case SYS_FILESIZE: /* Obtain a file's size. */
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 1);
      break;
    case SYS_READ: /* Read from a file. */
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 3);
      break;
    case SYS_WRITE: /* Write to a file. */
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 3);
      f->eax = write(args[0],args[1],args[2]);
      break;
    case SYS_SEEK: /* Change position in a file. */
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 2);
      break;
    case SYS_TELL: /* Report current position in a file. */
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 1);
      break;
    case SYS_CLOSE: /* Close a file. */
      copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * 1);
      break;  
    thread_exit();
  }
}

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
    {
      exit(-1);
      thread_exit ();
    }
}

/* Copies a byte from user address USRC to kernel address DST.
   USRC must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static inline bool
get_user (uint8_t *dst, const uint8_t *usrc)
{
  int eax;
  asm ("movl $1f, %%eax; movb %2, %%al; movb %%al, %0; 1:"
       : "=m" (*dst), "=&a" (eax) : "m" (*usrc));
  return eax != 0;
}

/* Returns true if UADDR is a valid, mapped user address,
   false otherwise. */
static bool
verify_user (const void *uaddr) 
{
  return (uaddr < PHYS_BASE
          && pagedir_get_page (thread_current ()->pagedir, uaddr) != NULL);
}
