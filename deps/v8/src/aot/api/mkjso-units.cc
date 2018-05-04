/**
  * {{{ Copyright (C) 2016 The YunOS Project. All rights reserved. }}}
  */

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <sys/statvfs.h>

#include <include/mkjso-units.h>

#define DEBUGING 0
#if DEBUGING
#define LOG_I(...) fprintf(stderr, __VA_ARGS__)
#define LOG_D(...) fprintf(stderr, __VA_ARGS__)
#define LOG_E(...) fprintf(stderr, __VA_ARGS__)
#else
#if defined(V8_CAN_USE_LOGCAT)
#include <log/Log.h>
#define LOG_TAG "mkjso"
#else
#define LOG_D(...) ((void)0)
#define LOG_I(...) ((void)0)
#define LOG_E(...) ((void)0)
#endif
#endif

using std::string;

namespace yunos {


static const char kJsoListName[] = "jso_file.list";
static const char kMkJsoBin[] = "/usr/bin/mkjso";
static const char kPreLoadedFrameworkJso[] =
                    "--preloaded_jso=/data/framework.jso";
static const char kSourceListFile[] = "--source_list_file=";
static const char kAotOut[] = "--aot_out=";
static const string kJsSuffix = ".js";

// Check whether src string ends with target string
static bool EndsWith(const string& src, const string& target) {
  if (target.size() > src.size()) {
    return false;
  }
  return std::equal(target.rbegin(), target.rend(), src.rbegin());
}

// Find all files with suffix ".js" in the specified directory "path"
// and save the full path name of found files into vector fileList
static void FindAllJsSrcFiles(const string& path,
                              std::vector<string>& fileList) {
  DIR* dirFile = opendir(path.c_str());
  if (dirFile) {
    struct dirent* hFile;
    errno = 0;
    while ((hFile = readdir( dirFile)) != NULL) {
       if (!strcmp(hFile->d_name, ".")) continue;
       if (!strcmp(hFile->d_name, "..")) continue;

       // If the current iterated item is directory,
       // recursively find all js source files
       if (hFile->d_type == DT_DIR) {
          string sub_dir = path + "/" + hFile->d_name;
          FindAllJsSrcFiles(sub_dir, fileList);
       }

       // check the current file name, if ends with suffix ".js",
       // push into fileList
       if (hFile->d_type == DT_REG && EndsWith(hFile->d_name, kJsSuffix)) {
         string jsFn = path + "/" + hFile->d_name;
         fileList.push_back(jsFn);
       }
    }
    closedir(dirFile);
  }
}

// Check whether there are enough free space to generate jso file
// As there is at least 1.5M one jso file at current stage, so if
// free space is less than 15M(must left some space for application),
// no free space left for generating jso file
static bool HasEnoughFreeSpace(const char* appRootPath) {
  struct statvfs buf;
  const unsigned long freeSpaceThreshold = 15 * 1024 * 1024;
  if (!statvfs(appRootPath, &buf)) {
    return (buf.f_bsize * buf.f_bfree) > freeSpaceThreshold;
  }
  return false;
}

static bool IsGenerateJsoNeeded(const char* appRootPath) {
  if (!HasEnoughFreeSpace(appRootPath)) {
    LOG_I("No enough free disk space for generateJso!\n");
    return false;
  }

  std::stringstream ss;
  ss << appRootPath << "/" << ".enable_jsaot";
  std::fstream contlFile;
  contlFile.open(ss.str().c_str(), std::ios::in);
  if (!contlFile) {
    LOG_I("There is lack of trait file %s, disable jsaot for application!\n", ss.str().c_str());
    return false;
  }
  std::vector<string> jsFilesList;
  FindAllJsSrcFiles(appRootPath, jsFilesList);
  if (jsFilesList.size() == 0) {
    return false;
  }

  string jsSrcFilesListName = appRootPath;
  jsSrcFilesListName += "/";
  jsSrcFilesListName += kJsoListName;

  std::ofstream listFile(jsSrcFilesListName.c_str());
  if (!listFile) {
    return false;
  }

  for (const auto iter : jsFilesList) {
    listFile << iter << std::endl;
  }

  listFile.close();
  return true;
}

// static
bool MkJsoUnits::GenerateJso(const char* appRootPath) {
  if (!IsGenerateJsoNeeded(appRootPath)) {
    return false;
  }
  std::vector<const char*> argv;

  string sourceListOption(kSourceListFile);
  string sourceListFile = appRootPath;
  sourceListFile += "/";
  sourceListFile += kJsoListName;
  sourceListOption += sourceListFile;

  string outFile(kAotOut);
  outFile += appRootPath;
  outFile += "/";
  outFile += basename(appRootPath);
  outFile += ".jso";

  argv.push_back(kMkJsoBin);
  argv.push_back(kPreLoadedFrameworkJso);
  argv.push_back(sourceListOption.c_str());
  argv.push_back(outFile.c_str());
  argv.push_back(nullptr);

  // fork and exec
  pid_t pid = fork();
  if (pid == 0) {
    // change process groups, so we don't get reaped by ProcessManager
    setpgid(0, 0);
    for (unsigned int i=0; i<argv.size(); i++)
      LOG_D("GenerateJso exec mkjso, args= %s\n", argv[i]);
    execv(kMkJsoBin, const_cast<char* const*>(argv.data()));
    LOG_E("Failed to execute command:%s\n", kMkJsoBin);
    exit(1);
  } else {
    if (pid == -1) {
      LOG_D("Failed to execv %s %s because fork failed: %s\n",
            kMkJsoBin, sourceListOption.c_str(), strerror(errno));
      return false;
    }
    // wait for subprocess to finish
    int status = 0;
    pid_t got_pid = static_cast<pid_t>(TEMP_FAILURE_RETRY(waitpid(pid, &status, 0)));
    if (got_pid != pid) {
      LOG_D("Failed after fork for execv %s %s %s because waitpid "
            "failed: wanted %d, got %d: %s\n", kMkJsoBin,
             sourceListOption.c_str(), kMkJsoBin, pid, got_pid,
             strerror(errno));
      return false;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      LOG_D("Failed execv %s %s %s because non-0 exit status %d\n",
            kMkJsoBin, sourceListOption.c_str(), kMkJsoBin, status);
      return false;
    }
  }

  if (remove(sourceListFile.c_str()) != 0) {
    LOG_D("Failed to delete source list file %s\n", sourceListFile.c_str());
  }
  LOG_D("GenerateJso successful with file %s\n", outFile.c_str());
  return true;
}

} // namespace
