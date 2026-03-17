{
    values = {
        "/usr/sbin/clang++",
        {
            "-m64",
            "-Lbuild/linux/x86_64/debug",
            "-Wl,-rpath=$ORIGIN",
            "-lgmodule-2.0",
            "-lsqlite3",
            "-lz",
            "-lgdk_pixbuf-2.0",
            "-lgobject-2.0",
            "-lglib-2.0",
            "-limobiledevice-1.0",
            "-lplist-2.0",
            "-lgpod",
            "-Wl,--export-dynamic",
            "-pthread"
        }
    },
    files = {
        "build/.objs/test_test-ls/linux/x86_64/debug/tests/test-ls.c.o"
    }
}