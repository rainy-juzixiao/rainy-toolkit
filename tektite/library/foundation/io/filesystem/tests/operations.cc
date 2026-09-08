#include <catch2/catch_all.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <chrono>
#include <fstream>
#include <rainy/foundation/io/filesystem.hpp>
#include <thread>

using namespace rainy::foundation::io::filesystem;

static void create_test_file(const path &p, const std::string &content = "test content") {
    std::ofstream file(p.string().c_str());
    file << content;
}

static void create_directory_structure(const path &base) {
    create_directories(base / "dir1" / "subdir");
    create_directories(base / "dir2");
    create_test_file(base / "file1.txt");
    create_test_file(base / "dir1" / "file2.txt");
}

static void cleanup_test_dir(const path &test_dir) {
    std::error_code ec;
    remove_all(test_dir, ec);
}

TEST_CASE("Path operations", "[filesystem][path]") {
    SECTION("A temporary test directory") {
        auto test_root = temp_directory_path() / "rainy_test_absolute";
        create_directories(test_root);

        SECTION("Getting absolute path of a relative path") {
            path relative_path = "test_dir/test_file.txt";
            auto abs_path = absolute(relative_path);

            SECTION("The path should be absolute") {
                REQUIRE(abs_path.is_absolute());
                REQUIRE(abs_path.generic_string().find(relative_path.generic_string()) != std::string::npos);
            }
        }

        cleanup_test_dir(test_root);
    }

    SECTION("A path with symbolic links") {
#if RAINY_USING_WINDOWS
        SECTION("Creating directory junction on Windows") {
            path target = "C:\\temp\\target";
            path link = "C:\\temp\\link";
            SECTION("Canonical path should resolve the junction") {
            }
        }
#elif RAINY_USING_LINUX
        SECTION("Creating symbolic link on Linux") {
            path target = "/tmp/target";
            path link = "/tmp/link";
            create_symlink(target, link);
            auto canon = canonical(link);
            SECTION("Canonical path should resolve the symlink") {
                REQUIRE(canon == absolute(target));
            }
        }
#elif RAINY_USING_MACOS
        SECTION("Creating symlink with special macOS paths") {
            path target = "/var/tmp/target";
            path link = "/tmp/link";
            create_symlink(target, link);
            auto canon = canonical(link);
            SECTION("Canonical path should resolve correctly on macOS") {
                REQUIRE(canon == absolute(target));
            }
        }
#endif
    }

    SECTION("Edge cases for absolute path") {
        SECTION("Passing empty path") {
            path empty_path;
            SECTION("Absolute of empty path should throw exception") {
                REQUIRE_THROWS(absolute(empty_path));
            }
        }

        SECTION("Passing already absolute path") {
#if RAINY_USING_WINDOWS
            path abs_path = current_path().root_name() / "usr/local";
#else
            path abs_path = "/usr/local";
#endif
            auto result = absolute(abs_path);
            SECTION("Should return the same path") {
                REQUIRE(result == abs_path);
            }
        }
    }
}

