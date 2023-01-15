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
static M3Function* __new;
static M3Function* host_entry;
static M3Function* add;
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

static m3ApiRawFunction (set_buffer) {
  m3ApiGetArgMem(char*, b);
  returnBuffer = b;
  m3ApiSuccess();
}

// this is exposed to wasm
static m3ApiRawFunction (hello) {
  m3ApiReturnType (char*);
  m3ApiGetArgMem(const char*, name);

  char* response;
  sprintf(response, "Hello %s", name);

  printf(response, "send to ASM: Hello %s\n", name);

  m3ApiReturn(response);
  m3ApiSuccess();
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
  m3_LinkRawFunction(module, "env", "set_buffer", "v(i)", &set_buffer);
  m3_LinkRawFunction(module, "env", "abort", "v(iiii)", &null0_abort);
  m3_LinkRawFunction(module, "env", "null0_log", "v(i)", &null0_log);
  m3_LinkRawFunction(module, "env", "hello", "i(i)", &hello);

  null0_check_wasm3_is_ok();

  // IMPORTS
  m3_FindFunction(&__new, runtime, "__new");
  m3_FindFunction(&host_entry, runtime, "host_entry");
  m3_FindFunction(&add, runtime, "add");

  null0_check_wasm3_is_ok();

  // simple return test
  null0_check_wasm3(m3_CallV (add, 1, 2));
  int value = 0;
  null0_check_wasm3(m3_GetResultsV(add, &value));
  assert(value == 3);

  // trigger WASM to call back into host
  m3_CallV(host_entry);
 
  printf("ok\n");
  return 0;
}