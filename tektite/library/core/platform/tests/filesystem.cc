#include <catch2/catch_all.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <rainy/core/layer.hpp>
#include <string>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef _T
#if RAINY_USING_WINDOWS
#define _T(x) L##x
#else
#define _T(x) x
#endif
#endif

namespace layer = rainy::core::layer;
using native_char = rainy::core::native_char;
using native_string = std::basic_string<native_char>;
using native_czstring = rainy::core::native_czstring;
using native_cstring = rainy::core::native_cstring;

#if RAINY_USING_WINDOWS
#include <windows.h>

// In swprintf (wide CRT), %s expects narrow char* — we need %ls for wchar_t*
#define PRI_NATIVE_S L"%ls"
// Native path separator: \ on Windows, / on POSIX
#define PRI_NATIVE_SEP L"\\"
#define NATIVE_PRINTF(buf, size, fmt, ...) std::swprintf((buf), (size), (fmt), __VA_ARGS__)
inline static std::size_t native_strlen(const native_char *s) {
    return std::wcslen(s);
}
inline static int native_strcmp(const native_char *a, const native_char *b) {
    return std::wcscmp(a, b);
}
inline static FILE *native_fopen(const native_char *path, const native_char *mode) {
    return _wfopen(path, mode);
}
#else
// In snprintf (narrow CRT), %s is correct for char*
#define PRI_NATIVE_S "%s"
// Native path separator: \ on Windows, / on POSIX
#define PRI_NATIVE_SEP "/"
#define NATIVE_PRINTF(buf, size, fmt, ...) std::snprintf((buf), (size), (fmt), __VA_ARGS__)
inline static std::size_t native_strlen(const native_char *s) {
    return std::strlen(s);
}
inline static int native_strcmp(const native_char *a, const native_char *b) {
    return std::strcmp(a, b);
}
inline static FILE *native_fopen(const native_char *path, const native_char *mode) {
    return fopen(path, mode);
}
#endif

struct TempDir {
    native_char root[PATH_MAX];
    native_char sub[PATH_MAX];
    native_char file_a[PATH_MAX];
    native_char file_b[PATH_MAX];
    native_char link_ab[PATH_MAX];
    native_char empty_dir[PATH_MAX];

    TempDir() {
        native_char tmp[PATH_MAX];
        const ssize_t tmp_len = layer::temp_directory_path_native(tmp, PATH_MAX);
        REQUIRE(tmp_len > 0);

        // Strip trailing separator from tmp so our format string controls the separator.
        if (tmp_len > 0 && (tmp[tmp_len - 1] == L'/' || tmp[tmp_len - 1] == L'\\')) {
            tmp[tmp_len - 1] = L'\0';
        }

        int n = NATIVE_PRINTF(root, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("rainy-fs-test-XXXXXX"), tmp);
        REQUIRE(n > 0);
#if RAINY_USING_WINDOWS
        n = NATIVE_PRINTF(root, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("rainy-fs-test-%lu"), tmp,
                          static_cast<unsigned long>(::GetCurrentProcessId()));
        REQUIRE(n > 0);
        // Clean up any stale directory from a previous test run (same PID).
        (void) layer::remove_all_native(root);
        if (!CreateDirectoryW(root, nullptr)) {
            FAIL("Failed to create root directory");
        }
#else
        REQUIRE(::mkdtemp(root) != nullptr);
#endif

        NATIVE_PRINTF(sub, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("canon"), root);
        NATIVE_PRINTF(file_a, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("canon") PRI_NATIVE_SEP _T("file_a.txt"), root);
        NATIVE_PRINTF(file_b, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("canon") PRI_NATIVE_SEP _T("file_b.txt"), root);
        NATIVE_PRINTF(link_ab, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("canon") PRI_NATIVE_SEP _T("link_ab"), root);
        NATIVE_PRINTF(empty_dir, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("empty_dir"), root);

        REQUIRE(layer::create_directories_native(sub));

        {
            FILE *f = native_fopen(file_a, _T("w"));
            REQUIRE(f != nullptr);
            const char content[] = "AAAA";
            std::fwrite(content, 1, sizeof(content) - 1, f);
            std::fclose(f);
        }
        {
            FILE *f = native_fopen(file_b, _T("w"));
            REQUIRE(f != nullptr);
            const char content[] = "BBBB";
            std::fwrite(content, 1, sizeof(content) - 1, f);
            std::fclose(f);
        }
        REQUIRE(layer::create_directory_native(empty_dir));
    }

