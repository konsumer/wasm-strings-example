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

static M3Function* wmalloc;
static M3Function* wfree;
static M3Function* add;
static M3Function* say_hello;
static M3Function* test_string_retstring;
static M3Function* test_string_param;


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
  printf("test_string_get was called.\n");
  m3ApiReturnType (uint32_t);

  // example of what you want to return
  char* buffer = "Hello from C host.";

  // lowerBuffer
  uint32_t wPointer;
  size_t s = strlen(buffer) + 1;
  null0_check_wasm3(m3_CallV (wmalloc, s));
  m3_GetResultsV(wmalloc, &wPointer);
  char* wBuffer = m3ApiOffsetToPtr(wPointer);
  memcpy(wBuffer, buffer, s);

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

  env = m3_NewEnvironment();
  runtime = m3_NewRuntime (env, 1024 * 1024, NULL);
  null0_check_wasm3(m3_ParseModule (env, &module, wasmBuffer, size));
  null0_check_wasm3(m3_LoadModule(runtime, module));

  // IMPORTS
  m3_LinkRawFunction(module, "env", "null0_log", "v(i)", &null0_log);
  m3_LinkRawFunction(module, "env", "abort", "v(iiii)", &null0_abort);
  m3_LinkRawFunction(module, "env", "test_string_get", "*()", &test_string_get);

  null0_check_wasm3_is_ok();

  // EXPORTS
  m3_FindFunction(&wmalloc, runtime, "wmalloc");
  m3_FindFunction(&wfree, runtime, "wfree");
  m3_FindFunction(&add, runtime, "add");
  m3_FindFunction(&say_hello, runtime, "say_hello");
  m3_FindFunction(&test_string_retstring, runtime, "test_string_retstring");
  m3_FindFunction(&test_string_param, runtime, "test_string_param");

  null0_check_wasm3_is_ok();

  // check exports
  if (!wmalloc) {
    fprintf(stderr, "wmalloc not defined.\n");
    return 1;
  }
  if (!wfree) {
    fprintf(stderr, "wfree not defined.\n");
    return 1;
  }
  if (!add) {
    fprintf(stderr, "add not defined.\n");
    return 1;
  }
  if (!say_hello) {
    fprintf(stderr, "say_hello not defined.\n");
    return 1;
  }
  if (!test_string_retstring) {
    fprintf(stderr, "test_string_retstring not defined.\n");
    return 1;
  }
  if (!test_string_param) {
    fprintf(stderr, "test_string_param not defined.\n");
    return 1;
  }

  // call add & get return-value
  null0_check_wasm3(m3_CallV(add, 1, 2));
  int value;
  m3_GetResultsV(add, &value);
  assert(value == 3);
  printf("add worked: %d\n", value);

  // call say_hello to trigger reaching back into host
  null0_check_wasm3(m3_CallV(say_hello));

  uint32_t wPointer;
  char* wBuffer;
  size_t s;


  // WASM -> HOST via return
  // XXX: this is not working, yet. Need m3ApiOffsetToPtr outside of callback.
  // null0_check_wasm3(m3_CallV(test_string_retstring));

  // HOST -> WASM via param
  // XXX: this is not working, yet. Need m3ApiOffsetToPtr outside of callback.
  // char* testParam = "Hello from C host.";
  // s = strlen(testParam) + 1;
  // null0_check_wasm3(m3_CallV (wmalloc, s));
  // m3_GetResultsV(wmalloc, &wPointer);
  // wBuffer = m3ApiOffsetToPtr(wPointer);
  // memcpy(wBuffer, testParam, s);
  // null0_check_wasm3(m3_CallV(test_string_param, wPointer));
  
 
  printf("ok\n");
  return 0;
}