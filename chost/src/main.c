#include <stdio.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "wasm3.h"
#include "m3_env.h"

static M3Environment* env;
static M3Runtime* runtime;
static M3Module* module;
static M3Function* add;
static M3Function* wmalloc;
static M3Function* wfree;
static M3Function* say_hello;
char* returnBuffer;


// this checks the general state of the runtime, to make sure there are no errors lingering
static void null0_check_wasm3_is_ok () {
  M3ErrorInfo error;
  m3_GetErrorInfo(runtime, &error);
  if (error.result) {
    fprintf(stderr, "%s - %s\n", error.result, error.message);
    exit(1);
  }
}

// all wasm3 functions return same sort of error-pattern, so this wraps that
static void null0_check_wasm3 (M3Result result) {
  if (result) {
    null0_check_wasm3_is_ok();
    exit(1);
  }
}

// Fatal error
static m3ApiRawFunction (null0_abort) {
  m3ApiGetArgMem(const char*, message);
  m3ApiGetArgMem(const char*, fileName);
  m3ApiGetArg(uint16_t, lineNumber);
  m3ApiGetArg(uint16_t, columnNumber);
  fprintf(stderr, "%s at %s:%d:%d\n", message, fileName, lineNumber, columnNumber);
  m3ApiSuccess();
}

// Log a string
static m3ApiRawFunction (null0_log) {
  m3ApiGetArgMem(const char*, message);
  printf("Log from WASM: %s\n", message);
  m3ApiSuccess();
}

static m3ApiRawFunction (test_string_get) {
  m3ApiReturnType (uint32_t);
  uint32_t wPointer;

  // example of what you want to return
  char* buffer = "Hello from the host.";

  // lowerBuffer
  // size_t s = strlen(buffer);
  // printf("length: %zu\ncalling wmalloc\n", s);
  // null0_check_wasm3(m3_CallV (wmalloc, s));
  // m3_GetResultsV(wmalloc, &wPointer);
  // printf("wasm pointer for return: %d\n", wPointer);
  // char* wBuffer = m3ApiOffsetToPtr(wPointer);
  // memcpy(wBuffer, buffer, s);
  // printf("buffer sent to wasm: %s\n", wBuffer);

  m3ApiReturn(wPointer);
  m3ApiSuccess();
}

int main (int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: chost WASMFILE.wasm\n");
    return 1;
  }

  // read the wasm file
  FILE* pFile = fopen(argv[1], "rb");
  fseek(pFile, 0L, SEEK_END);
  size_t size = ftell(pFile);
  fseek(pFile, 0L, SEEK_SET);
  unsigned char wasmBuffer[size];
  fread(wasmBuffer, sizeof(wasmBuffer), 1, pFile);

  // load the wasm file
  env = m3_NewEnvironment();
  runtime = m3_NewRuntime (env, 1024, NULL);
  null0_check_wasm3(m3_ParseModule (env, &module, (const unsigned char *)wasmBuffer, size));
  null0_check_wasm3(m3_LoadModule(runtime, module));

  // EXPORTS
  m3_LinkRawFunction(module, "env", "abort", "v(iiii)", &null0_abort);
  m3_LinkRawFunction(module, "env", "null0_log", "v(i)", &null0_log);
  m3_LinkRawFunction(module, "env", "test_string_get", "i()",  &test_string_get);
  

  /* something like this might be good for interop:

  (SuppressLookupFailure (m3_LinkRawFunction (module, env, "_debug",            "i(*i)",   &m3_libc_print)));
  (SuppressLookupFailure (m3_LinkRawFunction (module, env, "_memset",           "*(*ii)",  &m3_libc_memset)));
  (SuppressLookupFailure (m3_LinkRawFunction (module, env, "_memmove",          "*(**i)",  &m3_libc_memmove)));
  (SuppressLookupFailure (m3_LinkRawFunction (module, env, "_memcpy",           "*(**i)",  &m3_libc_memmove))); // just alias of memmove
  (SuppressLookupFailure (m3_LinkRawFunction (module, env, "_abort",            "v()",     &m3_libc_abort)));
  (SuppressLookupFailure (m3_LinkRawFunction (module, env, "_exit",             "v(i)",    &m3_libc_exit)));
  (SuppressLookupFailure (m3_LinkRawFunction (module, env, "clock_ms",          "i()",     &m3_libc_clock_ms)));
  (SuppressLookupFailure (m3_LinkRawFunction (module, env, "printf",            "i(**)",   &m3_libc_printf)));

  and maybe also add the assemblyscript functions as aliases (trace -> printf, abort -> _abort, etc)

  */

  null0_check_wasm3_is_ok();

  // IMPORTS
  m3_FindFunction(&wmalloc, runtime, "wmalloc");
  m3_FindFunction(&wfree, runtime, "wfree");

  m3_FindFunction(&add, runtime, "add");
  m3_FindFunction(&say_hello, runtime, "say_hello");

  null0_check_wasm3_is_ok();

  // simple return test
  null0_check_wasm3(m3_CallV (add, 1, 2));
  int value = 0;
  null0_check_wasm3(m3_GetResultsV(add, &value));
  assert(value == 3);
  printf("add worked: %d\n", value);

  // trigger the WASM to call into the host to get a string
  m3_CallV (say_hello);
  
 
  printf("ok\n");
  return 0;
}