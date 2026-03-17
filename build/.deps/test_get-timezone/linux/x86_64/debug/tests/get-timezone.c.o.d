{
    values = {
        "/usr/sbin/clang",
        {
            "-Qunused-arguments",
            "-m64",
            "-std=c99",
            "-Ibuild",
            "-Isrc",
            "-DWITH_GZFILEOP",
            "-isystem",
            "/usr/include/gdk-pixbuf-2.0",
            "-isystem",
            "/usr/include/glycin-2",
            "-isystem",
            "/usr/include/freetype2",
            "-isystem",
            "/usr/include/libpng16",
            "-isystem",
            "/usr/include/harfbuzz",
            "-isystem",
            "/usr/include/libxml2",
            "-isystem",
            "/usr/include/glib-2.0",
            "-isystem",
            "/usr/lib64/glib-2.0/include",
            "-isystem",
            "/usr/include/libmount",
            "-isystem",
            "/usr/include/blkid",
            "-isystem",
            "/usr/include/sysprof-6",
            "-pthread"
        }
    },
    depfiles_format = "gcc",
    files = {
        "tests/get-timezone.c"
    },
    depfiles = "build/.objs/test_get-timezone/linux/x86_64/debug/tests/__cpp_get-timezone.c.c:   tests/get-timezone.c src/itdb.h build/config.h src/itdb_device.h   src/itdb_sysinfo_extended_parser.h\
"
}