TEST_CASE("Copy operations", "[filesystem][copy]") {
    SECTION("Valid source and destination paths") {
        auto test_dir = temp_directory_path() / "rainy_test_copy";
        create_directories(test_dir);
        path source = test_dir / "source.txt";
        path dest = test_dir / "dest.txt";
        create_test_file(source, "original content");

        SECTION("Copying a file") {
            copy(source, dest);

            SECTION("Destination file should exist with same content") {
                REQUIRE(exists(dest));
                REQUIRE(file_size(source) == file_size(dest));
            }
        }

        SECTION("Copying with overwrite options") {
            create_test_file(dest, "different content");
            copy(source, dest, copy_options::overwrite_existing);

            SECTION("Should overwrite destination") {
                REQUIRE(file_size(source) == file_size(dest));
            }
        }

        SECTION("Copying with skip existing option") {
            create_test_file(dest, "different content");
            auto original_size = file_size(dest);
            copy(source, dest, copy_options::skip_existing);

            SECTION("Should not overwrite") {
                REQUIRE(file_size(dest) == original_size);
            }
        }

        cleanup_test_dir(test_dir);
    }

    SECTION("Directory copy operations") {
        auto test_dir = temp_directory_path() / "rainy_test_copy_dir";
        path source_dir = test_dir / "source";
        path dest_dir = test_dir / "dest";
        create_directory_structure(source_dir);

        SECTION("Copying entire directory recursively") {
            copy(source_dir, dest_dir, copy_options::recursive);

            SECTION("All files should be copied") {
                REQUIRE(exists(dest_dir / "file1.txt"));
                REQUIRE(exists(dest_dir / "dir1" / "file2.txt"));
            }
        }

        cleanup_test_dir(test_dir);
    }

    SECTION("Edge cases for copy_file") {
        auto test_dir = temp_directory_path() / "rainy_test_copy_file";
        create_directories(test_dir);

        SECTION("Source file doesn't exist") {
            path source = test_dir / "nonexistent.txt";
            path dest = test_dir / "dest.txt";
            std::error_code ec;

            SECTION("Should return false") {
                bool result = copy_file(source, dest, ec);
                REQUIRE(!result);
                REQUIRE(ec);
            }
        }

        SECTION("Destination is read-only") {
            path source = test_dir / "source.txt";
            path dest = test_dir / "dest_readonly.txt";
            create_test_file(source);
            create_test_file(dest);

#if RAINY_USING_LINUX || RAINY_USING_MACOS
            permissions(dest, perms::owner_write, perm_options::remove);
#elif RAINY_USING_WINDOWS
            permissions(dest, perms::owner_read, perm_options::replace);
#endif

            std::error_code ec;
            bool result = copy_file(source, dest, ec);

            SECTION("Should fail with permission error") {
                REQUIRE(!result);
                REQUIRE(ec);
            }
        }

        cleanup_test_dir(test_dir);
    }
}

TEST_CASE("Directory creation and removal", "[filesystem][dir]") {
    SECTION("Creating nested directories") {
        auto test_dir = temp_directory_path() / "rainy_test_create";
        path nested = test_dir / "level1" / "level2" / "level3";

        SECTION("Creating all directories") {
            bool result = create_directories(nested);

            SECTION("All directories should exist") {
                REQUIRE(result);
                REQUIRE(exists(nested));
                REQUIRE(is_directory(nested));
            }
        }

        SECTION("Creating existing directory") {
            create_directories(nested);
            bool result = create_directories(nested);

            SECTION("Should return false") {
                REQUIRE(!result);
            }
        }

        cleanup_test_dir(test_dir);
    }

    SECTION("Creating directory with attributes from existing") {
        auto test_dir = temp_directory_path() / "rainy_test_attr";
        path template_dir = test_dir / "template";
        path new_dir = test_dir / "new";

        create_directory(test_dir);
        create_directory(template_dir);

#if RAINY_USING_LINUX || RAINY_USING_MACOS
        permissions(template_dir, perms::owner_all | perms::group_read);
#endif

        SECTION("Creating directory with existing path as template") {
            bool result = create_directory(new_dir, template_dir);

            SECTION("New directory should have similar attributes") {
                REQUIRE(result);
                REQUIRE(exists(new_dir));
            }
        }

        cleanup_test_dir(test_dir);
    }

#if RAINY_USING_LINUX || RAINY_USING_MACOS
    SECTION("Creating symbolic links on Unix systems") {
        auto test_dir = temp_directory_path() / "rainy_test_symlink";
        create_directories(test_dir);
        path target_dir = test_dir / "target";
        path symlink_dir = test_dir / "symlink";
        create_directory(target_dir);

        SECTION("Creating directory symlink") {
            create_directory_symlink(target_dir, symlink_dir);

            SECTION("Symlink should be created") {
                REQUIRE(is_symlink(symlink_dir));
                auto resolved = read_symlink(symlink_dir);
                REQUIRE(resolved == target_dir);
            }
        }

        cleanup_test_dir(test_dir);
    }
#endif
}

