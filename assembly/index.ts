// The entry file of your WebAssembly module.

// included in example, shows basic stuff works
export function add(a: i32, b: i32): i32 {
  return a + b;
}

// gets string from host, returns string to host
export function stringinout(name: string) : string {
  return `Hello ${name}`
}

// this is UTF8 (null-terminated) wrapped, so it works in like other languages, in the host
// in my assemblyscript header, I would wrap this, so it uses string in/out (to make it easier to use)
export function stringinout_utf8(n: ArrayBuffer) : ArrayBuffer {
  const name = String.UTF8.decode(n, true)
  return String.UTF8.encode(`Hello ${name}`, true)
}

// host will define this, so it can check return value
@external("env", "hello")
declare function hello_host(url: ArrayBuffer): ArrayBuffer


export function host_entry():void {
  const greet:string = String.UTF8.decode(hello_host(String.UTF8.encode(`ASM`, true)), true)
  trace('Got this from JS, in ASM: ' + greet)
}
