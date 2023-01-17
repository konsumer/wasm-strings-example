This is a small example of passing strings back and forth over WASM, in a wasm3/js host. Since I need to support multiple wasm languages, the strings should be in a single format, and here I am exploring some differnt options.

## basic use-cases

- WASM -> HOST via param (`null0_log`)
- HOST -> WASM via return (`test_string_get`)

These are not working in C, but for my use-case, that is ok:

- WASM -> HOST via return (`test_string_retstring`)
- HOST -> WASM via param (`test_string_param`)

I needd to figure out how to access the memory outside of HOST callbacks ('m3ApiOffsetToPtr' only works there.)

## requirements

Before you get started, you will need some tools installed on your system.

- cmake
- llvm
- node/npm


On mac, with homebrew, I installed them like this:

```bash
brew install cmake node llvm
brew link --force llvm
```

Now, in the project-folder, run `npm i`

## Assemblyscript WASM

Included is a simple example assemblyscript-made WASM that illustrates a couple ways to pass strings.

```bash
# run JS-host tests (using JS wrapper generated by assemblyscript)
npm run test:asjs

# run JS-host tests (using non-wrapped WASM)
npm run test:js

# run the wasm in C-host
npm run test:c
```
