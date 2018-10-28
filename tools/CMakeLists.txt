set(TOOL_PROJS
        misc
        gen
        dump
        )


foreach (MODULE ${TOOL_PROJS})
    add_subdirectory(${MODULE})
endforeach ()
