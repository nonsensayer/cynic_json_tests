/* 
// This is a basic template for compiling ungrateful projects.
// Copy and modify for your likeness.
// 
// deps/
//   ungrateful/
//   nob.h/nob.h
// src/
//   entry.c
// build.c (this file)
//
// Defines that are set up after set_cc():
// 
// BUILD_ASAN           - for CONFIG_ASAN.
// BUILD_DEBUG          - for CONFIG_DEBUG.
// BUILD_OPTIMIZED      - for CONFIG_OPTIMIZED.
// BUILD_PUBLISH,NDEBUG - for CONFIG_PUBLISH.
//
*/

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#define NOB_NO_ECHO
#define NOB_IMPLEMENTATION
#include "./deps/nob.h/nob.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define CYNIC    true
#define VISAGE   false
#define DISGRACE false

#if (VISAGE && !CYNIC)
#error Cynic should be defined for using visage
#endif

#if (DISGRACE && !CYNIC)
#error Cynic should be defined for using disgrace
#endif

#define UNGRATEFUL_SOURCE_DIR "deps/ungrateful/src"
#define JSON_TESTS_SOURCE_DIR "deps/JSONTestSuite/test_parsing"

#define PROGRAM_NAME    "jtest"
#define BUILD_FOLDER    "bin"
#define SOURCE_FOLDER   "src"
#define OBJECT_FOLDER   "obj"

#define FLAG_CLANG      "clang"
#define FLAG_NO_COLOR   "no-color"

#define FLAG_ALL        "all"

#define FLAG_RUN        "run"
#define FLAG_ASAN       "asan"
#define FLAG_OPTIMIZE   "opt"
#define FLAG_PUBLISH    "pub"

typedef int32_t  b32;
typedef uint32_t Status;
typedef uint32_t Toolchain;
typedef uint32_t Configuration;
typedef uint32_t System;

enum Toolchain {
    TOOLCHAIN_MSVC,
    TOOLCHAIN_GCC,
    TOOLCHAIN_CLANG,
    TOOLCHAIN_UNKN,
};

enum Configuration {
    CONFIG_ASAN,
    CONFIG_DEBUG,
    CONFIG_OPTIMIZED,
    CONFIG_PUBLISH,
};

enum System {
    SYSTEM_WINDOWS,
    SYSTEM_LINUX,
};


enum Status {
    STATUS_OK     = 0,
    STATUS_FAILED = 1
};

#ifdef _WIN32
#define TOOLCHAIN TOOLCHAIN_MSVC
#define SYSTEM    SYSTEM_WINDOWS
#else
#define TOOLCHAIN TOOLCHAIN_GCC
#define SYSTEM    SYSTEM_LINUX
#endif

Toolchain     toolchain     = TOOLCHAIN;
Configuration configuration = CONFIG_DEBUG;
System        os            = SYSTEM;
b32           flag_no_color = false;

b32 msvc_is_at_linking_stage = false; // only for MSVC

#define HAS_FLAG(__flag) check_for_pack_flag(__flag, argc, argv)

int check_for_pack_flag(char *flag, int argc, char **argv) {
    for (int i = 1; i < argc; i++) { 
        size_t a = strlen(argv[i]);
        size_t b = strlen(flag);
        if (a != b) continue;
        if (memcmp(argv[i], flag, a) == 0) return 1;
    }

    return 0;
}