    ~TempDir() {
        layer::remove_all_native(root);
    }

    TempDir(const TempDir &) = delete;
    TempDir &operator=(const TempDir &) = delete;
};

TEST_CASE("path::absolute_native", "[filesystem][path]") {
    TempDir d;

    SECTION("absolute of absolute path returns as-is") {
        native_char buf[PATH_MAX];
        const ssize_t len = layer::absolute_native(d.sub, buf, PATH_MAX);
        REQUIRE(len > 0);
        buf[len] = '\0';
        REQUIRE(native_strcmp(buf, d.sub) == 0);
    }

    SECTION("absolute of relative path") {
        native_char cwd[PATH_MAX];
        REQUIRE(layer::current_path_native(cwd, PATH_MAX) > 0);

        native_char buf[PATH_MAX];
        const ssize_t len = layer::absolute_native(_T("."), buf, PATH_MAX);
        REQUIRE(len > 0);
        buf[len] = '\0';
#if RAINY_USING_WINDOWS
        REQUIRE(native_strcmp(buf, cwd) == 0);
#else
        REQUIRE(Catch::startsWith(buf, cwd)); 
#endif
    }

    SECTION("absolute with buffer too small") {
        native_char buf[4];
        const ssize_t len = layer::absolute_native(d.sub, buf, 2);
        REQUIRE(len > 0);
    }
}

TEST_CASE("path::canonical_native", "[filesystem][path]") {
    TempDir d;

    SECTION("canonical of existing file") {
        native_char buf[PATH_MAX];
        const ssize_t len = layer::canonical_native(d.file_a, buf, PATH_MAX);
        REQUIRE(len > 0);
        buf[len] = '\0';
        REQUIRE(native_strcmp(buf, d.file_a) == 0);
    }

    SECTION("canonical of non-existent fails") {
        native_char buf[PATH_MAX];
        native_char missing[PATH_MAX];
        NATIVE_PRINTF(missing, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("nonexistent"), d.sub);
        REQUIRE(layer::canonical_native(missing, buf, PATH_MAX) < 0);
    }
}

TEST_CASE("path::weakly_canonical_native", "[filesystem][path]") {
    TempDir d;

    SECTION("existing path") {
        native_char buf[PATH_MAX];
        REQUIRE(layer::weakly_canonical_native(d.file_a, buf, PATH_MAX) > 0);
    }

    SECTION("non-existent suffix") {
        native_char buf[PATH_MAX];
        native_char mixed[PATH_MAX];
        NATIVE_PRINTF(mixed, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("nonexistent_subdir"), d.sub);
        REQUIRE(layer::weakly_canonical_native(mixed, buf, PATH_MAX) > 0);
    }
}

TEST_CASE("path::relative_native", "[filesystem][path]") {
    TempDir d;

    SECTION("self to self is .") {
        native_char buf[PATH_MAX];
        const ssize_t len = layer::relative_native(d.file_a, d.file_a, buf, PATH_MAX);
        REQUIRE(len > 0);
        buf[len] = '\0';
        REQUIRE(native_strcmp(buf, _T(".")) == 0);
    }

    SECTION("against CWD") {
        native_char cwd[PATH_MAX];
        REQUIRE(layer::current_path_native(cwd, PATH_MAX) > 0);

        native_char buf[PATH_MAX];
        REQUIRE(layer::relative_native(d.sub, buf, PATH_MAX) > 0);
    }
}

TEST_CASE("path::proximate_native", "[filesystem][path]") {
    TempDir d;

    SECTION("returns relative when possible") {
        native_char buf[PATH_MAX];
        REQUIRE(layer::proximate_native(d.file_a, buf, PATH_MAX) > 0);
    }

    SECTION("with explicit base") {
        native_char buf[PATH_MAX];
        REQUIRE(layer::proximate_native(d.file_a, d.sub, buf, PATH_MAX) > 0);
    }
}

