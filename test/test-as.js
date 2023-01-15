// This test will only load AS WASM (since it uses the JS wrapper AS makes)

import assert from 'assert'
import { add, stringinout, stringinout_utf8, host_entry } from '../build/aswasm.js'
import { str2ab, ab2str } from './strings.js'

let buffer
globalThis.set_buffer = b => {
  buffer = b
}

// host function to say hello, whcih will be exposed to wasm
globalThis.hello = n => {
  const name = ab2str(n)
  return str2ab(`Hello ${name}`)
}

globalThis.null0_log = m => {
  console.log('Log from WASM: ', ab2str(m))
}

assert.strictEqual(add(1, 2), 3)

assert.strictEqual(stringinout('World'), 'Hello World')

const name = str2ab('World')
stringinout_utf8(name)
assert.strictEqual(ab2str(buffer), 'Hello World')

// this will trigger the wasm to call env.hello
host_entry()

console.log('ok')
