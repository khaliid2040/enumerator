# enumerator
The program called enumerator is a c program i developed from scratch i intended it to make my daily work easir. I chose c programming because first it is faster and second 
it's dependency management is easier i amde the program to become dependent from external libraries as much as possible. This is the beginning of the project and i will 
maintain it. if it becomes like fixing bugs.

PROJECT STRUCTURE 
extra/: It contains code related to any functionality and also code in arguments like -p and -H is implemented 
net/: It contains network information code 
main.c: as every c program the main interface.
config.sh: This is the main script that configures project for compiling. It is not big script like configure when using autoconf but it designed like autoconf the script will configre the CFLAGS and LDFLAGS and put them into config.mk and then the config.mk is included in the makefile. Another important responsibility of the script is to search for needed libraries and header files so if either shared library or header file didn't be found then it will probably disabled as in turn disabling all the code depend on libary to make the program more flexible and avoing any compiler linker errors that may arise. 

The program is not strictly depend on any library but for more functionality and using disabled features then it may be good idea to download those header files.

COMPILATION GUIDELINE
1. Use make but before compiling configure the project using:
    make checkdep 

NOTICE: if you want to configure with option please run the script manually to avoid issues ./config.sh is enough
you can choose the color you want as theme to print output currently my program supports green,red,yellow and meganta
ypu can choose your favorite color you want like this:
./config.sh --color [color] 
and it will configured accordingly.
2. Then compile
    make

3. If you want to install (OPTIONAL):
    make install

4. Cleaning up
    make clean