TEST_CASE("path::current_path_native", "[filesystem][cwd]") {
    TempDir d;

    SECTION("get returns non-empty") {
        native_char buf[PATH_MAX];
        REQUIRE(layer::current_path_native(buf, PATH_MAX) > 0);
    }

    SECTION("set then get matches") {
        native_char saved[PATH_MAX];
        REQUIRE(layer::current_path_native(saved, PATH_MAX) > 0);

        layer::current_path_native(d.sub);
        native_char after[PATH_MAX];
        const ssize_t len = layer::current_path_native(after, PATH_MAX);
        REQUIRE(len > 0);
        after[len] = '\0';
        REQUIRE(native_strcmp(after, d.sub) == 0);

        layer::current_path_native(saved);
    }
}

TEST_CASE("status::exists_native", "[filesystem][status]") {
    TempDir d;
    REQUIRE(layer::exists_native(d.file_a));

    native_char missing[PATH_MAX];
    NATIVE_PRINTF(missing, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("does_not_exist"), d.sub);
    REQUIRE_FALSE(layer::exists_native(missing));
}

TEST_CASE("status::is_*_native", "[filesystem][status]") {
    TempDir d;

    SECTION("is_regular_file_native") {
        REQUIRE(layer::is_regular_file_native(d.file_a));
        REQUIRE_FALSE(layer::is_regular_file_native(d.empty_dir));
    }

    SECTION("is_directory_native") {
        REQUIRE(layer::is_directory_native(d.sub));
        REQUIRE_FALSE(layer::is_directory_native(d.file_a));
    }

    SECTION("is_block_file_native") {
        REQUIRE_FALSE(layer::is_block_file_native(d.file_a));
    }

    SECTION("is_character_file_native") {
        REQUIRE_FALSE(layer::is_character_file_native(d.file_a));
    }

    SECTION("is_fifo_native") {
        REQUIRE_FALSE(layer::is_fifo_native(d.file_a));
    }

    SECTION("is_socket_native") {
        REQUIRE_FALSE(layer::is_socket_native(d.file_a));
    }

    SECTION("is_symlink_native") {
        REQUIRE_FALSE(layer::is_symlink_native(d.file_a));
    }

    SECTION("is_other_native") {
        REQUIRE_FALSE(layer::is_other_native(d.file_a));
    }

    SECTION("is_empty_native") {
        REQUIRE_FALSE(layer::is_empty_native(d.file_a));
        REQUIRE(layer::is_empty_native(d.empty_dir));
    }
}

TEST_CASE("status::status_native / symlink_status_native", "[filesystem][status]") {
    TempDir d;

    SECTION("status_native of regular file") {
        const auto st = layer::status_native(d.file_a);
        REQUIRE(st.type == layer::file_type::regular);
        REQUIRE(st.permissions != layer::perms::unknown);
    }

    SECTION("status_native of directory") {
        const auto st = layer::status_native(d.sub);
        CHECK(st.type == layer::file_type::directory);
    }

    SECTION("status_native of non-existent") {
        native_char missing[PATH_MAX];
        NATIVE_PRINTF(missing, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("ghost"), d.sub);
        const auto st = layer::status_native(missing);
        CHECK(st.type == layer::file_type::not_found);
    }

    SECTION("symlink_status_native on regular file") {
        const auto st = layer::symlink_status_native(d.file_a);
        REQUIRE(st.type == layer::file_type::regular);
    }
}

TEST_CASE("status::equivalent_native", "[filesystem][status]") {
    TempDir d;

    SECTION("same file") {
        REQUIRE(layer::equivalent_native(d.file_a, d.file_a));
    }

    SECTION("different files") {
        REQUIRE_FALSE(layer::equivalent_native(d.file_a, d.file_b));
    }

    SECTION("non-existent path") {
        native_char missing[PATH_MAX];
        NATIVE_PRINTF(missing, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("nonexistent"), d.sub);
        REQUIRE_FALSE(layer::equivalent_native(d.file_a, missing));
    }
}

