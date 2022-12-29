# Linux Shell
A basic Linux shell in C that uses multi-threading in C POSIX. Can handle most typical commands you give to your default terminal. 

This is an assignment project for the Operating Systems course at VNIT.

- Handles interrupts - Ctrl+Z / Ctrl+C.
- Handles multiple commands - (Each command is run in a child process)
  If you would like the commands to run in order i.e sequentially, seperate them by ##.
  If the order of the commands is immaterial i.e they can run parallelly, seperate them by &&.
- Handles output redirection (>).
- Handles change of directory (including 'cd ..')
- You can only exit the terminal program by typing "exit".