void setup_msvc_cc(Nob_Cmd *cmd, Configuration config, b32 only_compile) {
    nob_cmd_append(cmd, "cl", "/nologo", "/cgthreads8");

    nob_cmd_append(cmd, "/std:c11", "/utf-8", "/validate-charset", "/TC");
    nob_cmd_append(cmd, "/W4", "/WX-", "/diagnostics:column", "/fp:fast");

    if (only_compile) nob_cmd_append(cmd, "/c");

    nob_cmd_append(cmd, "/D", "STRICT", "/D", "_CRT_SECURE_NO_WARNINGS", "/D", "_UNICODE", "/D", "UNICODE");

    switch (config) {
        case CONFIG_ASAN:
            nob_cmd_append(cmd,  "/D", "BUILD_ASAN");
            nob_cmd_append(cmd,  "/Zi", "/MTd", "/MP", "/Od", "/GS");
            nob_cmd_append(cmd, "/fsanitize=address");
            break;
            
        case CONFIG_DEBUG:
            nob_cmd_append(cmd,  "/D", "BUILD_DEBUG");
            nob_cmd_append(cmd, "/Zi", "/MTd", "/MP", "/Od", "/GS");
            nob_cmd_append(cmd, "/wd4244", "/wd5105", "/wd4127");
            break;

        case CONFIG_OPTIMIZED:
            nob_cmd_append(cmd, "/D", "BUILD_OPTIMIZED");
            nob_cmd_append(cmd, "/MT", "/MP", "/Ox", "/GS-", "/GL");
            nob_cmd_append(cmd, "/Qvec-report:1");
            break;

        case CONFIG_PUBLISH:
            nob_cmd_append(cmd, "/D", "NDEBUG", "/D", "BUILD_PUBLISH");
            nob_cmd_append(cmd, "/MT", "/MP", "/Ox", "/GS-", "/GL");
            break;
    }
}

void setup_clang_cc(Nob_Cmd *cmd, Configuration config, b32 only_compile, b32 colored_output) {
    nob_cmd_append(cmd, "clang");

    nob_cmd_append(cmd, "-std=gnu99", "-D_GNU_SOURCE");
    nob_cmd_append(cmd, "-Wall", "-Wno-unused-function", "-Wextra", "-Wno-error");

    nob_cmd_append(cmd, "-fPIC", "-ffast-math", "-pthread");
    nob_cmd_append(cmd, "-fstack-protector");

    if (colored_output) nob_cmd_append(cmd, "-fdiagnostics-color=always");
    else                nob_cmd_append(cmd, "-fdiagnostics-color=never");

    if (only_compile) nob_cmd_append(cmd, "-c");

    switch (config) {
        case CONFIG_ASAN:
            nob_cmd_append(cmd, "-DBUILD_ASAN", "-fsanitize=address");
            break;
        case CONFIG_DEBUG:
            nob_cmd_append(cmd, "-DBUILD_DEBUG", "-g", "-O0", "-fno-omit-frame-pointer");
            break;
        case CONFIG_OPTIMIZED:
            nob_cmd_append(cmd, "-DBUILD_OPTIMIZED", "-O3");
            break;
        case CONFIG_PUBLISH:
            nob_cmd_append(cmd, "-DBUILD_PUBLISH", "-DNDEBUG", "-s", "-O3");
            break;
    }
}

void setup_gcc_cc(Nob_Cmd *cmd, Configuration config, b32 only_compile, b32 colored_output) {
    nob_cmd_append(cmd, "gcc");

    nob_cmd_append(cmd, "-std=gnu99", "-D_GNU_SOURCE");
    nob_cmd_append(cmd, "-Wall", "-Wno-unused-function", "-Wextra", "-Wno-error");

    nob_cmd_append(cmd, "-fPIC", "-ffast-math", "-pthread");
    nob_cmd_append(cmd, "-fstack-protector");

    if (colored_output) nob_cmd_append(cmd, "-fdiagnostics-color=always");
    else                nob_cmd_append(cmd, "-fdiagnostics-color=never");

    if (only_compile) nob_cmd_append(cmd, "-c");

    switch (config) {
        case CONFIG_ASAN:
            nob_cmd_append(cmd, "-DBUILD_ASAN", "-fsanitize=address");
            break;
        case CONFIG_DEBUG:
            nob_cmd_append(cmd, "-DBUILD_DEBUG", "-g", "-O0", "-fno-omit-frame-pointer");
            break;
        case CONFIG_OPTIMIZED:
            nob_cmd_append(cmd, "-DBUILD_OPTIMIZED", "-O3");
            break;
        case CONFIG_PUBLISH:
            nob_cmd_append(cmd, "-DBUILD_PUBLISH", "-DNDEBUG", "-s", "-O3");
            break;
    }
}


