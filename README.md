# Syscord

> The *goal* of this project is to record syscall

[Sysdig](https://github.com/draios/sysdig) is an awesome tool for capturing syscalls; however, it is too complex and introudces a relatively high overhead.

In our test (the same evluation environment as **Performance Evaluation**), the average overhead of Sysdig could reach up to **15%**!!!

`Syscord` aims to provide limited features to minimize dependencies and overhead

> WARNING: Syscord is still a developing project and all functions have not been fully tested.

## User Manual 

Basically, you can `make` to compile the driver and then `insmod syscord.ko` to install the kernel module. 

Then you could open file `/etc/syscall-record/record` to find all syscall logs. (You might need to create the directory and delete the file before run)

```bash
pid=22569, unknown, res=1
pid=22569, accept4, res=15
pid=22569, epoll_ctl, res=0
pid=22569, epoll_wait, res=1
pid=22569, accept4, res=16
pid=22569, epoll_ctl, res=0
pid=22569, epoll_wait, res=1
pid=22569, accept4, res=17
pid=22569, epoll_ctl, res=0
pid=22569, epoll_wait, res=1
pid=22569, accept4, res=18
pid=22569, epoll_ctl, res=0
```

### Filter condition

- `pid`: `insmod syscord.ko pid=1234`
- `proc_name`: `insmod syscord.ko proc_name=test_syscall`
- `ppid`: the parent pid
- `parent_proc_name`: the process name of parent

All filter conditions can set at the same time

Note: because of the constraint of Linux, we will only compare the first 15 characters of `proc_name`

#### Record the content (e.g., `read`)

sycalls like `size_t read(int fd, void *buf, size_t count);` might fill the buffer with some (kilo)bytes. Directly recording will inevitably introduce a considerable overhead (~13%), along with a huge record file...

By default, we defined a marco named `TRUNCATE_CONTENT_RECORD` at `syscalls/handlers.h`, which means it will only save the first `256` bytes for syscalls like `read`.

## Handlers for different type of syscalls

handler function are defined in the end of `syscalls/handlers.h`. To regitser it to let `Syscord` know, relavant codes are defined in `syscalls/handlers-table.h`

For example `openat_handle` to handle the syscall `openat` 

```c
int openat_handle(struct handler_args* _handler_args) {
  // dirfd = -100 represents AT_FDCWD (the current work directory)
  const int dirfd = _handler_args->saved_entry->arg0;
  const void* pathname = (void*)_handler_args->saved_entry->arg1;
  const int flags = get_arg2(_handler_args->regs);
  const mode_t mode = get_arg3(_handler_args->regs);
  char buf_tmp[ARGS_BUF_SIZE + 1];

  assemble_buf_arg(buf_tmp, pathname, ARGS_BUF_SIZE);
  fast_sprintf(_handler_args->small_buf,
          "pid=%d, openat, dir=%d, path=%s, flags=%d, mode=%u, "
          "res=%d\n",
          current->pid, dirfd, buf_tmp, flags, mode, (int)_handler_args->ret);
  return 0;
}
```

There are two tables in `handlers-table.h` are corresponded to register these handlers. The table also shows all types of syscalls are special treated.

```c
typedef int (*handler_callback)(struct handler_args*);
handler_callback functions[] = {
    &getuid_handle, &recvfrom_handle, &socket_handle, &fstat_handle,
    &getcwd_handle, &lseek_handle,    &futex_handle,  &sendto_handle,
    &clone_handle,  &read_handle,     &mmap_handle,   &exit_group_handle,
    &close_handle,  &tgkill_handle,   &munmap_handle, &nanosleep_handle,
    &ppoll_handle,  &dup_handle,      &ioctl_handle,  &open_handle,
    &creat_handle,  &openat_handle};

char handler_string[][32] = {
    "getuid", "recvfrom", "socket", "fstat",     "getcwd", "lseek",
    "futex",  "sendto",   "clone",  "read",      "mmap",   "exit_group",
    "close",  "tgkill",   "munmap", "nanosleep", "ppoll",  "dup",
    "ioctl",  "open",     "creat",  "openat"};
```


## Performance Evaluation

tl;dr: the overhead is extremely low (~1%)

### Test settings: 
- nginx web server (with a complex html page), 
- Juno r2 board, with 2 * Cortex-A72 and 4 * Cortex-A53 
- test by ` ab -n 500000 -c 100 http://localhost/`, 50 times

### Test result

|             | **baseline** | **Default**      |
| ----------- | ------------ | ---------------- |
| **1**       | 144.139      | 137.249          |
| **2**       | 133.001      | 135.170          |
| ...      | ...     | ...         |
| **46**      | 139.766      | 142.664          |
| **47**      | 141.629      | 144.294          |
| **48**      | 132.655      | 139.153          |
| **49**      | 141.153      | 156.555          |
| **50**      | 138.816      | 140.288          |
| **avg**     | 139.21458    | 140.5195         |
| **ratio** |              | 1.00937344349996 |

### What's the cost?

A process could issue many times of syscalls with a second, so the trace file can be very large..

The size of the recod file generated by this test is 2.16G (almost 1G/h)


## TO-DO List

- [ ] More syscalls..