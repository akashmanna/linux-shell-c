# linux-shell-c
fork-per-command Linux Shell written in C\n

# Supported Operations:
_Redirections:_ The shell supports > < >> redirections.\n
_Piping:_ The shell supports pipelining any number of commands.\n 
cat x| grep pat|uniq|sort\n
in addition, 2-level and 3-level piping are also supported using comma (',') as delimeter\n
ls -l || grep ^-, grep ^d\n

# Usage: 
$ make all\n
$ ./shell
\n
This program was written as an assignment of ISF462 Network Programming, BITS Pilani.
