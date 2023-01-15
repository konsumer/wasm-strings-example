// this will load a wasm file and test a few things with it, similar to chost

import assert from 'assert'
import { readFile } from 'fs/promises'
import { str2ab, ab2str } from './strings.js'

if (process.argv.length !== 3) {
  console.error('Usage: test WASMFILE.wasm')
  process.exit(1)
}

let stringBuffer

const wasmBuffer = await readFile(process.argv[2])
const wasmModule = await WebAssembly.instantiate(wasmBuffer, {
  env: {
    // host function to say hello, whcih will be exposed to wasm
    hello: n => {
      const name = ab2str(__liftBuffer(n))
      return __lowerBuffer(str2ab(`Hello ${name}`))
    },

    null0_log: m => {
      console.log('Log from WASM: ', ab2str(__liftBuffer(m)))
    },

    // not dealing with WTF16 strings here, this is just a stub to stop AS from complaining
    abort: (message, fileName, lineNumber, columnNumber) => {
      console.log('abort', lineNumber, columnNumber)
    },

    set_buffer (b) {
      stringBuffer = __liftBuffer(b)
    }
  }
})

const { memory, add, __new, stringinout_utf8, stringinout_utf8_callbackret, host_entry } = wasmModule.instance.exports

// these came from the generated wrapper
// In C, these are mostly handled automatically by the macros, but I will probly need to work out __new

function __liftBuffer (pointer) {
  if (!pointer) return null
  return memory.buffer.slice(pointer, pointer + new Uint32Array(memory.buffer)[pointer - 4 >>> 2])
}

function __lowerBuffer (value) {
  if (value == null) return 0
  const pointer = __new(value.byteLength, 1) >>> 0
  new Uint8Array(memory.buffer).set(new Uint8Array(value), pointer)
  return pointer
}

// tests start here

// simple return test
assert.strictEqual(add(1, 2), 3)

// jenky way to return strings
stringinout_utf8_callbackret(__lowerBuffer(str2ab('World')))
assert.strictEqual(ab2str(stringBuffer), 'Hello World')

// more sensible way to return strings, but can't get it working in C-host
const greet = stringinout_utf8(__lowerBuffer(str2ab('World')))
assert.strictEqual(ab2str(__liftBuffer(greet)), 'Hello World')

// trigger WASM to call into host
host_entry()

console.log('ok')
