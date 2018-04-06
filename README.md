# linux-shell-c
fork-per-command Linux Shell written in C

# Supported Operations:
_Redirections:_ The shell supports > < >> redirections.  
_Piping:_ The shell supports pipelining any number of commands.  
cat x| grep pat|uniq|sort  
in addition, 2-level and 3-level piping are also supported using comma (',') as delimeter  
ls -l || grep ^-, grep ^d  

# Usage: 
$ make all  
$ ./shell

This program was written as an assignment of ISF462 Network Programming, BITS Pilani.
