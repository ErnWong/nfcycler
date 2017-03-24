# nfcycler

[![Build Status](https://travis-ci.org/ErnWong/nfcycler.svg?branch=master)](https://travis-ci.org/ErnWong/nfcycler)

Pass in a shell command, and it will help you execute it, and pipe its stdout back into stdin.

## Usage

```
  Usage: nfcycler [options] <shell command>

  Options:

    -V, --version                 output program version
    -h, --help                    output help information
    -u, --usage                   give a short usage message
    -v, --verbose                 be crazy and log everything
    -q, --quiet                   suppress informative logs
    -p, --print-payload           log the pipe's value real time
```

For example, to set up a robot simulation, `nfcycler` lets you do this:

```sh
nfcycler 'get-joystick | robot-emulator | robot-simulator | robot-renderer'
```

**Note:** *get-joystick, robot-emulator, and the rest are just placeholder names, examples of some possible programs you can use.*

To initialise the pipeline data, `nfcycler` lets you print the initial value out to stdout like this:

```sh
nfcycler 'echo 1 && increment-o-matic'
```

This makes a typical simulation pipeline look like this:

```sh
nfcycler '
    init &&
    get-joystick |
    robot-emulator |
    robot-simulator |
    robot-renderer
'
```

Not a fan of robots? *Non forsit!* `nfcycler` is designed to work with any kind of program that supports standard input and output.

**Caution 1.** Make sure you flush your stdout buffers, or disable them, in your programs as they can lock up your pipeline.

**Caution 2.** If you're writing complex shell script directly inside the `'` quotation marks `'`, remember to escape special shell characters.

## Compiling

```sh
make
```

To install into `/usr/local/bin`, do `make install`

To install elsewhere, such as `/your/nice/directory`, do `make install PREFIX=/your/nice/directory`

To give `nfcycler` a health checkup, do `make test`.