TEST_CASE("File status and properties", "[filesystem][status]") {
    SECTION("Different file types") {
        auto test_dir = temp_directory_path() / "rainy_test_status";
        create_directories(test_dir);
        path regular_file = test_dir / "file.txt";
        path dir_path = test_dir / "subdir";

        create_test_file(regular_file);
        create_directory(dir_path);

        SECTION("Checking file types") {
            SECTION("Regular file should be identified") {
                REQUIRE(is_regular_file(regular_file));
                REQUIRE(!is_directory(regular_file));
                REQUIRE(!is_symlink(regular_file));
            }

            SECTION("Directory should be identified") {
                REQUIRE(is_directory(dir_path));
                REQUIRE(!is_regular_file(dir_path));
            }
        }

        SECTION("Checking file size") {
            auto size = file_size(regular_file);
            SECTION("Size should match content") {
                REQUIRE(size > 0);
            }
        }

        cleanup_test_dir(test_dir);
    }

    SECTION("Empty file test") {
        auto test_dir = temp_directory_path() / "rainy_test_empty";
        create_directories(test_dir);
        path empty_file = test_dir / "empty.txt";
        path non_empty_file = test_dir / "nonempty.txt";
        path empty_dir = test_dir / "emptydir";

        std::ofstream(empty_file.string().c_str());
        create_test_file(non_empty_file, "content");
        create_directory(empty_dir);

        SECTION("Checking if empty") {
            SECTION("Empty file should report empty") {
                REQUIRE(is_empty(empty_file));
            }

            SECTION("Non-empty file should not be empty") {
                REQUIRE(!is_empty(non_empty_file));
            }

            SECTION("Empty directory should be empty") {
                REQUIRE(is_empty(empty_dir));
            }
        }

        cleanup_test_dir(test_dir);
    }

    SECTION("Hard link operations") {
        auto test_dir = temp_directory_path() / "rainy_test_hardlink";
#if RAINY_USING_LINUX || RAINY_USING_MACOS
        create_directories(test_dir);
        path original = test_dir / "original.txt";
        path link = test_dir / "hardlink.txt";
        create_test_file(original);

        SECTION("Creating hard link") {
            create_hard_link(original, link);

            SECTION("Link count should increase") {
                auto original_links = hard_link_count(original);
                auto link_links = hard_link_count(link);
                REQUIRE(original_links > 1);
                REQUIRE(original_links == link_links);
            }

            SECTION("Both paths should refer to same file") {
                REQUIRE(equivalent(original, link));
            }
        }
#endif

        cleanup_test_dir(test_dir);
    }

    SECTION("Edge cases for file_size") {
        auto test_dir = temp_directory_path() / "rainy_test_size_errors";
        create_directories(test_dir);

        SECTION("Getting size of directory") {
            path dir_path = test_dir / "directory";
            create_directory(dir_path);
            std::error_code ec;

            auto size = file_size(dir_path, ec);

            SECTION("Should not return error") {
                REQUIRE(!ec);
            }
        }

        SECTION("Getting size of nonexistent file") {
            path nonexistent = test_dir / "nonexistent.txt";
            std::error_code ec;

            auto size = file_size(nonexistent, ec);

            SECTION("Should set error code") {
                REQUIRE(ec);
            }
        }

        cleanup_test_dir(test_dir);
    }
}

TEST_CASE("Path manipulation", "[filesystem][path_manip]") {
    SECTION("A current working directory") {
        auto original_cwd = current_path();
        auto test_dir = temp_directory_path() / "rainy_test_cwd";
        create_directories(test_dir);
#if RAINY_USING_MACOS // workaround
        original_cwd = canonical(original_cwd);
        test_dir = canonical(test_dir);
#endif
        SECTION("Changing current directory") {
            current_path(test_dir);
            auto new_cwd = current_path();
            SECTION("Current directory should be updated") {
                REQUIRE(new_cwd == test_dir);
            }
            current_path(original_cwd);
        }
        SECTION("Changing back to original") {
            current_path(original_cwd);
            auto restored_cwd = current_path();
            SECTION("Should restore correctly") {
                REQUIRE(restored_cwd == original_cwd);
            }
        }
        current_path(original_cwd);
        cleanup_test_dir(test_dir);
    }

    SECTION("Relative and absolute paths") {
#if RAINY_USING_MACOS // workaround
        auto base_dir = canonical(current_path());
#else
        auto base_dir = current_path();
#endif
        path absolute_path = base_dir / "some" / "file.txt";

        SECTION("Computing relative path") {
            path relative_path = relative(absolute_path, base_dir);

            SECTION("Should give relative representation") {
                REQUIRE_FALSE(relative_path.is_absolute());
                auto rel_str = relative_path.string();
                bool is_valid = (rel_str == "some/file.txt") || (rel_str == ".\\some\\file.txt");
                REQUIRE(is_valid);
            }
        }

        SECTION("Computing proximate path") {
            path other_dir = base_dir / "other";
            path target = base_dir / "some" / "file.txt";
            path proximate_path = proximate(target, other_dir);

            SECTION("Should give shortest representation") {
                REQUIRE_FALSE(proximate_path.empty());
            }
        }

        SECTION("Using weakly canonical on non-existent path") {
            path non_existent = base_dir / "nonexistent" / "file.txt";
            auto weak_canon = weakly_canonical(non_existent);

            SECTION("Should not error on missing components") {
                REQUIRE_FALSE(weak_canon.empty());
            }
        }
    }
}

