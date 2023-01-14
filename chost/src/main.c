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

// all wasm3 functions return same sort of error-pattern, so this wraps that
static void null0_check_wasm3 (M3Result result) {
  if (result) {
    M3ErrorInfo info;
    m3_GetErrorInfo(runtime, &info);
    fprintf(stderr, "%s - %s", result, info.message);
    exit(1);
  }
}

// this checks the general state of the runtime, to make sure there are no errors lingering
static void null0_check_wasm3_is_ok () {
  M3ErrorInfo error;
  m3_GetErrorInfo(runtime, &error);
  if (error.result) {
    fprintf(stderr, "%s - %s", error.result, error.message);
    exit(1);
  }
}

static m3ApiRawFunction (hello) {
  m3ApiReturnType (char*);
  m3ApiGetArgMem(const char*, name);

  char* response;
  sprintf(response, "Hello %s", name);

  m3ApiReturn(response);
  // m3ApiSuccess();
}

int main (int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: chost WASMFILE.wasm\n");
    return 1;
  }

  u8* wasmBuffer;
  struct stat sb;

  if (lstat(argv[1], &sb) == -1) {
    fprintf(stderr, "Could not open wasm\n");
    return 1;
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd != -1) {
    read(fd, wasmBuffer, sb.st_size);
    close(fd);
  } else {
    fprintf(stderr, "Could not open wasm\n");
    return 1;
  }


  env = m3_NewEnvironment();
  runtime = m3_NewRuntime (env, 1024, NULL);
  null0_check_wasm3(m3_ParseModule (env, &module, wasmBuffer, sb.st_size));
  null0_check_wasm3(m3_LoadModule(runtime, module));

  m3_LinkRawFunction(module, "env", "hello", "i(i)", &hello);

  // this will trigger the wasm to call env.hello
  static M3Function* host_entry;
  m3_FindFunction(&host_entry, runtime, "host_entry");
  m3_CallV(host_entry);
  return 0;
}