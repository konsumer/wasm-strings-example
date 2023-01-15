// The entry file of your WebAssembly module.

// log a string
@external("env", "null0_log")
declare function null0_log(text: ArrayBuffer): void
export function log(text: string): void {
  null0_log(String.UTF8.encode(text, true))
}

// included in example, shows basic stuff works
export function add(a: i32, b: i32): i32 {
  return a + b;
}

// gets string from host, returns string to host, this only works with correct host-side processing
export function stringinout(name: string) : string {
  return `Hello ${name}`
}

// this is UTF8 (null-terminated) wrapped, so it works like other languages, in the host
// in my assemblyscript header, I would normally wrap this, so it uses string in/out (to make it easier to use)
export function stringinout_utf8(n: ArrayBuffer) : ArrayBuffer {
  const name = String.UTF8.decode(n, true)
  return String.UTF8.encode(`Hello ${name}`, true)
}

// host will define this, so it can check return value
@external("env", "hello")
declare function hello_host(url: ArrayBuffer): ArrayBuffer

// entry point for the host to triger calling hello(name: string): string
export function host_entry():void {
  const greet:string = String.UTF8.decode(hello_host(String.UTF8.encode(`ASM`, true)), true)
  log('(WASM) Host said: ' + greet);
}
