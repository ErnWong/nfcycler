# nfcycler

Pass in a shell command, and it will help you execute it, and pipe its stdout back into stdin.

## Compiling

```sh
make
```

And for example:

```sh
make examples/sequence
./bin/nfcycler "examples/sequence"
```

Note: quotation marks needed if you have pipes and shell-related symbols in your shell command argument.
