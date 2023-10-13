set(TOOL_PROJS
        misc
        gen
        dump
        )


foreach (module ${TOOL_PROJS})
    add_subdirectory(${module})
endforeach ()
