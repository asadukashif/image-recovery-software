# Image Recovery Software
A software written in C that allows users to recover PNGs, JPEGS, and GIFs from their deleted drives, or any provided hexdumps

<br />

## Build
### Prerequisites 
You need to have the following installed
- [GNU Make](http://gnuwin32.sourceforge.net/packages/make.htm)
- [MinGW C Compiler](https://sourceforge.net/projects/mingw-w64/)
- [git](https://git-scm.com/downloads)
 
### Steps
Clones the repository
```bash 
git clone https://github.com/ShaderOX/image-recovery-software.git && cd image-recovery-software
```

Compiles the source code
```bash
make -C src/
```

Runs the executable
```bash
./dist/recover.exe --help
```
and it gives the following output <br />
`Usage: ./recover.exe --filename <filename, usb.dmp> | --drive <drive, C:> --buffer <buffer_size, >=512> (optional)`

<br />

## Working
The application is able to recover deleted files by tracking their headers and trailers. All files have certain types of headers and trailers (in hex) that is unique to their type and by reading between those headers and trailers, the file can be recovered. This is true not only for images but is the basis for all the recovery softwares.

<br />

## Licence
[LICENSE - MIT](./LICENSE)
