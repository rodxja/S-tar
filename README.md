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
- -v : verbose option, with 1 v will include log for erros, with 2 v (vv) will log more info
- -f : given the \<file_1\> ... \<file_n\> it will store then into the .star created
- -x : it will extract all files from <output_file>

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