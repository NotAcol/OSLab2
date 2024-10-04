(export I_NEED_TECH_HINTS=1) 

- Create .hello_there (touch .hello_there)
- Remove perms for anything except user read for .hello_there (chmod 400 .hello_there)
- Need to send a SIGCONT in 10 secs otherwise it aborts (pkill -CONT "riddle")
- Need to have global variable ANSWER=42 (export ANSWER=42)
- Need to make a fifo named "magic_mirror" (mkfifo magic_mirror)
- Made an executable opened a random file and dup2(fd,99) then called riddle from within the executab   le
