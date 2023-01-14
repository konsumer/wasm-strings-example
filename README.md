This is a small example of passing strings back and forth over WASM, in a wasm3 host. SInce I need to support multiple wasm languages, the strings should be in 

Here is soem stuff you can do:
```
# install tools & deps
npm i 

# build the assemblyscript-based wasm
npm run asbuild:debug

# run JS-host tests
npm test

# build c host tester: you will need cmake/make
npm run chost

# run C-host tests
./chost/build/chost ./build/debug.wasm
```