idf_component_register(SRCS "hello_world.c"
                    INCLUDE_DIRS "."
                    REQUIRES bt 
                        nvs_flash 
                        driver
                    )