void set_cc(Nob_Cmd *cmd, Configuration config, b32 only_compile, b32 colored_output) {
    switch (toolchain) {
        case TOOLCHAIN_MSVC:  setup_msvc_cc(cmd, config, only_compile); break;
        case TOOLCHAIN_GCC:   setup_gcc_cc(cmd, config, only_compile, colored_output); break;
        case TOOLCHAIN_CLANG: setup_clang_cc(cmd, config, only_compile, colored_output); break;
        default: NOB_UNREACHABLE("No such toolchain exists"); break;
    }
}

void set_include(Nob_Cmd *cmd, const char *folder) {
    switch (toolchain) {
        case TOOLCHAIN_MSVC:  nob_cmd_append(cmd, "/I", folder); break;
        case TOOLCHAIN_GCC:   nob_cmd_append(cmd, nob_temp_sprintf("-I%s", folder)); break;
        case TOOLCHAIN_CLANG: nob_cmd_append(cmd, nob_temp_sprintf("-I%s", folder)); break;
        default: NOB_UNREACHABLE("No such toolchain exists"); break;
    }
}

void set_obj_link(Nob_Cmd *cmd, const char *name) {
    switch (toolchain) {
        case TOOLCHAIN_MSVC:
            if (!msvc_is_at_linking_stage) {
                nob_cmd_append(cmd, "/link", "/INCREMENTAL:NO");
                msvc_is_at_linking_stage = true;
            }

            nob_cmd_append(cmd, nob_temp_sprintf(OBJECT_FOLDER"/%s.obj", name)); break;
            break;
        case TOOLCHAIN_GCC:
            nob_cmd_append(cmd, nob_temp_sprintf(OBJECT_FOLDER"/%s.o", name)); break;
            break;
        case TOOLCHAIN_CLANG:
            nob_cmd_append(cmd, nob_temp_sprintf(OBJECT_FOLDER"/%s.o", name)); break;
            break;
        default: NOB_UNREACHABLE("No such toolchain exists"); break;
    }
}

void set_lib_link(Nob_Cmd *cmd, const char *name) {
    switch (toolchain) {
        case TOOLCHAIN_MSVC:
            if (!msvc_is_at_linking_stage) {
                nob_cmd_append(cmd, "/link", "/INCREMENTAL:NO");
                msvc_is_at_linking_stage = true;
            }

            nob_cmd_append(cmd, nob_temp_sprintf("%s.lib", name)); break;
            break;
        case TOOLCHAIN_GCC:
            nob_cmd_append(cmd, nob_temp_sprintf("-l%s", name)); break;
            break;
        case TOOLCHAIN_CLANG:
            nob_cmd_append(cmd, nob_temp_sprintf("-l%s", name)); break;
            break;
        default: NOB_UNREACHABLE("No such toolchain exists"); break;
    }
}

void set_default_links(Nob_Cmd *cmd) {
    switch (os) {
        case SYSTEM_LINUX:
            set_lib_link(cmd, "m");
            // Glibc uses libmvec to automatically vectorize some functions.
            // There is no good way of disabling it, if we dont do our own sqrt
            // and so on.
            set_lib_link(cmd, "mvec");
            break;
        default:
            break;
    }
}

