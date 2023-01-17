// this will load a wasm file and test a few things with it, similar to chost

import assert from 'assert'
import { readFile } from 'fs/promises'
import { str2ab, ab2str } from './strings.js'

if (process.argv.length !== 3) {
  console.error('Usage: test WASMFILE.wasm')
  process.exit(1)
}
const wasmFile = process.argv[2]

let stringBuffer

const wasmBuffer = await readFile(wasmFile)
const wasmModule = await WebAssembly.instantiate(wasmBuffer, {
  env: {
    // host function to say hello, whcih will be exposed to wasm
    test_string_get: () => {
      return lowerBuffer(str2ab('Hello from JS'))
    },

    null0_log: m => {
      console.log('Log from WASM: ', ab2str(liftBuffer(m)))
    },

    // not dealing with WTF16 strings here, this is just a stub to stop AS from complaining
    abort: (message, fileName, lineNumber, columnNumber) => {
      console.log('abort', lineNumber, columnNumber)
    }
  }
})

const { memory, add, wmalloc, say_hello } = wasmModule.instance.exports

// these loosely came from the generated wrapper
// In C, these are mostly handled automatically by the macros, but I will need to work out wmalloc

function liftBuffer (pointer) {
  if (!pointer) return null
  return memory.buffer.slice(pointer, pointer + new Uint32Array(memory.buffer)[pointer - 4 >>> 2])
}

function lowerBuffer (value) {
  if (value == null) return 0
  const pointer = wmalloc(value.byteLength) >>> 0
  new Uint8Array(memory.buffer).set(new Uint8Array(value), pointer)
  return pointer
}

// tests start here

// simple return test
const value = add(1, 2)
assert.strictEqual(value, 3)
console.log(`add worked: ${value}`)

// trigger WASM to call into host
say_hello()

console.log('ok')
