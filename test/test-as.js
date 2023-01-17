// This test will only load AS WASM (since it uses the JS wrapper AS makes)

/* eslint-disable camelcase */

import assert from 'assert'
import { add, say_hello, test_string_retstring, test_string_param } from '../build/aswasm.js'
import { str2ab, ab2str } from './strings.js'

// host function to say hello, whcih will be exposed to wasm
globalThis.test_string_get = () => {
  console.log('test_string_get was called.')
  return str2ab('Hello from AS/JS host')
}

globalThis.null0_log = m => {
  console.log('Log from WASM: ', ab2str(m))
}

// simple return test
const value = add(1, 2)
assert.strictEqual(value, 3)
console.log(`add worked: ${value}`)

// trigger WASM to call into host
say_hello()

//  WASM -> HOST via return
const v = ab2str(test_string_retstring())
assert.strictEqual(v, 'Hi, from WASM')
console.log('test_string_retstring:', v)

// HOST -> WASM via param
test_string_param(str2ab('JS Param'))

console.log('ok')
