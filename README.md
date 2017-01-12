# 6502
Simple 6502 emulator.

# Usage
In one of your c/cpp files you need to define VM_IMPLEMENTATION before you include 6502.h. This will 
generate the actual implementation. Otherwise only the header is included.

```c
#define VM_IMPLEMENTATION
#include "6502.h"

```

# API

```c
vm_context* vm_create();
```
Creates the internal vm_context. You need to call it once to initialize the virtual machine.

```c
void vm_release();
```
Destroys the internal vm_context. Make sure to call it at the end of your program.

```c
bool vm_load(const char* fileName);
```


```c	
void vm_save(const char* fileName);
```


```c
int vm_assemble_file(const char* fileName);
```
Loads a text file containing some code and run the assembler. The generated byte code is located at 0x600 in memory.


```c
void vm_disassemble();
```

```c
int vm_assemble(const char* code);
```

```c
void vm_dump(int pc, int num);
```

```c
void vm_dump_registers();
```

```c
void vm_memory_dump(int pc, int num);
```


```c
void vm_run();
```
Will run the byte code at location 0x600. Make sure that you have either loaded or assembled your code before running it.

```c
bool vm_step();
```
# Examples

# Status

This project is an alpha version. There might be changes to the API itself. 

# Changelist

No intial release so far

# License

The project is released under the MIT license

