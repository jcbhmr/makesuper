#ifndef __COSMOCC__
#error "must be compiled with cosmocc"
#endif

#include <chrono>
#include <cosmo.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <spawn.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

std::string vector_string_join(const std::vector<std::string> &vector,
                               const std::string &delimiter);
static const char *GetHostOs();
static const char *GetHostIsa();
int my_execv(const char *path, char *const argv[]);
std::filesystem::path get_app_state_root(const std::string &appname);
void filesystem_copy_recursive(const std::filesystem::path &src,
                               const std::filesystem::path &dest);
std::string to_string_unordered_map_string_filesystem_path(
    const std::unordered_map<std::string, std::filesystem::path> &map);

int main(int argc, char *argv[]) {
  const auto superapp_txt_path = std::filesystem::path("/zip/superapp.txt");
  auto superapp_txt = std::ifstream(superapp_txt_path);

  std::string appname;
  superapp_txt >> appname;

  auto bin_relative_paths =
      std::unordered_map<std::string, std::filesystem::path>();
  std::string line;
  while (std::getline(superapp_txt, line)) {
    auto iss = std::istringstream(line);
    std::string key;
    std::string value;
    iss >> key;
    iss >> value;
    bin_relative_paths[key] = std::filesystem::path(value);
  }

  auto const platform_pair = std::string(GetHostOs()) + "-" + GetHostIsa();
  const auto zip_platform_root = std::filesystem::path("/zip") / platform_pair;
  const auto app_state_root = get_app_state_root(appname);
  auto const bin_relative_path = bin_relative_paths.find(platform_pair);
  if (bin_relative_path == bin_relative_paths.end()) {
    auto oss = std::ostringstream();
    oss << platform_pair << " not in "
        << to_string_unordered_map_string_filesystem_path(bin_relative_paths);
    throw std::runtime_error(oss.str());
  }
  auto const bin_path = app_state_root / bin_relative_path->second;

  if (!std::filesystem::exists(app_state_root)) {
    std::filesystem::create_directories(app_state_root);
    // For some reason std::filesystem::copy() doesn't work.
    // std::filesystem::copy(zip_platform_root, app_state_root,
    // std::filesystem::copy_options::recursive);
    filesystem_copy_recursive(zip_platform_root, app_state_root);
  }

  const int err = my_execv(bin_path.c_str(), argv);
  if (err) {
    auto const args = std::vector<std::string>(argv, argv + argc);
    auto oss = std::ostringstream();
    oss << "my_execv() " << bin_path << " " << vector_string_join(args, " ")
        << ": " << err;
    throw std::runtime_error(oss.str());
  }
}

std::string to_string_unordered_map_string_filesystem_path(
    const std::unordered_map<std::string, std::filesystem::path> &map) {
  auto oss = std::ostringstream();
  oss << "{";
  auto first = true;
  for (auto const &[key, value] : map) {
    if (first) {
      first = false;
    } else {
      oss << ",";
    }
    oss << key << "=" << value;
  }
  oss << "}";
  return oss.str();
}

std::string vector_string_join(const std::vector<std::string> &vector,
                               const std::string &delimiter) {
  std::ostringstream oss;
  for (size_t i = 0; i < vector.size(); ++i) {
    if (i > 0) {
      oss << delimiter;
    }
    oss << vector[i];
  }
  return oss.str();
}

// https://github.com/jart/cosmopolitan/blob/efb3a346086b0b54c923eae7c847c1af5795d4c6/tool/net/lfuncs.c#L197-L237
static const char *GetHostOs() {
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
static const char *GetHostIsa() {
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

int my_execv(const char *path, char *const argv[]) {
  if (IsWindows()) {
    pid_t pid;
    int err;
    const auto path_string = std::string(path);
    if (path_string.ends_with(".bat") || path_string.ends_with(".cmd")) {
      int argc = 0; 
      while(argv[++argc]);
      char * *new_argv = (char**)malloc((argc + 5) * sizeof(argv[0]));
      new_argv[0] = (char*)malloc(20 + 3 + 3 + 3 + path_string.length() + 1);
      memcpy(new_argv[0], "C:\\System32\\cmd.exe", 20);
      new_argv[1] = new_argv[0] + 20;
      memcpy(new_argv[1], "/d", 3);
      new_argv[2] = new_argv[1] + 3;
      memcpy(new_argv[2], "/s", 3);
      new_argv[3] = new_argv[2] + 3;
      memcpy(new_argv[3], "/c", 3);
      new_argv[4] = new_argv[3] + 3;
      memcpy(new_argv[4], path, path_string.length() + 1);
      for (int i = 1; i < argc; i++) {
        new_argv[4 + i] = argv[i];
      }
      new_argv[argc] = NULL;
      err = posix_spawn(&pid, "/C/System32/cmd.exe", NULL, NULL, new_argv, NULL);
    } else {
      err = posix_spawn(&pid, path, NULL, NULL, argv, NULL);
    }
    if (err) {
      return err;
    }
    pid_t status;
    while (waitpid(pid, &status, 0) != -1)
      ;
    std::exit(status);
  } else {
    return execv(path, argv);
  }
}

std::filesystem::path get_app_state_root(const std::string &appname) {
  if (IsWindows()) {
    const auto localappdata = std::getenv("LOCALAPPDATA");
    if (!localappdata) {
      throw std::runtime_error("%LOCALAPPDATA% env not set");
    }
    return std::filesystem::path(localappdata) / appname;
  } else {
    auto const xdg_state_home = std::getenv("XDG_STATE_HOME");
    if (xdg_state_home) {
      return std::filesystem::path(xdg_state_home) / appname;
    } else {
      auto const home = std::getenv("HOME");
      if (!home) {
        throw std::runtime_error("$HOME env not set");
      }
      return std::filesystem::path(home) / ".local/state" / appname;
    }
  }
}

void filesystem_copy_recursive(const std::filesystem::path &src,
                               const std::filesystem::path &dest) {
  for (auto const &entry : std::filesystem::directory_iterator(src)) {
    auto const relative_path = entry.path().lexically_relative(src);
    auto const dest_path = dest / relative_path;
    if (entry.is_directory()) {
      std::filesystem::create_directories(dest_path);
    } else if (entry.is_regular_file()) {
      auto ifs = std::ifstream(entry.path(), std::ios::binary);
      auto ofs = std::ofstream(dest_path, std::ios::binary);
      std::copy(std::istreambuf_iterator<char>(ifs),
                std::istreambuf_iterator<char>(),
                std::ostreambuf_iterator<char>(ofs));
      std::filesystem::permissions(dest_path, entry.status().permissions());
    } else {
      auto oss = std::ostringstream();
      oss << entry.path() << " is not a directory or regular file";
      throw std::runtime_error(oss.str());
    }
  }
}