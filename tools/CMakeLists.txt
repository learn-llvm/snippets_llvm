set(SUB_PROJECTS
        misc
        gen
        dump
        )


foreach (MODULE ${SUB_PROJECTS})
    add_subdirectory(${MODULE})
endforeach ()
