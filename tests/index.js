import assert from 'assert'
import { add, stringinout, stringinout_utf8, host_entry } from '../build/debug.js'

// in js-host, I have to encode/decode the UTF8 string, because of how the assemblyscript wrapper is generated
function ab2str (buf) {
  return String.fromCharCode.apply(null, new Uint8Array(buf).slice(0, -1))
}

function str2ab (str) {
  const buf = new ArrayBuffer(str.length + 1)
  buf[str.length] = 0
  const bufView = new Uint8Array(buf)
  for (let i = 0, strLen = str.length; i < strLen; i++) {
    bufView[i] = str.charCodeAt(i)
  }
  return buf
}

assert.strictEqual(add(1, 2), 3)

assert.strictEqual(stringinout('World'), 'Hello World')

const name = str2ab('World')
const greet = ab2str(stringinout_utf8(name))
assert.strictEqual(greet, 'Hello World')

// host function to say hello, whcih will be exposed to wasm
globalThis.hello = n => {
  const name = ab2str(n)
  return str2ab(`Hello ${name}`)
}

globalThis.null0_log = m => {
  console.log('Log from WASM: ', ab2str(m))
}

// this will trigger the wasm to call env.hello
host_entry()

console.log('ok')
