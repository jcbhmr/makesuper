/*
 * zip contents
 *     .
 *     ├── LINUX-X86_64/
 *     │   └── app
 *     ├── LINUX-AARCH64/
 *     │   └── app
 *     ├── XNU-X86_64/
 *     │   └── app
 *     ├── XNU-AARCH64/
 *     │   └── app
 *     ├── WINDOWS-X86_64/
 *     │   └── app.exe
 *     └── makesuper.txt
 *
 * makesuper.txt
 *     superapp
 *     LINUX-X86_64   app
 *     LINUX-AARCH64  app
 *     XNU-X86_64     app
 *     XNU-AARCH64    app
 *     WINDOWS-X86_64 app.exe
 */

#ifndef __COSMOCC__
  #error "must be compiled with cosmocc"
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <cosmo.h>

static const char* GetHostOs();
static const char* GetHostIsa();
int my_execv(const char* path, char *const argv[]);

// https://github.com/jart/cosmopolitan/blob/efb3a346086b0b54c923eae7c847c1af5795d4c6/tool/net/lfuncs.c#L197-L237
static const char* GetHostOs() {
  const char *s = NULL;
  if (IsLinux()) {
    s = "LINUX";
  } else if (IsMetal()) {
    s = "METAL";
  } else if (IsWindows()) {
    s = "WINDOWS";
  } else if (IsXnu()) {
    s = "XNU";
  } else if (IsOpenbsd()) {
    s = "OPENBSD";
  } else if (IsFreebsd()) {
    s = "FREEBSD";
  } else if (IsNetbsd()) {
    s = "NETBSD";
  }
  return s;
}

// https://github.com/jart/cosmopolitan/blob/efb3a346086b0b54c923eae7c847c1af5795d4c6/tool/net/lfuncs.c#L197-L237
static const char* GetHostIsa() {
  const char *s;
#ifdef __x86_64__
  s = "X86_64";
#elif defined(__aarch64__)
  s = "AARCH64";
#elif defined(__powerpc64__)
  s = "POWERPC64";
#elif defined(__s390x__)
  s = "S390X";
#else
#error "unsupported architecture"
#endif
  return s;
}

int my_execv(const char* path, char *const argv[]) {
  if (IsWindows()) {
    pid_t pid;
    const int err = posix_spawn(&pid, path, NULL, NULL, argv, NULL);
    if (err) {
      return err;
    }
    pid_t status;
    while (waitpid(pid, &status, 0) != -1);
    exit(status);
  } else {
    return execv(path, argv);
  }
}

int main(int argc char *argv[]) {
  FILE* makesuper_txt = fopen("/zip/makesuper.txt", "r");
  
  char appname[30];
  fscanf(makesuper_txt, "%s", appname);

  char extracted_root[260];
  if (IsWindows()) {
    strcat(extracted_root, getenv("LOCALAPPDATA"));
    strcat(extracted_root, "/");
    strcat(extracted_root, appname);
  } else {
    char* xdg_share_home = getenv("XDG_STATE_HOME");
    if (xdg_state_home) {
      strcat(extracted_root, xdg_state_home);
    } else {
      strcat(extracted_root, getenv("HOME");
      if (IsXnu()) {
        strcat(extracted_root, "/Library/Application Support");
      } else {
        strcat(extracted_root, "/.local/state");
      }
    }
  }
  
  char platform_pair[20];
  strcat(platform_pair, GetHostOs());
  strcat(platform_pair, "-");
  strcat(platform_pair, GetHostIsa());

  char zip_root[30];
  strcat(zip_root, "/zip/");
  strcat(zip_root, platform_pair);

  filesystem_copy(zip_root, extracted_root);
  
  char key[20];
  char value[100];
  bool ok = false;
  while (fscanf(makesuper_txt, "%s %s", key, value) != EOF) {
    if (strcmp(platform_pair, key) == 0) {
      ok = true;
      break;
    }
  }
  
  strcat(extracted_root, "/");
  strcat(extracted_root, value);
  
  const int err = my_execv(extracted_root, argv);
  if (err) {
    printf("my_execve() %s failed: %d\n", extracted_root, err);
    exit(1);
  }
}
