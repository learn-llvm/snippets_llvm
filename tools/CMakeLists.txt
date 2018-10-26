set(SUB_PROJECTS
        DS
        hello_ir
        rtti
        simple
        yaml
        )


foreach (MODULE ${SUB_PROJECTS})
    add_subdirectory(${MODULE})
endforeach ()
