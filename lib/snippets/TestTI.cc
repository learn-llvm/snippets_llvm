#define DEBUG_TYPE "TestTI"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "Version.hh"

#include "llvm/Pass.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include <string>

#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Target/TargetMachine.h"

#include "Logging.hh"

using namespace llvm;

/// use static ***TypeName to get name
/// ***Name whose return value is StringRef is used for set internally

namespace {

const char* StandardNames[LibFunc::NumLibFuncs] = {
    "_IO_getc",             "_IO_putc",            "_ZdaPv",
    "_ZdaPvRKSt9nothrow_t", "_ZdlPv",              "_ZdlPvRKSt9nothrow_t",
    "_Znaj",                "_ZnajRKSt9nothrow_t", "_Znam",
    "_ZnamRKSt9nothrow_t",  "_Znwj",               "_ZnwjRKSt9nothrow_t",
    "_Znwm",                "_ZnwmRKSt9nothrow_t", "__cospi",
    "__cospif",             "__cxa_atexit",        "__cxa_guard_abort",
    "__cxa_guard_acquire",  "__cxa_guard_release", "__isoc99_scanf",
    "__isoc99_sscanf",      "__memcpy_chk",        "__sincospi_stret",
    "__sincospi_stretf",    "__sinpi",             "__sinpif",
    "__sqrt_finite",        "__sqrtf_finite",      "__sqrtl_finite",
    "__strdup",             "__strndup",           "__strtok_r",
    "abs",                  "access",              "acos",
    "acosf",                "acosh",               "acoshf",
    "acoshl",               "acosl",               "asin",
    "asinf",                "asinh",               "asinhf",
    "asinhl",               "asinl",               "atan",
    "atan2",                "atan2f",              "atan2l",
    "atanf",                "atanh",               "atanhf",
    "atanhl",               "atanl",               "atof",
    "atoi",                 "atol",                "atoll",
    "bcmp",                 "bcopy",               "bzero",
    "calloc",               "cbrt",                "cbrtf",
    "cbrtl",                "ceil",                "ceilf",
    "ceill",                "chmod",               "chown",
    "clearerr",             "closedir",            "copysign",
    "copysignf",            "copysignl",           "cos",
    "cosf",                 "cosh",                "coshf",
    "coshl",                "cosl",                "ctermid",
    "exp",                  "exp10",               "exp10f",
    "exp10l",               "exp2",                "exp2f",
    "exp2l",                "expf",                "expl",
    "expm1",                "expm1f",              "expm1l",
    "fabs",                 "fabsf",               "fabsl",
    "fclose",               "fdopen",              "feof",
    "ferror",               "fflush",              "ffs",
    "ffsl",                 "ffsll",               "fgetc",
    "fgetpos",              "fgets",               "fileno",
    "fiprintf",             "flockfile",           "floor",
    "floorf",               "floorl",              "fmod",
    "fmodf",                "fmodl",               "fopen",
    "fopen64",              "fprintf",             "fputc",
    "fputs",                "fread",               "free",
    "frexp",                "frexpf",              "frexpl",
    "fscanf",               "fseek",               "fseeko",
    "fseeko64",             "fsetpos",             "fstat",
    "fstat64",              "fstatvfs",            "fstatvfs64",
    "ftell",                "ftello",              "ftello64",
    "ftrylockfile",         "funlockfile",         "fwrite",
    "getc",                 "getc_unlocked",       "getchar",
    "getenv",               "getitimer",           "getlogin_r",
    "getpwnam",             "gets",                "gettimeofday",
    "htonl",                "htons",               "iprintf",
    "isascii",              "isdigit",             "labs",
    "lchown",               "llabs",               "log",
    "log10",                "log10f",              "log10l",
    "log1p",                "log1pf",              "log1pl",
    "log2",                 "log2f",               "log2l",
    "logb",                 "logbf",               "logbl",
    "logf",                 "logl",                "lstat",
    "lstat64",              "malloc",              "memalign",
    "memccpy",              "memchr",              "memcmp",
    "memcpy",               "memmove",             "memrchr",
    "memset",               "memset_pattern16",    "mkdir",
    "mktime",               "modf",                "modff",
    "modfl",                "nearbyint",           "nearbyintf",
    "nearbyintl",           "ntohl",               "ntohs",
    "open",                 "open64",              "opendir",
    "pclose",               "perror",              "popen",
    "posix_memalign",       "pow",                 "powf",
    "powl",                 "pread",               "printf",
    "putc",                 "putchar",             "puts",
    "pwrite",               "qsort",               "read",
    "readlink",             "realloc",             "reallocf",
    "realpath",             "remove",              "rename",
    "rewind",               "rint",                "rintf",
    "rintl",                "rmdir",               "round",
    "roundf",               "roundl",              "scanf",
    "setbuf",               "setitimer",           "setvbuf",
    "sin",                  "sinf",                "sinh",
    "sinhf",                "sinhl",               "sinl",
    "siprintf",             "snprintf",            "sprintf",
    "sqrt",                 "sqrtf",               "sqrtl",
    "sscanf",               "stat",                "stat64",
    "statvfs",              "statvfs64",           "stpcpy",
    "stpncpy",              "strcasecmp",          "strcat",
    "strchr",               "strcmp",              "strcoll",
    "strcpy",               "strcspn",             "strdup",
    "strlen",               "strncasecmp",         "strncat",
    "strncmp",              "strncpy",             "strndup",
    "strnlen",              "strpbrk",             "strrchr",
    "strspn",               "strstr",              "strtod",
    "strtof",               "strtok",              "strtok_r",
    "strtol",               "strtold",             "strtoll",
    "strtoul",              "strtoull",            "strxfrm",
    "system",               "tan",                 "tanf",
    "tanh",                 "tanhf",               "tanhl",
    "tanl",                 "times",               "tmpfile",
    "tmpfile64",            "toascii",             "trunc",
    "truncf",               "truncl",              "uname",
    "ungetc",               "unlink",              "unsetenv",
    "utime",                "utimes",              "valloc",
    "vfprintf",             "vfscanf",             "vprintf",
    "vscanf",               "vsnprintf",           "vsprintf",
    "vsscanf",              "write"};
}