TEST_CASE("file::file_size_native", "[filesystem][file]") {
    TempDir d;

    SECTION("existing via out-parameter") {
        std::uintmax_t sz{};
        REQUIRE(layer::file_size_native(d.file_a, &sz));
        REQUIRE(sz == 4);
    }

    SECTION("non-existent via out-parameter") {
        std::uintmax_t sz{999};
        native_char missing[PATH_MAX];
        NATIVE_PRINTF(missing, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("missing"), d.sub);
        REQUIRE_FALSE(layer::file_size_native(missing, &sz));
    }

    SECTION("return-value overload") {
        REQUIRE(layer::file_size_native(d.file_a) == 4);
    }

    SECTION("return-value on non-existent") {
        native_char missing[PATH_MAX];
        NATIVE_PRINTF(missing, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("missing"), d.sub);
        REQUIRE(layer::file_size_native(missing) == static_cast<std::uintmax_t>(-1));
    }
}

TEST_CASE("file::hard_link_count_native", "[filesystem][file]") {
    TempDir d;

    SECTION("existing via out-parameter") {
        std::uintmax_t cnt{};
        REQUIRE(layer::hard_link_count_native(d.file_a, &cnt));
        REQUIRE(cnt >= 1);
    }

    SECTION("non-existent via out-parameter") {
        std::uintmax_t cnt{};
        native_char missing[PATH_MAX];
        NATIVE_PRINTF(missing, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("missing"), d.sub);
        REQUIRE_FALSE(layer::hard_link_count_native(missing, &cnt));
    }

    SECTION("return-value overload") {
        REQUIRE(layer::hard_link_count_native(d.file_a) >= 1);
    }

    SECTION("return-value on non-existent") {
        native_char missing[PATH_MAX];
        NATIVE_PRINTF(missing, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("missing"), d.sub);
        REQUIRE(layer::hard_link_count_native(missing) == static_cast<std::uintmax_t>(-1));
    }
}

TEST_CASE("file::last_write_time_native", "[filesystem][file]") {
    TempDir d;

    SECTION("existing via out-parameter") {
        std::time_t t{};
        REQUIRE(layer::last_write_time_native(d.file_a, &t));
        REQUIRE(t > 0);
    }

    SECTION("non-existent via out-parameter") {
        std::time_t t{};
        native_char missing[PATH_MAX];
        NATIVE_PRINTF(missing, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("missing"), d.sub);
        REQUIRE_FALSE(layer::last_write_time_native(missing, &t));
    }

    SECTION("return-value overload") {
        REQUIRE(layer::last_write_time_native(d.file_a) > 0);
    }

    SECTION("set then get") {
        layer::last_write_time_native(d.file_a, 1000000000);
        REQUIRE(layer::last_write_time_native(d.file_a) > 0);
    }
}

TEST_CASE("dir::create_directory_native", "[filesystem][dir]") {
    TempDir d;

    SECTION("create single directory") {
        native_char single[PATH_MAX];
        NATIVE_PRINTF(single, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("single_dir"), d.root);
        REQUIRE(layer::create_directory_native(single));
        REQUIRE_FALSE(layer::create_directory_native(single));
        REQUIRE(layer::is_directory_native(single));
    }

    SECTION("with existing_p") {
        native_char single[PATH_MAX];
        NATIVE_PRINTF(single, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("copy_perm_dir"), d.root);
        REQUIRE(layer::create_directory_native(single, d.sub));
        REQUIRE(layer::is_directory_native(single));
    }

    SECTION("empty path fails") {
        REQUIRE_FALSE(layer::create_directory_native(_T("")));
    }
}

