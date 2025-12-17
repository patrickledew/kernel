# How stdin will work

Solution
- The ProcessControlBlock struct will have a pointer to a buffer that holds the "stdin" file.
    - The buffer will have fixed size, but will be a ring buffer, so it can be filled and emptied as needed.
    - The buffer will be read starting from pcb->fd_table[i].position, and will be written to by the keyboard driver.
- The shell will read from this buffer, and write to the terminal.
- The shell will have a loop that reads from the buffer, and processes the input.
    - It will handle special characters like backspace, enter, etc. (These will be represented as special ascii characters in the buffer).
    - It will also handle command history, and allow the user to navigate through it.