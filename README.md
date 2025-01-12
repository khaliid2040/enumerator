# what is this?
This an open source c program i completely written from scrach to get information about running a linux operating system.
This program was for learning and educational purpose until i decided to completely write some useful thing for community.
I spent time searching for projects to start with so i found out somethings that i didn't like about tools like neofetch
and screenfetch, the issue with those tools are their information is often minimal non technical for broader community with
different technical skills, in order to get information that is more technical you need to use many commands with their own 
man pages and help and spend time reading for their man page and go online seek for help or even worst download a bunch of tools most of them with their own library apis so i thought may be we need more optimal way to fetch system information.
This tool is first i set a fundamental robust principles to work optimal and retrieve occurate information. This tool takes advantage of some libraries if they are present it uses libraries and api with your favourate tool and avoids running the binary behind the scense so it maintains the performance and try to be independent from other tools.

## This tool vs neofetch/screenfetch
First as time writing this readme neofetch discontinued and screenfetch has the critisim i critisized with it so my program will deliver technical high information the information will be more and may be in the future supported a pager.

## Target audience
- System administrators: who tired of running a command for single hardware information my tool did it for you as a user friendly by taking advantage of there stable apis if present or a custom implemenation with varienty of success.
- ethuasists : who enjoy using custom tools this project is welcome for every contributor 

## Supported Architectures
Yeah! that is true in order to take advantage from performance and make program minimalistic all code that retrieves information about cpu became architecture specific due to taking advantage of specific instructions and compiler instrincts.
In the future if used for other architectures then every supported architecture will be written by seperate code.
Currently only x86 and x86_64 are supported. Other architectures there is a fallback code that is using architecture independent way however it is buggy and have undefined behaviour not throughly tested

## Supported Linux distributions
Currently when it comes to using package manager for like checking if package present and/or counting packages present on the system this functionality is supported for all distributions that use apt,dnf,pacman and emerge as a package manager are supported for this task, however please report a bug if you observe unexpected behaviour.
Other than all distributions are supported.

## Compilation and usage guidelines
First of all currently since this project didn't get ported with distributions you need to compile it, unfortunately.
However about dependency nothing at all only make,gcc and ldconfig used by `config.sh`. However you need to run `config.sh`,
This script will handle everything after it's is done it will produce `CFLAGS` and `LDFLAGS` written in a file `config.mk` consumed by gcc, btw be caution you must know what you are doing if you decide to directly modify that file. The file `config.mk` will check libraries and headers require if they are present and act accordingly by disabling the code that depend on the missing library.

### Step one:
`make checkdep` or manually `./config.sh`, also reminder you can the color you prefer or the theme by specifying 
the color as an argument like `./config.sh [color]`
### Step two:
compile `make` optionally install `make install` copying resulted binary to `~/.local/bin`
### Step three
Done that is :(