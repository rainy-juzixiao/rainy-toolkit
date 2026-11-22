#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/io/filesystem/path.hpp>

using namespace rainy::foundation::io::filesystem;

TEST_CASE("path - Windows-style roots", "[filesystem][path][root]") {
#if RAINY_USING_WINDOWS
    SECTION("drive-letter root") {
        const path p{"C:\\dir\\file"};
        CHECK(p.root_name() == path{"C:"});
        CHECK(p.root_directory() == path{"\\"});
        CHECK(p.is_absolute());
        CHECK(p.filename() == path{"file"});
    }
    SECTION("UNC root") {
        const path p{"\\\\server\\share\\dir"};
        CHECK(p.root_name() == path{"\\\\server"});
        CHECK(p.root_directory() == path{"\\"});
        CHECK(p.is_absolute());
        CHECK(p.relative_path() == path{"share\\dir"});
    }
    SECTION("UNC root without a share") {
        const path p{"\\\\server"};
        CHECK(p.root_name() == path{"\\\\server"});
        CHECK_FALSE(p.has_root_directory());
        CHECK_FALSE(p.is_absolute());
    }
#endif
}

TEST_CASE("path - backslash is a filename character on POSIX", "[filesystem][path][separator]") {
#if !RAINY_USING_WINDOWS
    SECTION("backslash does not split components") {
        const path p{"dir\\file"};
        CHECK(p.filename() == path{"dir\\file"});
        CHECK(p.parent_path() == path{});
        CHECK(p.has_filename());
    }
    SECTION("forward slash splits components") {
        const path p{"dir/file"};
        CHECK(p.filename() == path{"file"});
        CHECK(p.parent_path() == path{"dir"});
    }
#endif
}
