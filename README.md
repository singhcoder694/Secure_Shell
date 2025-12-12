# Secure Shell - XV6 Operating System Extensions

A modified version of the XV6 operating system implementing enhanced security features including user authentication, process tracking, syscall control, and file permission management.

## 🔐 Features

### 1. Username/Password Authentication System

- **Three-attempt login limit**: Users are locked out after three failed login attempts
- **Credential validation**: Implemented at system initialization level in `init.c`
- **Configurable credentials**: Username and password can be set via Makefile variables
- **Session management**: Prevents shell access until successful authentication

**Implementation Details:**

- Authentication occurs during system boot before shell initialization
- Failed attempts display remaining tries to the user
- After exhausting all attempts, the system enters an idle state preventing further access
- Credentials are compiled into the binary using preprocessor directives

**Files Modified:**

- `init.c`: Core authentication logic
- `Makefile`: USERNAME and PASSWORD configuration

### 2. Process History Tracking

- **Comprehensive logging**: Tracks all process execution with detailed metadata
- **Persistent storage**: Maintains history of up to 64 processes
- **Rich information capture**: Records PID, process name, memory usage, and start time
- **Sorted output**: Results automatically sorted by start time for chronological viewing

**System Call Added:**

```c
int gethistory(void)
```

**Information Tracked:**

- Process ID (PID)
- Process name
- Memory usage at start
- Start timestamp

**Implementation Details:**

- Process history stored in a global array `phistory[MAX_HISTORY]`
- Thread-safe access using spinlock (`phistory_lock`)
- History populated automatically on process exit
- Bubble sort algorithm used for chronological ordering

**Files Modified:**

- `proc.h`: Added `proc_history` structure and declarations
- `proc.c`: History collection logic in exit routine
- `sysproc.c`: System call implementation with sorting
- `syscall.h`, `syscall.c`, `user.h`, `usys.S`: System call registration

### 3. System Call Blocking/Unblocking

- **Granular control**: Shell can block/unblock specific system calls for child processes
- **Per-shell isolation**: Each shell instance maintains independent blocked call lists
- **Protected operations**: Fork and exit cannot be blocked to maintain system stability
- **Bitmask implementation**: Efficient storage using 32-bit bitmasks

**System Calls Added:**

```c
int block(int syscall_id)
int unblock(int syscall_id)
```

**Features:**

- Only shell processes can invoke block/unblock
- Blocked syscalls return -1 when attempted by child processes
- Supports up to 30 different system calls (MAX_SYSCALLS)
- Fork (SYS_fork=1) and Exit (SYS_exit=2) are protected from blocking

**Implementation Details:**

- `blocked_calls[MAX_SH]` array stores blocked syscalls per shell
- Bitmap representation: each bit represents a syscall ID
- Validation in `syscall.c` checks if current process's parent shell has blocked the call
- Shell command integration for interactive blocking/unblocking

**Files Modified:**

- `proc.c`: `block()` and `unblock()` functions
- `sysproc.c`: System call wrappers
- `syscall.c`: Syscall execution validation logic
- `sh.c`: Shell commands for "block" and "unblock"
- `syscall.h`, `user.h`, `usys.S`: System call registration

### 4. File Permission Management (chmod)

- **Unix-style permissions**: Implements classic 3-bit permission model (rwx)
- **Granular access control**: Separate read, write, and execute permissions
- **Enforcement at syscall level**: Permissions checked during open, read, write, and exec operations
- **Shell integration**: Interactive chmod command for easy permission modification

**System Call Added:**

```c
int chmod(const char *path, int mode)
```

**Permission Bits:**

- Bit 0 (LSB): Read permission (0b001)
- Bit 1: Write permission (0b010)
- Bit 2: Execute permission (0b100)

**Valid Modes (0-7):**

- 0: No permissions (---)
- 1: Execute only (--x)
- 2: Write only (-w-)
- 3: Write + Execute (-wx)
- 4: Read only (r--)
- 5: Read + Execute (r-x)
- 6: Read + Write (rw-)
- 7: Full permissions (rwx)

**Implementation Details:**

- Permission mode stored in inode structure
- Default mode (0b111 = 7) assigned to all new files and directories
- Permission checks enforced in:
  - `sys_open()`: Validates read/write access on file open
  - `sys_read()`: Verifies read permission
  - `sys_write()`: Verifies write permission
  - `exec()`: Checks execute permission before loading binary
- Device files (T_DEV) bypass permission checks

**Files Modified:**

- `fs.h`: Added `mode` field to `dinode` structure
- `fs.c`: Updated `ialloc()`, `ilock()`, `iupdate()` for mode handling
- `sysfile.c`: Permission enforcement in I/O operations, chmod implementation
- `exec.c`: Execute permission validation
- `sh.c`: Shell command parsing for chmod
- `syscall.h`, `syscall.c`, `user.h`, `usys.S`: System call registration

## 🛠️ Technical Architecture

### System Call Flow

```
User Program (user.h)
    ↓
USYS Assembly Stub (usys.S)
    ↓
Trap Handler (trap.c)
    ↓
System Call Dispatcher (syscall.c)
    ↓
Implementation (sysproc.c / sysfile.c / proc.c)
```

