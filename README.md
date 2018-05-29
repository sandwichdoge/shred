# shred
Securely scramble file without truncating it

Compile:
```
gcc -o <prog_name> shre.c stringops.c
```

Usage:
```
<prog_name> <file/directory_path> [-r] [-d]
```

# note
Will not work on SSD. Disk space wipe is required for secure erasure, even then some residual data will still remain!
