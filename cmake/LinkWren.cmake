function(LinkWren)
  SET(CliSource
    "deps/wren/src/cli/main.c"
    "deps/wren/src/cli/modules.c"
    "deps/wren/src/cli/modules.h"
    "deps/wren/src/cli/path.c"
    "deps/wren/src/cli/path.h"
    "deps/wren/src/cli/stat.h"
    "deps/wren/src/cli/vm.c"
    "deps/wren/src/cli/vm.h"
  )
  SET(ModuleSource
    "deps/wren/src/module/io.h"
    "deps/wren/src/module/io.c"
    "deps/wren/src/module/os.h"
    "deps/wren/src/module/os.c"
    "deps/wren/src/module/repl.h"
    "deps/wren/src/module/repl.c"
    "deps/wren/src/module/scheduler.h"
    "deps/wren/src/module/scheduler.c"
    "deps/wren/src/module/timer.c"
  )
  SET(OptionalSource
    "deps/wren/src/optional/wren_opt_meta.h"
    "deps/wren/src/optional/wren_opt_meta.c"
    "deps/wren/src/optional/wren_opt_random.h"
    "deps/wren/src/optional/wren_opt_random.c"
  )
  SET(VmSource
    "deps/wren/src/vm/wren_common.h"
    "deps/wren/src/vm/wren_compiler.h"
    "deps/wren/src/vm/wren_compiler.c"
    "deps/wren/src/vm/wren_core.h"
    "deps/wren/src/vm/wren_core.c"
    "deps/wren/src/vm/wren_debug.h"
    "deps/wren/src/vm/wren_debug.c"
    "deps/wren/src/vm/wren_opcodes.h"
    "deps/wren/src/vm/wren_primitive.h"
    "deps/wren/src/vm/wren_primitive.c"
    "deps/wren/src/vm/wren_utils.h"
    "deps/wren/src/vm/wren_utils.c"
    "deps/wren/src/vm/wren_value.h"
    "deps/wren/src/vm/wren_value.c"
    "deps/wren/src/vm/wren_vm.h"
    "deps/wren/src/vm/wren_vm.c"
  )

  SET(Sources
    # ${CliSource}
    # ${ModuleSource}
    # ${OptionalSource}
    ${VmSource}
  )

  ADD_LIBRARY(wren ${Sources})
  TARGET_INCLUDE_DIRECTORIES(wren PUBLIC "deps/wren/src/include")
endfunction()