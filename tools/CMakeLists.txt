set(TOOL_PROJS
        # misc
        dump
        )


foreach (module ${TOOL_PROJS})
    add_subdirectory(${module})
endforeach ()