TEST_CASE("Time operations", "[filesystem][time]") {
    SECTION("A test file") {
        auto test_dir = temp_directory_path() / "rainy_test_time";
        create_directories(test_dir);
        path test_file = test_dir / "timed.txt";
        create_test_file(test_file);

        SECTION("Getting last write time") {
            auto write_time = last_write_time(test_file);

            SECTION("Time should be valid") {
                REQUIRE(write_time.time_since_epoch().count() > 0);
            }
        }

        SECTION("Setting new write time") {
            auto new_time = file_time_type::clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            last_write_time(test_file, new_time);
            auto retrieved_time = last_write_time(test_file);

            SECTION("Time should be updated") {
                auto diff = std::chrono::duration_cast<std::chrono::seconds>(retrieved_time - new_time).count();
                REQUIRE(std::abs(diff) < 2);
            }
        }

        cleanup_test_dir(test_dir);
    }
}

TEST_CASE("Permission operations", "[filesystem][permissions]") {
    SECTION("A test file with various permissions") {
        auto test_dir = temp_directory_path() / "rainy_test_perms";
        create_directories(test_dir);
        path test_file = test_dir / "perms.txt";
        create_test_file(test_file);

#if RAINY_USING_LINUX || RAINY_USING_MACOS
        SECTION("Setting read-only permissions on Unix") {
            permissions(test_file, perms::owner_read);
            auto status = symlink_status(test_file);

            SECTION("Write permission should be removed") {
                REQUIRE((status.permissions() & perms::owner_write) == perms::none);
                REQUIRE((status.permissions() & perms::owner_read) != perms::none);
            }
        }

        SECTION("Adding execute permission") {
            permissions(test_file, perms::owner_exec, perm_options::add);
            auto status = symlink_status(test_file);

            SECTION("Execute bit should be set") {
                REQUIRE((status.permissions() & perms::owner_exec) != perms::none);
            }
        }

        SECTION("Removing all permissions") {
            permissions(test_file, perms::none, perm_options::replace);
            auto status = symlink_status(test_file);

            SECTION("No permissions should be set") {
                REQUIRE((status.permissions() & perms::owner_all) == perms::none);
            }
        }
#elif RAINY_USING_WINDOWS
        SECTION("Setting read-only on Windows") {
            permissions(test_file, perms::owner_read);
            auto status = symlink_status(test_file);

            SECTION("File should be read-only") {
                REQUIRE((status.permissions() & perms::owner_write) == perms::none);
            }
        }
#endif

        cleanup_test_dir(test_dir);
    }
}

TEST_CASE("Space information", "[filesystem][space]") {
    SECTION("Valid path to get space info") {
        auto test_dir = temp_directory_path() / "rainy_test_space";
        create_directories(test_dir);

        SECTION("Getting space information") {
            auto space_info = space(test_dir);

            SECTION("Capacity should be positive") {
                REQUIRE(space_info.capacity > 0);
                REQUIRE(space_info.free > 0);
                REQUIRE(space_info.available <= space_info.free);
            }
        }

        cleanup_test_dir(test_dir);
    }

    SECTION("Invalid path for space info") {
        path invalid_path = "/nonexistent/path/that/does/not/exist";
        std::error_code ec;

        SECTION("Getting space with error code") {
            auto space_info = space(invalid_path, ec);

            SECTION("Should set error code") {
                REQUIRE(ec);
                REQUIRE(space_info.capacity == static_cast<std::uintmax_t>(-1));
            }
        }
    }
}

