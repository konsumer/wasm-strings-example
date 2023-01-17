// The entry file of your WebAssembly module.

// malloc for ArrayBuffer
export function wmalloc(size: usize): usize {
  return __pin(__new(size, 1))
}

// free an ArrayBuffer
export function wfree(ptr: usize): void {
  __unpin(ptr)
}

// get a string from the host
@external("env", "test_string_get")
declare function test_string_get(): ArrayBuffer

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

// call into host, get a string, then log it (sending string to host)
export function say_hello(): void {
  log('testing log in wasm')
  log(String.UTF8.decode(test_string_get()))
}