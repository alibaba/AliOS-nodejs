// Copyright 2017 the Alibaba YunOS project.  All rights reserved.

#include <include/v8.h>
#include <src/utils.h>

#include <include/libplatform/libplatform.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include "src/flags.h"

#ifdef ENABLE_JSAOT_
// Reads a file into a v8 string.
static v8::MaybeLocal<v8::String> ReadFile(v8::Isolate* isolate, const char* name) {
  FILE* file = fopen(name, "rb");
  if (file == NULL) return v8::MaybeLocal<v8::String>();

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (size_t i = 0; i < size;) {
    i += fread(&chars[i], 1, size - i, file);
    if (ferror(file)) {
      fclose(file);
      delete[] chars;
      return v8::MaybeLocal<v8::String>();
    }
  }
  fclose(file);
  // Remove shebang
  // According to node/lib/module.js Module.prototype._compile
  size_t loc = 0;
  if (size >= 2) {
    if (chars[0] == '#' && chars[1] == '!') {
      for (loc = 2; loc < size; ++loc) {
        if (chars[loc] == '\n' || chars[loc] == '\r') {
          break;
        }
      }
    }
  }
  v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(
      isolate, chars + loc, v8::NewStringType::kNormal, static_cast<int>(size - loc));
  delete[] chars;
  return result;
}

static void SetFlagsFromString(const char* flags) {
  v8::V8::SetFlagsFromString(flags, static_cast<int>(strlen(flags)));
}

static void ResetDefaultFlagsForCompileJso() {
  // Reset default stack size if stack-size is set a value
  // less than default stack size in aot compile mode
  if (v8::internal::FLAG_stack_size < V8_DEFAULT_STACK_SIZE_KB) {
    char stack_size_flag[20]; // --stack-size=dddd";
    sprintf(stack_size_flag, "%s%d", "--stack-size=", V8_DEFAULT_STACK_SIZE_KB);
    SetFlagsFromString(stack_size_flag);
  }
}

static std::string Trim(std::string& str) {
  std::stringstream trimmer;
  trimmer << str;
  trimmer >> str;
  return str;
}

struct Options {
  const char* output_file = nullptr;
  const char* preloaded_jso = nullptr;
  bool flag_aot_dump = false;
  bool flag_skip_if_aot_exists = false;
  size_t node_module_start_index = 0;
  uintptr_t jso_range_start_addr = 0;
  size_t jso_range_size = 0; /* MB */
  std::vector<const char*> src_files;
};

static void ProcessFlagsForJso(int argc, char** argv, Options &o) {
  o.node_module_start_index = argc;
  bool has_source_list_file = false;
  bool has_node_module = false;


  for (int i = 1; i < argc; i++) {
    if (strncmp(argv[i], "--aot_out=", 10) == 0) {
      o.output_file = argv[i] + 10;
      continue;
    } else if (strncmp(argv[i], "--jso_range_size=", 17) == 0) {
      o.jso_range_size = static_cast<size_t>(atoi(argv[i] + 17));
      continue;
    } else if (strncmp(argv[i], "--jso_range_start_addr=", 23) == 0) {
      o.jso_range_start_addr = static_cast<uintptr_t>(atoi(argv[i] + 23));
      continue;
    } else if (strncmp(argv[i], "--preloaded_jso=", 16) == 0) {
      o.preloaded_jso = argv[i] + 16;
      continue;
    } else if (strncmp(argv[i], "--source_list_file=", 19) == 0) {
      has_source_list_file = true;
      o.node_module_start_index = 0;
      const char* source_list_file = argv[i] + 19;
      // Read source list file and push back all source file names into
      // o.src_files. A timmed line of text specifies one source file name.
      std::ifstream file(source_list_file);
      std::string line;
      while (file.peek() != EOF) {
        getline(file, line);
        std::string trim_line = Trim(line);
        if (trim_line.compare("--node_module") == 0) {
          o.node_module_start_index = o.src_files.size();
        } else {
          o.src_files.push_back(strdup(trim_line.c_str()));
        }
      }
      continue;
    } else if (strcmp(argv[i], "--aot_dump") == 0) {
      o.flag_aot_dump = true;
      continue;
    } else if (strcmp(argv[i], "--skip_if_aot_exists") == 0) {
      o.flag_skip_if_aot_exists = true;
      continue;
    } else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
      i++;  // skip script string in argument.
      continue;
    } else if (strcmp(argv[i], "--node_module") == 0) {
      if (!has_source_list_file) {
        o.node_module_start_index = o.src_files.size();
      }
      has_node_module = true;
      continue;
    } else if (strncmp(argv[i], "--", 2) == 0) {
      continue;
    }

    o.src_files.push_back(argv[i]);
  }

  if (!v8::internal::FLAG_test_d8) {
    SetFlagsFromString("--harmony");
  }

  if (has_node_module &&
      has_source_list_file) {
    v8::internal::PrintF("Notice!!! --node_module option will be ignored! \
            As --source_list_file is provided!\n");
  }

  ResetDefaultFlagsForCompileJso();
}

