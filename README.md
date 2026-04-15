warning: ultra vibe coded
warning: ultra vibe coded
warning: ultra vibe coded

# TUI Selector

A lightweight ncurses-based command-line tool to interactively filter and select lines from standard input. It supports selecting a specific word from the chosen line.

## Compilation

Requires `ncurses` development libraries.

```bash
gcc -o mxws mxws.c -lncurses
```

Fedora for example needs the ncurses devel package
```bash
sudo dnf install ncurses-devel
sudo gcc -lncurses mxws.c -o /usr/bin/mxws
```

## Usage

```bash
<command> | ./mxws [OPTIONS] [INITIAL_SEARCH]
```

### Options
* `--word` : After selecting a line, parse it and open a second selection menu to choose a specific word from that line.
* `-d <delimiter>` : Set a custom delimiter for word splitting (default is space).

Any remaining arguments are treated as the initial search string but are optional. You can type in the filter during the interactive wizard as well

## Use case

### Selecting an ip addr on mac

Filter `ifconfig` output for "192" and then select the specific IP address from the chosen line:

```bash
ifconfig | ./mxws --word 192
```

1. The program first displays lines containing `192`.
2. Press `Enter` to select the desired network interface line.
3. The program splits the selected line into words.
4. Select the specific IP address and press `Enter` to output it to stdout.


### SCP a file with selection from your desktop to a server
```bash
scp $(ls -1t ~/Downloads | mxws) somehuman@192.168.0.2:/home/somehuman/folder
```

ls -1t lists the files in the Downloads section with the newest one on top

### Moving files

```bash
mv  $(find ~/Downloads | mxws)  $(ls -1d "$PWD"/* | mxws)
```

### Openshift OC 
#### Get route url via curl
```bash
export BASEDOMAIN=.example.com
oc get route -o yaml | ./mxws --word $BASEDOMAIN  --print 'curl http://{}' | sh
```
#### Select a cluster role
```bash
oc get clusterrole -o name | ./mxws 
```



## License


MIT License

It breaks you, you can keep all the pieces

Copyright (c) 2026

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

