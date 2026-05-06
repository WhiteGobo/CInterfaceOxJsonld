corrosion_import_crate(MANIFEST_PATH Cargo.toml)
target_sources(rusttools INTERFACE
        FILE_SET HEADERS
        BASE_DIRS ./
        FILES
        rusttools.h
)

