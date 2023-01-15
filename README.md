This is a small example of passing strings back and forth over WASM, in a wasm3/js host. Since I need to support multiple wasm languages, the strings should be in a single format, and here I am exploring some differnt options.

## requirements

Before you get started, you will need some tools installed on your system.

- cmake
- llvm
- node/npm


On mac, with homebrew, I installed them like this:

```sh
brew install cmake node llvm
brew link --force llvm
```

Now, in the project-folder, run `npm i`


## WASM Shape

The basic shape of the functions exported and tested in both runtimes looks like this:

## imports

```ts
// a simple string-logging function, so WASM can talk about what's going on
null0_log(text: ArrayBuffer): void

// host will define this, so it can check return value
hello_host(url: ArrayBuffer): ArrayBuffer
```

### exports

```ts
// entry-point for the WASM-host to start things
host_entry():void {}

// UTF8, null-terminated strings in and out
stringinout_utf8(n: ArrayBuffer) : ArrayBuffer {}

// simple non-string tester to ensure host-linkage is working right
add(a: i32, b: i32): i32 {}
```

## chost

You can build this once, and use it for the all the WASM files to test C-host.

```sh
# build c host tester: you will need cmake/make
npm run build:chost
```

## Assemblyscript WASM

Included is a simple example assemblyscript-made WASM that illustrates a couple ways to pass strings.

```sh
# build the assemblyscript-based wasm
npm run build:aswasm

# run JS-host tests (using JS wrapper generated by assemblyscript)
npm run test:asjs

# run JS-host tests (using non-wrapped WASM)
npm run test:js build/aswasm.wasm

# run the wasm in C-host
npm run test:c build/aswasm.wasm
```

## C WASM

This is essentiually the same WASM, written in C.

```sh
# build the assemblyscript-based wasm
npm run build:cwasm

# run JS-host tests (using nopn-wrapped WASM)
npm run test:js build/cwasm.wasm

# run the wasm in C-host
npm run test:c build/cwasm.wasm
```