namespace {

struct TestTI final : public ModulePass {
  static char ID;

  TestTI() : ModulePass(ID) {}

  void test_TI(Module& M) {
    /// TODO seems buggy
    errs() << Triple::normalize("gnu-linux-x86_64") << "\n";
    std::string TT = M.getTargetTriple();
    Triple triple(TT);
    unsigned major, minor, micro;
    triple.getOSVersion(major, minor, micro);
    triple.setVendorName("pc");
    errs() << format("OSTypeName=%s\tvendorTypeName=%s\tversion=%u.%u.%u\n",
                     Triple::getOSTypeName(triple.getOS()),
                     Triple::getVendorTypeName(triple.getVendor()), major,
                     minor, micro);
    errs() << format("archTypeName=%s\tarchPrefix=%s\tenvironmentTypeName=%s\n",
                     Triple::getArchTypeName(triple.getArch()),
                     Triple::getArchTypePrefix(triple.getArch()),
                     Triple::getEnvironmentTypeName(triple.getEnvironment()));

    if (triple.isArch64Bit()) {
      errs() << "32bit variant: " << triple.get32BitArchVariant().str() << "\n";
    } else {
      errs() << "64bit variant: " << triple.get64BitArchVariant().str() << "\n";
    }

    triple.setArch(Triple::getArchTypeForLLVMName("amdil"));
    errs() << triple.getTriple() << "\n";

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
#else
    /// only for apple
    triple.setVendorName("apple");
    errs() << "archNameForAsm: " << triple.getArchNameForAssembler() << "\n";
#endif
  }

  bool runOnModule(Module& M) {
    /// test_TI(M);
    TargetLibraryInfo TLI(Triple(M.getTargetTriple()));
    LibFunc::Func libFunc;
    for (unsigned i = 0u; i < LibFunc::NumLibFuncs; ++i) {
      auto* funcName = StandardNames[i];
      TLI.getLibFunc(funcName, libFunc);
      errs() << format("%3d\t%-25s has=%d hasOptimizedCodeGen=%d\n", i,
                       funcName, TLI.has(libFunc),
                       TLI.hasOptimizedCodeGen(libFunc));
    }
    return false;
  }

  virtual void getAnalysisUsage(AnalysisUsage& AU) const {
    AU.setPreservesAll();
  }
};

char TestTI::ID = 0;
static RegisterPass<TestTI> X("TestTI", "TestTI", true, true);
}
