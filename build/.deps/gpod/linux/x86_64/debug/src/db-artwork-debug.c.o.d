{
    values = {
        "/usr/sbin/clang",
        {
            "-Qunused-arguments",
            "-m64",
            "-fPIC",
            "-std=c99",
            "-Ibuild",
            "-Isrc",
            "-DHAVE_GDKPIXBUF=1",
            "-DHAVE_LIBIMOBILEDEVICE=1",
            "-DGETTEXT_PACKAGE=\"libgpod\"",
            "-DHAVE_STRUCT_TM_TM_GMTOFF",
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
    files = {
        "src/db-artwork-debug.c"
    },
    depfiles_format = "gcc",
    depfiles = "build/.objs/gpod/linux/x86_64/debug/src/__cpp_db-artwork-debug.c.c:   src/db-artwork-debug.c src/db-artwork-debug.h src/db-itunes-parser.h\
"
}