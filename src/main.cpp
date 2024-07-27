#ifndef __COSMOCC__
  #error "must be compiled with cosmocc"
#endif

#include <iostream>
#include <filesystem>
#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <string>
#include <chrono>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <unistd.h>
#include <cosmo.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>

std::string vector_string_join(const std::vector<std::string>& vector, const std::string& delimiter);
static const char* GetHostOs();
static const char* GetHostIsa();
int my_execv(const char* path, char *const argv[]);

int main(int argc char *argv[]) {
  auto makesuper_txt_path = std::filesystem::path("/zip/makesuper.txt");
  auto makesuper_txt = std::ifstream(makesuper_txt_path);
  
  std::string appname;
  makesuper_txt >> appname;
  if (!makesuper_txt) {
    auto oss = std::ostringstream();
    oss << "std::ifstream() " << makesuper_txt_path << " error";
    throw std::runtime_error(oss.str());
  }
  
  auto bin_relative_paths = std::unordered_map<std::string, std::filesystem::path>();
  std::string line;
  while (std::getline(ifs, line)) {
    auto iss = std::istringstream(line);
    std::string key;
    std::string value;
    iss >> key;
    iss >> value;
    bin_relative_paths[key] = std::filesystem::path(value);
  }

  // TODO

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




























// For some reason the native std::filesystem::copy() and std::filesystem::copy_file() FAIL when trying to copy
// from the virtual /zip filesystem to the real filesystem. I don't know why.
void filesystem_copy_file( const std::filesystem::path& src, const std::filesystem::path& dest) {
  auto ifs = std::ifstream(src, std::ios::binary);
                       if (!ifs) {
                        auto oss = std::ostringstream();
                        oss << src << " ifstream error after std::ifstream()";
                        throw std::runtime_error(oss.str());
                       }
  auto ofs = std::ofstream(dest, std::ios::binary);
                       if (!ofs) {
                        auto oss = std::ostringstream();
                        oss << dest << " ofstream error after std::ofstream()";
                        throw std::runtime_error(oss.str());
                       }

std::copy( std::istreambuf_iterator<char>(ifs),
      std::istreambuf_iterator<char>(),
      std::ostreambuf_iterator<char>(ofs)
);
                       if (!ifs) {
                        auto oss = std::ostringstream();
                        oss << src << " ifstream error after std::copy()";
                        throw std::runtime_error(oss.str());
                       }
                       if (!ofs) {
                        auto oss = std::ostringstream();
                        oss << dest << " ofstream error after std::copy()";
                        throw std::runtime_error(oss.str());
                       }
                       auto const perms = std::filesystem::status(src).permissions();
                       std::filesystem::permissions(dest, perms);
                       auto const mtime = std::filesystem::last_write_time(src);
                       std::filesystem::last_write_time(dest, mtime);
}

// rsync --archive --delete
void rsync_archive_delete( const std::filesystem::path& src_root, const std::filesystem::path& dest_root) {
    std::filesystem::create_directories(dest_root);
    for (auto const& src_entry : std::filesystem::recursive_directory_iterator(src_root)) {
        auto const relative_path = src_entry.path().lexically_relative(src_root);
        auto const dest_path = dest_root / relative_path;
        if (src_entry.is_directory()) {
            std::filesystem::create_directories(dest_path);
        } else if (src_entry.is_regular_file()) {
            filesystem_copy_file(src_entry.path(), dest_path);
        } else {
                        auto oss = std::ostringstream();
                        oss << src_entry.path() << " is not a directory or a regular file";
                        throw std::runtime_error(oss.str());
        }
    }
    for (const auto& dest_entry : std::filesystem::recursive_directory_iterator(dest_root)) {
        auto const relative_path = dest_entry.path().lexically_relative(dest_root);
        auto const src_path = src_root / relative_path;
        if (!std::filesystem::exists(src_path)) {
            std::filesystem::remove_all(dest_entry.path());
        }
    }
                       auto const mtime = std::filesystem::last_write_time(src_root);
                       std::filesystem::last_write_time(dest_root, mtime);
}

    // cosmopolitan execv() on Windows has issues. It's like it spawns a sibling process? Output doesn't show up
    // as output of this process but instead AFTER it's completed. It's weird. In PowerShell I can't
    // even see the output. Unsure if this is how it should be or if this is a bug? posix_spawn() works.
int my_execv(const std::filesystem::path& path, char *const argv[]) {
  if (IsWindows()) {
        pid_t pid;
        auto err = posix_spawn(&pid, path.c_str(), NULL, NULL, argv, NULL);
        if (err) {
            return err;
        }
        pid_t status;
        while (waitpid(pid, &status, 0) != -1);
        std::exit(status);
    } else {
        auto const err = execv(path.c_str(), argv);
        if (err) {
            return err;
        }
        std::unreachable();
    }
}

int main(int argc, char *argv[]) {
    auto const zip_root = std::filesystem::path("/zip");
    auto const exe_path = std::filesystem::path(GetProgramExecutableName());
    auto const platform_pair = std::string(GetHostOs()) + "-" + GetHostIsa();
    auto const zip_platform_root = zip_root / platform_pair;
    // Maybe $XDG_STATE_DIR or something is better? Basically want it keyed to THIS BINARY and let it be moved.
    // Want to mirror the behaviour or PyInstaller somewhat, but without the "no caching ever" rule.
    auto const extracted_root = std::filesystem::path(exe_path.string() + "_" + platform_pair + "_extracted");

    auto const zip_platform_mtime = std::filesystem::last_write_time(zip_platform_root);
    std::error_code err;
    auto const extracted_mtime = std::filesystem::last_write_time(extracted_root, err);
    if (err) {
      // Do nothing.
    }
    if (extracted_mtime == zip_platform_mtime) {
      // Do nothing. Assume perfectly in sync.
    } else if (extracted_mtime > zip_platform_mtime) {
      // Do nothing. Assume that the application has stored some state in its install folder.
      // rsync_archive_delete(zip_platform_root, extracted_root);
    } else if (extracted_mtime < zip_platform_mtime) {
      // Outdated. Update from the embedded /zip/<platform_pair> archive.
      rsync_archive_delete(zip_platform_root, extracted_root);
    } else {
      std::unreachable();
    }
    
    auto ifs = std::ifstream("/zip/makesuper.txt");
    std::string line;
    std::string key;
    std::string value;
    auto ok = false;
    while (std::getline(ifs, line)) {
      auto iss = std::istringstream(line);
      iss >> key;
      iss >> value;
      if (key == platform_pair) {
        ok = true;
        break;
      }
    }
    if (!ok) {
      auto oss = std::ostringstream();
      oss << "no key for " << platform_pair << " in makesuper.txt";
      throw std::runtime_error(oss.str());
    }
    auto const bin_path = extracted_root / value;
    auto const err2 = my_execv(bin_path, argv);
    if (err2) {
      auto const args = std::vector<std::string>(argv + 1, argv + argc);
      auto oss = std::ostringstream();
      oss << "my_execv() " << bin_path << " " << vector_string_join(args, " ") << ": " << err2;
      throw std::runtime_error(oss.str());
    }
}




std::string vector_string_join(const std::vector<std::string>& vector, const std::string& delimiter) {
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
    std::exit(status);
  } else {
    return execv(path, argv);
  }
}