TEST_CASE("dir::create_directories_native", "[filesystem][dir]") {
    TempDir d;

    SECTION("deep hierarchy") {
        native_char deep[PATH_MAX];
        NATIVE_PRINTF(deep, PATH_MAX,
                      PRI_NATIVE_S PRI_NATIVE_SEP _T("x") PRI_NATIVE_SEP _T("y") PRI_NATIVE_SEP _T("z") PRI_NATIVE_SEP
                                                  _T("w") PRI_NATIVE_SEP _T("v"),
                      d.root);
        REQUIRE(layer::create_directories_native(deep));
        REQUIRE(layer::is_directory_native(deep));
        REQUIRE_FALSE(layer::create_directories_native(deep));
    }
}

TEST_CASE("dir::remove_native", "[filesystem][dir]") {
    TempDir d;

    SECTION("existing file") {
        native_char tmpfile[PATH_MAX];
        NATIVE_PRINTF(tmpfile, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("to_remove.txt"), d.sub);
        {
            FILE *f = native_fopen(tmpfile, _T("w"));
            REQUIRE(f);
            std::fclose(f);
        }
        REQUIRE(layer::remove_native(tmpfile));
        REQUIRE_FALSE(layer::exists_native(tmpfile));
    }

    SECTION("non-existent") {
        native_char missing[PATH_MAX];
        NATIVE_PRINTF(missing, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("ghost"), d.sub);
        REQUIRE_FALSE(layer::remove_native(missing));
    }
}

TEST_CASE("dir::remove_all_native", "[filesystem][dir]") {
    TempDir d;

    SECTION("directory with contents") {
        native_char parent[PATH_MAX];
        NATIVE_PRINTF(parent, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("rm_all_parent"), d.root);
        native_char child[PATH_MAX];
        NATIVE_PRINTF(child, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("a") PRI_NATIVE_SEP _T("b") PRI_NATIVE_SEP _T("c"), parent);
        REQUIRE(layer::create_directories_native(child));
        native_char leaf[PATH_MAX];
        NATIVE_PRINTF(leaf, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("leaf.txt"), child);
        {
            FILE *f = native_fopen(leaf, _T("w"));
            REQUIRE(f);
            std::fclose(f);
        }

        const std::uintmax_t removed = layer::remove_all_native(parent);
        REQUIRE(removed >= 4);
        REQUIRE_FALSE(layer::exists_native(parent));
    }

    SECTION("non-existent") {
        native_char missing[PATH_MAX];
        NATIVE_PRINTF(missing, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("ghost_dir"), d.root);
        REQUIRE(layer::remove_all_native(missing) == 0);
    }
}

TEST_CASE("file::copy_file_native", "[filesystem][file]") {
    TempDir d;

    SECTION("copy to new path") {
        native_char dest[PATH_MAX];
        NATIVE_PRINTF(dest, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("copy_of_a.txt"), d.sub);
        REQUIRE(layer::copy_file_native(d.file_a, dest));
        REQUIRE(layer::is_regular_file_native(dest));
        REQUIRE(layer::file_size_native(dest) == 4);
    }

    SECTION("skip_existing when target exists") {
        REQUIRE_FALSE(layer::copy_file_native(d.file_a, d.file_b, layer::copy_options::skip_existing));
    }

    SECTION("overwrite_existing") {
        REQUIRE(layer::copy_file_native(d.file_a, d.file_b, layer::copy_options::overwrite_existing));
    }
}

TEST_CASE("file::copy_native", "[filesystem][file]") {
    TempDir d;

    SECTION("without recursive") {
        native_char dest[PATH_MAX];
        NATIVE_PRINTF(dest, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("copied_dir_bare"), d.root);
        layer::copy_native(d.sub, dest);
        CHECK(layer::exists_native(dest));
    }

    SECTION("with recursive + skip_symlinks") {
        native_char dest[PATH_MAX];
        NATIVE_PRINTF(dest, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("copied_dir_recursive"), d.root);
        layer::copy_native(d.sub, dest, layer::copy_options::recursive | layer::copy_options::skip_symlinks);
        CHECK(layer::exists_native(dest));
    }
}

