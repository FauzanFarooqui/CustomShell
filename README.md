# CustomShell
A custom shell for UNIX environments in C that uses multi-threading (fork, exec, etc.). Can handle most typical commands you give to, say, a Linux terminal. 
This is a assignment project for the Operating Systems course at VNIT.

- Handles interrupts - Ctrl+Z / Ctrl+C.
- Handles multiple commands - 
  If you would like the commands to run in order i.e sequentially, seperate them by ##.
  If the order of the commands is immaterial i.e they can parallelly, seperate them by &&.
- Handles output redirection (>).
- Handles change of directory
- You can only exit the terminal program by typing "exit".
