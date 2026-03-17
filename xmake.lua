toolchain("clang")
    set_kind("standalone")
    set_toolset("cc", "clang")
    set_toolset("ld", "clang")
toolchain_end()

set_project("libgpod")
set_version("0.8.0")
set_languages("c99")
set_toolchains("clang")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "." })

option("gdkpixbuf")
    set_default(true) -- default to ON if it can be found
    set_showmenu(true)
    set_description("Build with gdk-pixbuf support")
option_end()

option("libimobiledevice")
    set_default(true)
    set_showmenu(true)
    set_description("Build with libimobiledevice support")
option_end()

-- Dependencies via pkg-config (system=true = use distro packages)
add_requires("pkgconfig::glib-2.0",      { alias = "glib",      system = true })
add_requires("pkgconfig::gmodule-2.0",   { alias = "gmodule",   system = true })
add_requires("pkgconfig::gobject-2.0",   { alias = "gobject",   system = true })
add_requires("pkgconfig::libplist-2.0",  { alias = "libplist",  system = true, optional = true })
add_requires("pkgconfig::sqlite3",       { alias = "sqlite3",   system = true })
add_requires("pkgconfig::zlib",          { alias = "zlib",      system = true })

-- Optional deps (only used if config option is enabled)
add_requires("pkgconfig::gdk-pixbuf-2.0",
             { alias = "gdkpixbuf", system = true, optional = true })
add_requires("pkgconfig::libimobiledevice-1.0",
             { alias = "libimobiledevice", system = true, optional = true })

set_configvar("HAVE_UNISTD_H", 1)
set_configvar("LIBGPOD_BLOB_DIR", "$(installdir)/libgpod")
set_configvar("VERSION", "0.8.0")

-- CMake-style install prefix used by libgpod-1.0.pc.in
set_configvar("CMAKE_INSTALL_PREFIX", "$(installdir)")
set_configvar("CMAKE_INSTALL_FULL_LIBDIR", "$(installdir)/lib")
set_configvar("CMAKE_INSTALL_FULL_INCLUDEDIR", "$(installdir)/include")
set_configvar("CMAKE_INSTALL_FULL_BINDIR", "$(installdir)/bin") -- harmless if unused

if has_config("gdkpixbuf") then
    set_configvar("HAVE_GDKPIXBUF", 1)
    set_configvar("GDKPIXBUF_REQ", "gdk-pixbuf-2.0")
else
    set_configvar("GDKPIXBUF_REQ", "")
end

if has_config("libimobiledevice") then
    set_configvar("HAVE_LIBIMOBILEDEVICE", 1)
    set_configvar("LIBIMOBILEDEVICE_REQ", "libimobiledevice-1.0")
else
    set_configvar("LIBIMOBILEDEVICE_REQ", "")
end

set_configvar("HAVE_G_MEMDUP2", 1)

-- Generate config.h and libgpod-1.0.pc from their .in templates
-- Make sure config.h.in and libgpod-1.0.pc.in live next to xmake.lua
add_configfiles("config.h.in", {
    pattern  = "@(.-)@",      -- CMake-style @VAR@ substitution
    filename = "config.h"
})
add_configfiles("libgpod-1.0.pc.in", {
    pattern  = "@(.-)@",
    filename = "libgpod-1.0.pc"
})

-- List each test source file
local test_sources = {
    "tests/get-timezone.c",
    "tests/itdb_main.c",
    "tests/test-covers.c",
    "tests/test-fw-id.c",
    "tests/test-ls.c",
    "tests/test-write-covers.c",
    "tests/test-sysinfo-extended-parsing.c",
    "tests/test-init-ipod.c",
    "tests/test-photos.c",
    "tests/test-cp.cc",
    "tests/test-rebuild-db.cc",
}

for _, src in ipairs(test_sources) do
    -- Derive a nice target name: test_<basename>
    local name = path.basename(src)          -- e.g. "test-ls"
    local tname = "test_" .. name:gsub("%.", "_")

    target(tname)
        set_kind("binary")
        set_group("tests")                   -- groups them in xmake’s UI

        add_files(src)

        -- Use the same headers as the lib
        add_includedirs("$(builddir)", "src")

        -- Link against libgpod and its deps
        add_deps("gpod")
        add_packages("glib", "gmodule", "gobject", "libplist", "sqlite3", "zlib")

        if has_config("gdkpixbuf") then
            add_packages("gdkpixbuf")
        end

        if has_config("libimobiledevice") then
            add_packages("libimobiledevice")
        end
end

-- libgpod itself
target("gpod")
    set_kind("shared")
    set_basename("gpod")        -- libgpod.so
    set_version("0.8.0")

    -- source files
    add_files(
        "src/itdb_thumb.c",
        "src/itdb_track.c",
        "src/itdb_zlib.c",
        "src/pixmaps.c",
        "src/itdb_itunesdb.c",
        "src/db-parse-context.c",
        "src/db-artwork-parser.c",
        "src/db-artwork-writer.c",
        "src/itdb_artwork.c",
        "src/itdb_iphone.c",
        "src/itdb_playlist.c",
        "src/rijndael.c",
        "src/ithumb-writer.c",
        "src/itdb_tzinfo.c",
        "src/itdb_photoalbum.c",
        "src/itdb_hash58.c",
        "src/db-image-parser.c",
        "src/itdb_hash72.c",
        "src/itdb_sqlite.c",
        "src/itdb_device.c",
        "src/itdb_sysinfo_extended_parser.c",
        "src/itdb_chapterdata.c",
        "src/db-artwork-debug.c",
        "src/itdb_hashAB.c",
        "src/itdb_plist.c"
    )

    -- include dirs: generated config.h + src/
    add_includedirs("$(builddir)", "src")

    -- core deps
    add_packages("glib", "gmodule", "gobject", "libplist", "sqlite3", "zlib")

    -- optional deps
    if has_config("gdkpixbuf") then
        add_packages("gdkpixbuf")
        add_defines("HAVE_GDKPIXBUF=1")
    end

    if has_config("libimobiledevice") then
        add_packages("libimobiledevice")
        add_defines("HAVE_LIBIMOBILEDEVICE=1")
    end

    -- defines from CMake
    add_defines('GETTEXT_PACKAGE="libgpod"')

    if is_plat("linux") then
        add_defines("HAVE_STRUCT_TM_TM_GMTOFF")
    end

    -- Install the shared lib (xmake will put it under lib/ by default)
    set_installdir(".")

    -- Install public header like CMake did
    add_installfiles("src/itdb.h", {
        prefixdir = "include/gpod-1.0/gpod"
    })

    -- Install pkg-config file
    add_installfiles("$(builddir)/libgpod-1.0.pc", {
        prefixdir = "lib/pkgconfig"
    })
