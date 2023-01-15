// in js-host, I have to encode/decode the UTF8 string, because of how the assemblyscript wrapper is generated

// ArrayBuffer -> String
export function ab2str (buf) {
  return String.fromCharCode.apply(null, new Uint8Array(buf).slice(0, -1))
}

// String -> ArrayBuffer
export function str2ab (str) {
  const buf = new ArrayBuffer(str.length + 1)
  buf[str.length] = 0
  const bufView = new Uint8Array(buf)
  for (let i = 0, strLen = str.length; i < strLen; i++) {
    bufView[i] = str.charCodeAt(i)
  }
  return buf
}
