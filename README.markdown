# nfcycler

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
    -i, --init [command]          supply a payload initialisation command
```

## Compiling

```sh
make
```

And for example:

```sh
make examples
./nfcycler "examples/sequence"
```

Note: quotation marks needed if you have pipes and shell-related symbols in your shell command argument.