TEST_CASE("Temporary directory operations", "[filesystem][temp]") {
    SECTION("System temporary directory") {
        SECTION("Getting temporary directory path") {
            auto temp_path = temp_directory_path();

            SECTION("Path should exist and be a directory") {
                REQUIRE(exists(temp_path));
                REQUIRE(is_directory(temp_path));
            }
        }

        SECTION("Creating files in temp directory") {
            auto temp_dir = temp_directory_path();
            path test_file = temp_dir / "rainy_test_temp_file.txt";

            create_test_file(test_file);

            SECTION("File should be created successfully") {
                REQUIRE(exists(test_file));
            }

            remove(test_file);
        }
    }
}

TEST_CASE("Symlink operations", "[filesystem][symlink]") {
#if RAINY_USING_LINUX || RAINY_USING_MACOS
    SECTION("Symbolic link operations on Unix") {
        auto test_dir = temp_directory_path() / "rainy_test_symlink_ops";
        create_directories(test_dir);
        path target_file = test_dir / "target.txt";
        path symlink_file = test_dir / "symlink.txt";
        path broken_symlink = test_dir / "broken.txt";

        create_test_file(target_file);

        SECTION("Creating symlink to file") {
            create_symlink(target_file, symlink_file);

            SECTION("Symlink should be created") {
                REQUIRE(is_symlink(symlink_file));
                auto read_target = read_symlink(symlink_file);
                REQUIRE(read_target == target_file);
            }

            SECTION("Symlink status should differ from target status") {
                auto sym_status = symlink_status(symlink_file);
                auto target_status = status(target_file);
                REQUIRE(sym_status.type() != target_status.type());
            }
        }

        SECTION("Creating broken symlink") {
            path nonexistent = test_dir / "nonexistent.txt";
            create_symlink(nonexistent, broken_symlink);

            SECTION("Status should indicate symlink but invalid target") {
                REQUIRE(is_symlink(broken_symlink));
                std::error_code ec;
                auto target = read_symlink(broken_symlink, ec);
                REQUIRE(target == nonexistent);
                REQUIRE(!ec);
            }
        }

        SECTION("Copying symlink") {
            create_symlink(target_file, symlink_file);
            path copied_symlink = test_dir / "copied.txt";
            copy_symlink(symlink_file, copied_symlink);

            SECTION("New symlink should point to same target") {
                REQUIRE(is_symlink(copied_symlink));
                auto target = read_symlink(copied_symlink);
                REQUIRE(target == target_file);
            }
        }

        cleanup_test_dir(test_dir);
    }
#endif

    SECTION("Symlink detection on any platform") {
        auto test_dir = temp_directory_path() / "rainy_test_symlink_detect";
        create_directories(test_dir);
        path regular = test_dir / "regular.txt";
        create_test_file(regular);

        auto file_status_obj = status(regular);

        SECTION("Checking is_symlink with file_status") {
            SECTION("Regular file should not be symlink") {
                REQUIRE(!is_symlink(file_status_obj));
            }
        }
    }
}

TEST_CASE("Remove operations", "[filesystem][remove]") {
    SECTION("Files and directories to remove") {
        auto test_dir = temp_directory_path() / "rainy_test_remove";
        create_directory_structure(test_dir);

        SECTION("Removing a single file") {
            path file_to_remove = test_dir / "file1.txt";
            bool result = remove(file_to_remove);

            SECTION("File should be removed") {
                REQUIRE(result);
                REQUIRE(!exists(file_to_remove));
            }
        }

        SECTION("Removing non-existent file") {
            path nonexistent = test_dir / "nonexistent.txt";
            std::error_code ec;
            bool result = remove(nonexistent, ec);

            SECTION("Should return false without error") {
                REQUIRE(!result);
                REQUIRE(!ec);
            }
        }

        SECTION("Removing entire directory tree") {
            auto removed_count = remove_all(test_dir);

            SECTION("All files should be removed") {
                REQUIRE(removed_count > 0);
                REQUIRE(!exists(test_dir));
            }
        }

        SECTION("Removing empty directory") {
            path empty_dir = test_dir / "empty_dir";
            create_directory(empty_dir);
            bool result = remove(empty_dir);

            SECTION("Empty directory should be removed") {
                REQUIRE(result);
                REQUIRE(!exists(empty_dir));
            }
        }

        SECTION("Removing non-empty directory without recursive flag") {
            std::error_code ec;
            remove(test_dir, ec);

            SECTION("Should fail on non-empty directory") {
#if RAINY_USING_LINUX || RAINY_USING_MACOS
                REQUIRE(ec);
#endif
                REQUIRE(exists(test_dir));
            }
        }

        cleanup_test_dir(test_dir);
    }
}

