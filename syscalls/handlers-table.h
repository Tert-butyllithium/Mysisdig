#ifndef SYSCORD_HANDLERS_TABLE_H
#define SYSCORD_HANDLERS_TABLE_H
#include "handlers.h"
extern char syscall_id_to_name[][32];

typedef int (*handler_callback)(struct handler_args*);
handler_callback functions[] = {
    &getuid_handle, &recvfrom_handle, &socket_handle, &fstat_handle,
    &getcwd_handle, &lseek_handle,    &futex_handle,  &sendto_handle,
    &clone_handle,  &read_handle,     &mmap_handle,   &exit_group_handle,
    &close_handle,  &tgkill_handle,   &munmap_handle, &nanosleep_handle,
    &ppoll_handle,  &dup_handle,      &ioctl_handle,  &open_handle,
    &creat_handle,  &openat_handle, &close_handle, &writev_handle};

char handler_string[][32] = {
    "getuid", "recvfrom", "socket", "fstat",     "getcwd", "lseek",
    "futex",  "sendto",   "clone",  "read",      "mmap",   "exit_group",
    "close",  "tgkill",   "munmap", "nanosleep", "ppoll",  "dup",
    "ioctl",  "open",     "creat",  "openat", "close", "writev"};

handler_callback syscall_id_handlers[512];

void init_syscall_id_handlers(void) {
  int i, j;
  const static size_t SYSCALL_TABLE_SIZE =
      sizeof(syscall_id_to_name) / sizeof(syscall_id_to_name[0]);
  const static size_t HANDLER_TABLE_SIZE =
      sizeof(handler_string) / sizeof(handler_string[0]);
  printk("[syscord:] These following syscalls have been special handled");
  for (i = 0; i < SYSCALL_TABLE_SIZE; i++) {
    for (j = 0; j < HANDLER_TABLE_SIZE; j++) {
      if (strcmp(syscall_id_to_name[i], handler_string[j]) == 0) {
        syscall_id_handlers[i] = functions[j];
        printk("[syscord:] %d:%s - %pF", i, syscall_id_to_name[i],
               functions[j]);
        goto next_label;
      }
    }
    syscall_id_handlers[i] = &default_handle;
  next_label:;
  }
}

#endif