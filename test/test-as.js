// This test will only load AS WASM (since it uses the JS wrapper AS makes)

import assert from 'assert'
import { add, stringinout, stringinout_utf8, stringinout_utf8_callbackret, host_entry } from '../build/aswasm.js'
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

// simple return test
assert.strictEqual(add(1, 2), 3)

// jenky way to return strings
stringinout_utf8_callbackret(str2ab('World'))
assert.strictEqual(ab2str(buffer), 'Hello World')

// more sensible way to return strings, but can't get it working in C-host
const greet = stringinout_utf8(str2ab('World'))
assert.strictEqual(ab2str(greet), 'Hello World')

// trigger WASM to call into host
host_entry()

// this is just to compare, with regular AS strings
assert.strictEqual(stringinout('World'), 'Hello World')

console.log('ok')