### Authentication Flow

```
System Boot
    ↓
init.c main()
    ↓
Console Setup
    ↓
Login Prompt Loop (max 3 attempts)
    ├─ Username Validation
    └─ Password Validation
        ↓
    Success → Shell Launch
    Failure → System Idle
```

### Syscall Blocking Flow

```
Shell Process: block(syscall_id)
    ↓
Set bit in blocked_calls[shell_id]
    ↓
Child Process: attempts syscall
    ↓
syscall() validates → Check parent shell's mask
    ↓
Blocked? Return -1 : Execute syscall
```

## 📦 Building and Running

### Prerequisites

- QEMU emulator
- GCC cross-compiler for i386
- Make build system

### Build

```bash
# Set credentials in Makefile (optional)
# USERNAME = "your_username"
# PASSWORD = "your_password"

# Compile the system
make

# Run in QEMU
make qemu
```

### Usage Examples

#### Login

```
xv6...
init: starting init
Enter Username: 1
Enter Password: 2
Login successful
init: starting sh
$
```

#### Process History

```
$ gethistory
1 init 4096
2 sh 8192
```

#### Block System Calls

```
$ block 11          # Block getpid
$ echo test         # Works (spawns child)
test
$ getpid           # Returns -1 in child processes
```

#### File Permissions

```
$ chmod 7 file.txt  # Full permissions (rwx)
$ chmod 4 file.txt  # Read-only (r--)
$ chmod 6 file.txt  # Read-write (rw-)
$ cat file.txt      # Requires read permission
$ echo "data" > file.txt  # Requires write permission
```

## 🔍 Key Files and Components

### Core Implementation Files

| File        | Purpose                                                |
| ----------- | ------------------------------------------------------ |
| `init.c`    | System initialization and login authentication         |
| `proc.c`    | Process management, history tracking, syscall blocking |
| `sysproc.c` | Process-related system calls                           |
| `sysfile.c` | File system calls including chmod                      |
| `syscall.c` | System call dispatcher and validation                  |
| `sh.c`      | Shell with integrated commands                         |
| `fs.c`      | File system with permission support                    |
| `exec.c`    | Program execution with permission checks               |

### Header Files

| File        | Purpose                                |
| ----------- | -------------------------------------- |
| `proc.h`    | Process and history structures         |
| `syscall.h` | System call number definitions         |
| `user.h`    | User-space system call declarations    |
| `fs.h`      | File system structures with mode field |

### Assembly Files

| File     | Purpose                     |
| -------- | --------------------------- |
| `usys.S` | System call stub generation |

## 🧪 Testing

### Authentication Testing

```bash
# Test 1: Valid credentials
Enter Username: 1
Enter Password: 2
# Expected: Login successful

# Test 2: Invalid credentials (3 attempts)
Enter Username: wrong
# Expected: Invalid username. Attempts left: 2
# (Repeat until locked out)
```

### History Tracking

```bash
$ ls
$ cat README
$ gethistory
# Expected: Sorted list of PIDs with memory usage
```

### Syscall Blocking

```bash
$ block 11      # Block getpid
$ fork
# Child process cannot call getpid
$ unblock 11    # Restore getpid
```

### Permission Testing

```bash
$ echo "test" > file.txt
$ chmod 4 file.txt    # Read-only
$ cat file.txt        # Should work
$ echo "new" > file.txt  # Should fail
$ chmod 6 file.txt    # Read-write
$ echo "new" > file.txt  # Should work
```

## 🔒 Security Considerations

### Authentication

- Credentials compiled into binary (not runtime configurable for security)
- No password recovery mechanism
- System lockout after 3 failed attempts
- No backdoor or bypass available

### Syscall Blocking

- Fork and exit are protected from blocking to prevent system deadlock
- Only shell processes can modify block lists
- Each shell maintains independent blocked call lists
- Validation occurs before syscall execution

### File Permissions

- Default mode (rwx) for new files
- Device files exempt from permission checks
- Permission checks enforced at syscall level
- Permissions persist across reboots (stored in inode)

## 📝 System Call IDs

| ID  | System Call | Blockable |
| --- | ----------- | --------- |
| 1   | fork        | ❌        |
| 2   | exit        | ❌        |
| 3   | wait        | ✅        |
| 5   | read        | ✅        |
| 11  | getpid      | ✅        |
| 16  | write       | ✅        |
| 22  | gethistory  | ✅        |
| 23  | block       | ✅        |
| 24  | unblock     | ✅        |
| 25  | chmod       | ✅        |

## 🎓 Educational Value

This project demonstrates:

- **Operating System Security**: Multi-layer security from boot to syscall execution
- **System Programming**: Low-level C programming with kernel modifications
- **Process Management**: Process lifecycle tracking and control
- **Access Control**: Fine-grained permission systems
- **System Call Design**: Adding new kernel functionality
- **Shell Integration**: Extending user interface with new commands

## 📄 License

Based on MIT's XV6 operating system. Original XV6 license applies.

## 👨‍💻 Author

This implementation extends the XV6 operating system with security and monitoring features for educational purposes.

---

**Note**: This is an educational project demonstrating operating system concepts. The security implementations are simplified and designed for learning rather than production use.