TEST_CASE("file::rename_native", "[filesystem][file]") {
    TempDir d;

    SECTION("rename file") {
        native_char src[PATH_MAX];
        NATIVE_PRINTF(src, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("rename_src.txt"), d.sub);
        native_char dst[PATH_MAX];
        NATIVE_PRINTF(dst, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("rename_dst.txt"), d.sub);

        {
            FILE *f = native_fopen(src, _T("w"));
            REQUIRE(f);
            std::fprintf(f, "rename me");
            std::fclose(f);
        }
        REQUIRE(layer::exists_native(src));

        layer::rename_native(src, dst);
        REQUIRE_FALSE(layer::exists_native(src));
        REQUIRE(layer::is_regular_file_native(dst));
    }
}

TEST_CASE("file::resize_file_native", "[filesystem][file]") {
    TempDir d;

    SECTION("truncate") {
        layer::resize_file_native(d.file_a, 2);
        REQUIRE(layer::file_size_native(d.file_a) == 2);
    }

    SECTION("extend") {
        layer::resize_file_native(d.file_b, 100);
        REQUIRE(layer::file_size_native(d.file_b) == 100);
    }
}

TEST_CASE("symlink::create_symlink_native / read_symlink_native / copy_symlink", "[filesystem][symlink]") {
    TempDir d;

    SECTION("create_symlink then read back") {
        layer::create_symlink_native(d.file_a, d.link_ab);
        REQUIRE(layer::is_symlink_native(d.link_ab));

        native_char target[PATH_MAX];
        const ssize_t len = layer::read_symlink_native(d.link_ab, target, PATH_MAX);
        REQUIRE(len > 0);
        target[len] = '\0';
        REQUIRE(native_strcmp(target, d.file_a) == 0);
    }

    SECTION("copy_symlink") {
        layer::create_symlink_native(d.file_a, d.link_ab);
        REQUIRE(layer::is_symlink_native(d.link_ab));

        native_char copied[PATH_MAX];
        NATIVE_PRINTF(copied, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("copied_link"), d.sub);
        layer::copy_symlink_native(d.link_ab, copied);
        REQUIRE(layer::is_symlink_native(copied));

        native_char target[PATH_MAX];
        const ssize_t len = layer::read_symlink_native(copied, target, PATH_MAX);
        REQUIRE(len > 0);
        target[len] = '\0';
        REQUIRE(native_strcmp(target, d.file_a) == 0);
    }

    SECTION("create_directory_symlink") {
        layer::create_directory_symlink_native(d.sub, d.link_ab);
        REQUIRE(layer::is_symlink_native(d.link_ab));
    }

    SECTION("read_symlink on non-symlink") {
        native_char buf[PATH_MAX];
        REQUIRE(layer::read_symlink_native(d.file_a, buf, PATH_MAX) < 0);
    }
}

TEST_CASE("hardlink::create_hard_link_native", "[filesystem][hardlink]") {
    TempDir d;

    SECTION("create and verify via equivalent_native") {
        native_char hl[PATH_MAX];
        NATIVE_PRINTF(hl, PATH_MAX, PRI_NATIVE_S PRI_NATIVE_SEP _T("hardlink_to_a"), d.sub);
        layer::create_hard_link_native(d.file_a, hl);
        REQUIRE(layer::exists_native(hl));
        REQUIRE(layer::equivalent_native(d.file_a, hl));
        std::uintmax_t cnt{};
        REQUIRE(layer::hard_link_count_native(d.file_a, &cnt));
        REQUIRE(cnt >= 2);
    }
}

