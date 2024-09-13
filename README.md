# Enumerator

**Enumerator** is a C program I developed from scratch to make my daily work easier. I chose C programming because:

1. It is faster.
2. Dependency management is easier.

I designed the program to minimize dependencies on external libraries as much as possible. This is the beginning of the project, and I will maintain it, including fixing any bugs that arise.

## Project Structure

- **extra/**: Contains code related to additional functionality and implementation of arguments like `-p` and `-H`.
- **net/**: Contains code related to network information.
- **main.c**: Contains the main interface of the program.
- **config.sh**: This script configures the project for compilation. It is similar to `autoconf` but not as extensive. It configures `CFLAGS` and `LDFLAGS`, writes them to `config.mk`, and includes `config.mk` in the `Makefile`. The script also searches for needed libraries and header files. If a required library or header file is not found, it will be disabled, which in turn disables the corresponding code to make the program more flexible and avoid compiler or linker errors.

While the program is not strictly dependent on any library, downloading the necessary header files might be beneficial for additional functionality and to enable disabled features.

## Compilation Guidelines   

1. **Configure the Project**:
   Before compiling, configure the project using:
   ```sh
   make checkdep
   It is better if you use the script directly for parsing arguments
2. **Compile it**
    make 
3. **Install optional** 
   it will be installed in /home/[user]/.local/bin but you can copy the binary directly and put where ever you want