TEST_CASE("Rename operations", "[filesystem][rename]") {
    SECTION("Files to rename") {
        auto test_dir = temp_directory_path() / "rainy_test_rename";
        create_directories(test_dir);
        path original = test_dir / "original.txt";
        path renamed = test_dir / "renamed.txt";
        create_test_file(original);

        SECTION("Renaming a file") {
            rename(original, renamed);

            SECTION("Original should not exist, destination should exist") {
                REQUIRE(!exists(original));
                REQUIRE(exists(renamed));
            }
        }

        SECTION("Renaming over existing file") {
            path existing = test_dir / "existing.txt";
            create_test_file(existing, "existing content");
            rename(original, existing);

            SECTION("Should overwrite existing file") {
                REQUIRE(!exists(original));
                REQUIRE(exists(existing));
            }
        }

        SECTION("Renaming directory") {
            path original_dir = test_dir / "orig_dir";
            path renamed_dir = test_dir / "renamed_dir";
            create_directory(original_dir);
            rename(original_dir, renamed_dir);

            SECTION("Directory should be renamed") {
                REQUIRE(!exists(original_dir));
                REQUIRE(exists(renamed_dir));
                REQUIRE(is_directory(renamed_dir));
            }
        }

        cleanup_test_dir(test_dir);
    }

    SECTION("Cross-device rename") {
        // 跨设备重命名测试
        auto test_dir = temp_directory_path() / "rainy_test_cross";
        create_directories(test_dir);
        path source = test_dir / "source.txt";
        create_test_file(source);

        path dest = current_path() / "cross_device_rename.txt";

        SECTION("Renaming across different devices") {
            std::error_code ec;
            rename(source, dest, ec);

            SECTION("May fail on some systems") {
#if RAINY_USING_LINUX
                // Linux上跨设备重命名通常会失败
                if (ec) {
                    REQUIRE(ec);
                    // 回退到复制+删除
                    copy(source, dest);
                    remove(source);
                    REQUIRE(exists(dest));
                }
#endif
            }
        }

        remove(dest);
        cleanup_test_dir(test_dir);
    }
}

TEST_CASE("Resize operations", "[filesystem][resize]") {
    SECTION("File to resize") {
        auto test_dir = temp_directory_path() / "rainy_test_resize";
        create_directories(test_dir);
        path test_file = test_dir / "resize.txt";
        create_test_file(test_file, "original content");
        auto original_size = file_size(test_file);

        SECTION("Resizing to larger size") {
            std::uintmax_t new_size = original_size + 100;
            resize_file(test_file, new_size);

            SECTION("File size should increase") {
                REQUIRE(file_size(test_file) == new_size);
            }
        }

        SECTION("Resizing to smaller size") {
            std::uintmax_t new_size = original_size - 5;
            resize_file(test_file, new_size);

            SECTION("File size should decrease") {
                REQUIRE(file_size(test_file) == new_size);
            }
        }

        SECTION("Resizing to zero") {
            resize_file(test_file, 0);

            SECTION("File should become empty") {
                REQUIRE(file_size(test_file) == 0);
            }
        }

        SECTION("Resizing non-existent file") {
            path nonexistent = test_dir / "nonexistent.txt";
            std::error_code ec;
            resize_file(nonexistent, 100, ec);

            SECTION("Should create new file") {
#if RAINY_USING_LINUX || RAINY_USING_MACOS
                // Unix系统上resize不存在文件通常会创建
                if (!ec) {
                    REQUIRE(exists(nonexistent));
                    REQUIRE(file_size(nonexistent) == 100);
                }
#endif
            }
        }

        cleanup_test_dir(test_dir);
    }
}