void set_entry(Nob_Cmd *cmd, const char *src, const char *name, const char *output_name, b32 only_compile) {
    if (!output_name) output_name = name;

    switch (toolchain) {
        case TOOLCHAIN_MSVC:
            nob_cmd_append(cmd, nob_temp_sprintf("%s/%s.c", src, name));

            if (only_compile) {
                nob_cmd_append(cmd, nob_temp_sprintf("/Fo./"OBJECT_FOLDER"/%s.obj", output_name));
                nob_cmd_append(cmd, nob_temp_sprintf("/Fd./"OBJECT_FOLDER"/%s.pdb", output_name));
            } else {
                nob_cmd_append(cmd, nob_temp_sprintf("/Fe./"BUILD_FOLDER"/%s.exe", output_name));
                nob_cmd_append(cmd, nob_temp_sprintf("/Fo./"OBJECT_FOLDER"/%s.obj", output_name));
                nob_cmd_append(cmd, nob_temp_sprintf("/Fd./"BUILD_FOLDER"/%s.pdb", output_name));
            }
            break;
        case TOOLCHAIN_GCC:
            nob_cmd_append(cmd, nob_temp_sprintf("%s/%s.c", src, name));

            if (only_compile) {
                nob_cmd_append(cmd, "-o", nob_temp_sprintf("./"OBJECT_FOLDER"/%s.o", output_name));
            } else {
                nob_cmd_append(cmd, "-o", nob_temp_sprintf("./"BUILD_FOLDER"/%s", output_name));
            }
            break;
        case TOOLCHAIN_CLANG:
            nob_cmd_append(cmd, nob_temp_sprintf("%s/%s.c", src, name));

            if (only_compile) {
                nob_cmd_append(cmd, "-o", nob_temp_sprintf("./"OBJECT_FOLDER"/%s.o", output_name));
            } else {
                nob_cmd_append(cmd, "-o", nob_temp_sprintf("./"OBJECT_FOLDER"/%s", output_name));
            }
            break;
        default: NOB_UNREACHABLE("No such toolchain exists"); break;
    }
}

Status libs_compile(void) {
    Status status = STATUS_OK;
    Nob_Cmd cmd = (Nob_Cmd) { 0 };
    size_t count;

    set_cc(&cmd, configuration, true, !flag_no_color);
    set_include(&cmd, UNGRATEFUL_SOURCE_DIR);

    {
        count = cmd.count;
        set_entry(&cmd, UNGRATEFUL_SOURCE_DIR, "ungrateful", NULL, true);

        if (!nob_cmd_run(&cmd, .dont_reset = true)) {
            nob_log(NOB_ERROR, "---- compilation failed.");
            status = STATUS_FAILED;
        }

        cmd.count = count;
    }

    if (CYNIC) {
        count = cmd.count;
        set_entry(&cmd, UNGRATEFUL_SOURCE_DIR, "cynic", NULL, true);

        if (!nob_cmd_run(&cmd, .dont_reset = true)) {
            nob_log(NOB_ERROR, "---- compilation failed.");
            status = STATUS_FAILED;
        }
        cmd.count = count;
    }
    
    if (VISAGE) {
        count = cmd.count;
        set_entry(&cmd, UNGRATEFUL_SOURCE_DIR, "visage", NULL, true);

        if (!nob_cmd_run(&cmd, .dont_reset = true)) {
            nob_log(NOB_ERROR, "---- compilation failed.");
            status = STATUS_FAILED;
        }
        cmd.count = count;
    }

    // @todo we need to select which libraries to build and use
    if (DISGRACE) {
        count = cmd.count;
        set_entry(&cmd, UNGRATEFUL_SOURCE_DIR, "disgrace", NULL, true);

        if (!nob_cmd_run(&cmd, .dont_reset = true)) {
            nob_log(NOB_ERROR, "---- compilation failed.");
            status = STATUS_FAILED;
        }
        cmd.count = count;
    }

    return status;
}

Status program_compile(void) {
    Nob_Cmd cmd = (Nob_Cmd) { 0 };

    set_cc(&cmd, configuration, false, !flag_no_color);
    set_entry(&cmd, SOURCE_FOLDER, "entry", PROGRAM_NAME, false);

    set_include(&cmd, UNGRATEFUL_SOURCE_DIR);

    set_default_links(&cmd);

    set_obj_link(&cmd, "ungrateful");
    if (CYNIC)    set_obj_link(&cmd, "cynic");
    if (VISAGE)   set_obj_link(&cmd, "visage");
    if (DISGRACE) set_obj_link(&cmd, "disgrace");

    if (!nob_cmd_run(&cmd)) return STATUS_FAILED;

    return STATUS_OK;
}