// node.js module wrapper from lib/internal/bootstrap_node.js:
// NativeModule.wrapper.
static const char* node_module_wrapper[] = {
  "(function (exports, require, module, __filename, __dirname, nativeLoad) { ",
  "\n});"
};

int main(int argc, char** argv) {
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);

  Options options;
  ProcessFlagsForJso(argc, argv, options);

  // Check whether it must skip to generate jso file
  if (options.output_file) {
    std::ifstream ifile(options.output_file);
    if (ifile && options.flag_skip_if_aot_exists) {
      v8::internal::PrintF("Skip to generate aot file as %s has existed!\n", options.output_file);
      v8::internal::PrintF("If you want to regenerate it, remove option --skip_if_aot_exists\n");
      return 0;
    }
  }
  v8::V8::InitializeJsoRange(options.jso_range_start_addr,
                             options.jso_range_size);

  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  v8::Platform* platform = v8::platform::CreateDefaultPlatform();
  v8::V8::InitializePlatform(platform);
  v8::V8::Initialize();
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate* isolate = v8::Isolate::New(create_params, true);
  {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);
    auto NewString = [isolate](const char* str) {
      return v8::String::NewFromUtf8(isolate, str,
                                     v8::NewStringType::kNormal).ToLocalChecked();
    };

    if (options.preloaded_jso) {
      v8::JsoLoader loader(isolate);
      const auto fd = open(options.preloaded_jso, 0);
      if (fd < 0) {
        v8::internal::PrintF("Failed to open preloaded jso file %s\n", options.preloaded_jso);
        return 1;
      }
      loader.Load(fd);
      close(fd);
    }

    if (!options.src_files.empty()) {
      v8::ApiObjectAccessScope aoa(isolate);
      v8::JsoGenerator generator(isolate);
      for (size_t i = 0; i < options.src_files.size(); i++) {
        const auto filename = options.src_files[i];
        char* resolved_filename = realpath(filename, nullptr);
        if (resolved_filename == nullptr) {
          v8::internal::PrintF("Failed to resolve real path for filename: %s",
                  filename);
          continue;
        }

        auto source_string = ReadFile(isolate, filename).ToLocalChecked();
        if (i >= options.node_module_start_index) {
          source_string = v8::String::Concat(
              v8::String::Concat(NewString(node_module_wrapper[0]), source_string),
              NewString(node_module_wrapper[1]));
        }
        v8::ScriptOrigin origin(NewString(resolved_filename));
        free((void*)resolved_filename);
        v8::ScriptCompiler::Source source(source_string, origin);
        generator.Compile(source);
      }

      generator.Generate();

      if (options.output_file) {
        FILE* jso_file = fopen(options.output_file, "wb");
        generator.Output(jso_file);
        fclose(jso_file);
      }

      if (options.flag_aot_dump) {
        generator.Print(stdout);
      }
    }
  }
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete platform;
  delete create_params.array_buffer_allocator;
  return 0;
}
#else

// This is workaround for empty main file for executable target
// 'mkjso' in v8.gyp
int main(int argc, char** argv) {
  return 0;
}
#endif  // ENABLE_JSAOT_