TEST_CASE("permissions::permissions_native", "[filesystem][perms]") {
    TempDir d;

#if RAINY_USING_WINDOWS
    // Windows permission model is READONLY bit only: !READONLY → 0666, READONLY → 0444
    SECTION("replace") {
        layer::permissions_native(d.file_a, layer::perms::owner_write);
        {
            const auto st = layer::status_native(d.file_a);
            REQUIRE(static_cast<unsigned>(st.permissions) == 0666u);
        }
        layer::permissions_native(d.file_a, layer::perms::owner_read | layer::perms::owner_write);
    }

    SECTION("add + remove") {
        layer::permissions_native(d.file_a, layer::perms::owner_write);
        layer::permissions_native(d.file_a, layer::perms::owner_read, layer::perm_options::add);
        {
            const auto st = layer::status_native(d.file_a);
            REQUIRE(static_cast<unsigned>(st.permissions) == 0666u);
        }
        layer::permissions_native(d.file_a, layer::perms::owner_write, layer::perm_options::remove);
        {
            const auto st = layer::status_native(d.file_a);
            REQUIRE(static_cast<unsigned>(st.permissions) == 0444u);
        }
        layer::permissions_native(d.file_a, layer::perms::owner_read | layer::perms::owner_write);
    }
#else
    // POSIX: fine-grained permission bits via chmod
    SECTION("replace") {
        layer::permissions_native(d.file_a, layer::perms::owner_write);
        {
            const auto st = layer::status_native(d.file_a);
            REQUIRE(static_cast<unsigned>(st.permissions) == static_cast<unsigned>(layer::perms::owner_write));
        }
        layer::permissions_native(d.file_a, layer::perms::owner_read | layer::perms::owner_write);
    }

    SECTION("add + remove") {
        layer::permissions_native(d.file_a, layer::perms::owner_write);
        layer::permissions_native(d.file_a, layer::perms::owner_read, layer::perm_options::add);
        {
            const auto st = layer::status_native(d.file_a);
            REQUIRE(static_cast<unsigned>(st.permissions) ==
                    (static_cast<unsigned>(layer::perms::owner_write) | static_cast<unsigned>(layer::perms::owner_read)));
        }
        layer::permissions_native(d.file_a, layer::perms::owner_write, layer::perm_options::remove);
        {
            const auto st = layer::status_native(d.file_a);
            REQUIRE(static_cast<unsigned>(st.permissions) == static_cast<unsigned>(layer::perms::owner_read));
        }
        layer::permissions_native(d.file_a, layer::perms::owner_read | layer::perms::owner_write);
    }
#endif
}

TEST_CASE("space::space_native", "[filesystem][space]") {
    TempDir d;

    SECTION("out-parameter") {
        layer::space_info info{};
        REQUIRE(layer::space_native(d.root, &info));
        REQUIRE(info.capacity > 0);
        REQUIRE(info.free > 0);
        REQUIRE(info.available > 0);
        REQUIRE(info.free >= info.available);
    }

    SECTION("return-value") {
        const auto info = layer::space_native(d.root);
        REQUIRE(info.capacity > 0);
        REQUIRE(info.free > 0);
        REQUIRE(info.available > 0);
    }
}

TEST_CASE("misc::temp_directory_path_native", "[filesystem][misc]") {
    SECTION("returns a directory path") {
        native_char buf[PATH_MAX];
        const ssize_t len = layer::temp_directory_path_native(buf, PATH_MAX);
        REQUIRE(len > 0);
        REQUIRE(layer::is_directory_native(buf));
    }

    SECTION("buffer too small") {
        native_char buf[2];
        const ssize_t len = layer::temp_directory_path_native(buf, 2);
        REQUIRE(len != 0);
    }
}

TEST_CASE("edge::error_cases", "[filesystem][edge]") {
    SECTION("exists_native on empty") {
        REQUIRE_FALSE(layer::exists_native(_T("")));
    }

    SECTION("is_directory_native on empty") {
        REQUIRE_FALSE(layer::is_directory_native(_T("")));
    }

    SECTION("is_regular_file_native on empty") {
        REQUIRE_FALSE(layer::is_regular_file_native(_T("")));
    }

    SECTION("status_native on empty") {
        const auto st = layer::status_native(_T(""));
        REQUIRE((st.type == layer::file_type::not_found || st.type == layer::file_type::unknown));
    }

    SECTION("file_size_native on empty") {
        REQUIRE(layer::file_size_native(_T("")) == static_cast<std::uintmax_t>(-1));
    }

    SECTION("remove_native on empty") {
        REQUIRE_FALSE(layer::remove_native(_T("")));
    }

    SECTION("remove_all_native on non-existent") {
        REQUIRE(layer::remove_all_native(_T("/tmp/_____should_not_exist_12345_____")) == 0);
    }
}