enum {
    TEST_RESULT_IGNORE,
    TEST_RESULT_TRUE,
    TEST_RESULT_FALSE,
};

bool test_run(Nob_Walk_Entry entry) {
    Nob_Cmd cmd = { 0 };
    Nob_String_View sv;
    Nob_Log_Level ll = nob_minimal_log_level;

    int error_code;
    *entry.action = NOB_WALK_CONT;

    if (entry.type != NOB_FILE_REGULAR) {
        return true;
    }
    
    sv = nob_sv_from_cstr(entry.path);

    if (!nob_sv_chop_prefix(&sv, nob_sv_from_cstr(JSON_TESTS_SOURCE_DIR"/"))) {
        nob_log(NOB_ERROR, "Failed to chop prefix from '%s'", entry.path);
        return false;
    }

    if (!nob_sv_ends_with_cstr(sv, ".json")) {
        return true;
    }


    nob_cmd_append(&cmd, BUILD_FOLDER"/"PROGRAM_NAME);
    nob_cmd_append(&cmd, entry.path);
    nob_cmd_append(&cmd, nob_temp_sprintf(SV_Fmt, SV_Arg(sv)));


    nob_log(NOB_INFO, "TEST: " SV_Fmt, SV_Arg(sv));

    nob_minimal_log_level = NOB_NO_LOGS;

    if (nob_cmd_run(&cmd, .stderr_path = "ignore.stderr", .stdout_path = "ignore.stdout")) {
        nob_minimal_log_level = ll;
        nob_log(NOB_INFO, "      \x1b[0;32mPASS\x1b[0m");
    } else {
        nob_minimal_log_level = ll;
        nob_log(NOB_INFO, "      \x1b[0;31mFAIL\x1b[0m");
    }

    nob_cmd_free(cmd);

    return true;
}

void tests_run(void) {
    nob_log(NOB_INFO, "Running tests...");
    nob_walk_dir(JSON_TESTS_SOURCE_DIR, test_run);
}

int main(int argc, char **argv) {
    b32 run_build, rebuild, asan, optimize, publish;

    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;
    if (!nob_mkdir_if_not_exists(OBJECT_FOLDER)) return 1;

    flag_no_color = HAS_FLAG(FLAG_NO_COLOR);
    rebuild       = HAS_FLAG(FLAG_ALL);
    run_build     = HAS_FLAG(FLAG_RUN);

    if (HAS_FLAG(FLAG_CLANG)) {
        toolchain = TOOLCHAIN_CLANG;
    }

    asan     = HAS_FLAG(FLAG_ASAN);
    optimize = HAS_FLAG(FLAG_OPTIMIZE);
    publish  = HAS_FLAG(FLAG_PUBLISH);

    if (asan && optimize) {
       nob_log(NOB_ERROR, "Can't use ASAN and Optimization at the same time.");
       return 10;
    }

    if (asan && publish) {
       nob_log(NOB_ERROR, "Can't use ASAN and Publish at the same time.");
       return 11;
    }

    if (optimize && publish) {
        nob_log(NOB_ERROR, "Publish implies optimization, use only one of these.");
        return 12;
    }

    if (asan)     configuration = CONFIG_ASAN;
    if (optimize) configuration = CONFIG_OPTIMIZED;
    if (publish)  configuration = CONFIG_PUBLISH;

    rebuild = rebuild || publish;

    if (rebuild && libs_compile()) {
        nob_log(NOB_ERROR, "Failed to compile libs.");
        return 2;
    }

    if (program_compile()) {
        nob_log(NOB_ERROR, "Failed to compile '"PROGRAM_NAME"'.");
        return 1;
    }

    if (run_build) {
        tests_run();
    }

    printf("\n\nDone\n");

    return 0;
}
