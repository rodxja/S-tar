S-tar
Simple tar

C program that ...


## Compilation

to compile:
```
    gcc -o <command_name> main.c tableFile.c file.c
```

to run:
```
    <command_name> <options> <output_file> <file_1> ... <file_n>
```
- `<options>` 
- `<output_file>` 
- `<file_n>`

## Options
- -c : creates a new .star file
    - <command_name> -c <output_file>

- -v : verbose option, there are 4 log levels, and they are specified by the number of 'v's
    - 0 : log basic info
    - 1 : log errors
    - 2 : log extra info
    - 3 : log debug 
    - <command_name> -cvvv <output_file>

- -f : given the \<file_1\> ... \<file_n\> it will store then into the .star created
    - it must be execute with -c
    - <command_name> -cfvvv <output_file> <file_1> ... <file_n>
    - validates:
        - at least one file

- -x : it will extract all files from <output_file>
    - <command_name> -x <output_file>
    - <command_name> -x <output_file> <file_1> ... <file_n>, pending to implement
        - at least one file

- -t : list all files in <output_file>
    - <command_name> -t <output_file>

- -d : deletes the files specified 
    - <command_name> -d <output_file> <file_1> ... <file_n>
    - at least one file

- -u : updates the files specified
    - <command_name> -u <output_file> <file_1> ... <file_n>

- -r : appends the files specified
    - <command_name> -a <output_file> <file_1> ... <file_n>

- -p : remove fragmentation of block into <output_file>
    - <command_name> -p <output_file>

## Cases

1. ./star -c output.star
2. ./star -cf output.star test.txt test2.txt
3. ./star -x output.star

*Note* : all cases can implement verbose option, v's must be at the beginning
- ./star -vvcf output.star test.txt test2.txt
- ./star -vx output.star

e.g.:
```
    gcc -o star main.c tableFile.c file.c verbose.c
    ./star -c output.star
```

## TODO

- implement -x with <file_1> .. <file_n> in order to extract specfic files
- validate that -c and -x are not run at the same command
- validat that -f and -x are not run at the same command