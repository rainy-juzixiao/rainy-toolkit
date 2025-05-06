#include <ctime>
#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <fstream>

#include <rainy/core/core.hpp>

using namespace rainy::core;

#if RAINY_USING_WINDOWS
#include <windows.h>
#else
#include <fcntl.h>
#endif

class FileStatusTest : public ::testing::Test {
protected:
    std::string GetTestDir() {
        return std::filesystem::current_path().string() + "/";
    }

    void SetUp() override {
        std::string test_dir = GetTestDir();

        // 创建测试文件和目录
#ifdef _WIN32
        // 创建和设置文件
        {
            std::ofstream file(test_dir + "test_regular.txt");
            file << "Test file content";
        }
        SetFileAttributesA((test_dir + "test_regular.txt").c_str(), FILE_ATTRIBUTE_READONLY);

        // 创建目录
        CreateDirectoryA((test_dir + "test_directory").c_str(), NULL);

        // 创建隐藏文件
        {
            std::ofstream file(test_dir + "test_hidden.txt");
            file << "Test hidden file";
        }
        SetFileAttributesA((test_dir + "test_hidden.txt").c_str(), FILE_ATTRIBUTE_HIDDEN);

        // 创建符号链接
        CreateSymbolicLinkA((test_dir + "test_symlink.txt").c_str(), (test_dir + "test_regular.txt").c_str(),
                            0 // 不是目录
        );
#else
        system(("echo 'Test file content' > " + test_dir + "test_regular.txt").c_str());
        system(("mkdir -p " + test_dir + "test_directory").c_str());
        system(("chmod -w " + test_dir + "test_regular.txt").c_str());
        system(("touch " + test_dir + ".test_hidden.txt").c_str());
        system(("ln -sf " + test_dir + "test_regular.txt " + test_dir + "test_symlink.txt").c_str());
        system(("dd if=/dev/zero of=" + test_dir + "test_sparse.txt bs=1 count=0 seek=1M").c_str());
#endif
    }

    void TearDown() override {
        std::string test_dir = GetTestDir();

        // 清理测试文件
#ifdef _WIN32
        SetFileAttributesA((test_dir + "test_regular.txt").c_str(), FILE_ATTRIBUTE_NORMAL);
        DeleteFileA((test_dir + "test_regular.txt").c_str());
        DeleteFileA((test_dir + "test_hidden.txt").c_str());
        DeleteFileA((test_dir + "test_symlink.txt").c_str());
        RemoveDirectoryA((test_dir + "test_directory").c_str());
#else
        system(("chmod +w " + test_dir + "test_regular.txt 2>/dev/null").c_str());
        system(("rm -f " + test_dir + "test_regular.txt 2>/dev/null").c_str());
        system(("rm -f " + test_dir + ".test_hidden.txt 2>/dev/null").c_str());
        system(("rm -f " + test_dir + "test_symlink.txt 2>/dev/null").c_str());
        system(("rm -f " + test_dir + "test_sparse.txt 2>/dev/null").c_str());
        system(("rmdir " + test_dir + "test_directory 2>/dev/null").c_str());
#endif
    }

    // 辅助函数，检查特定属性是否设置
    bool HasAttribute(pal::file_attributes attrs, pal::file_attributes attr) {
        return (static_cast<int>(attrs) & static_cast<int>(attr)) != 0;
    }
};

// 测试普通文件
TEST_F(FileStatusTest, RegularFile) {
    std::string test_file = GetTestDir() + "test_regular.txt";
    pal::file_status status = pal::get_file_status(test_file.c_str());
    // 验证文件大小大于0
    EXPECT_GT(status.file_size, 0);
    // 验证最后修改时间是最近的
    time_t now = time(nullptr);
    EXPECT_LT(now - status.last_write_time, 120); // 最多120秒前创建
    // Windows的只读文件通常不会同时具有normal属性，所以我们不检查normal
    EXPECT_FALSE(HasAttribute(status.attributes, pal::file_attributes::directory));
    // 验证不是重解析点
    EXPECT_EQ(status.reparse_tag, pal::reparse_tag::none);
    // 验证链接计数
    EXPECT_GE(status.link_count, 1);
}

// 测试目录
TEST_F(FileStatusTest, Directory) {
    std::string test_dir = GetTestDir() + "test_directory";
    pal::file_status status = pal::get_file_status(test_dir.c_str());

    // 验证目录属性
    EXPECT_TRUE(HasAttribute(status.attributes, pal::file_attributes::directory));
    EXPECT_FALSE(HasAttribute(status.attributes, pal::file_attributes::normal));

    // 验证不是重解析点
    EXPECT_EQ(status.reparse_tag, pal::reparse_tag::none);
}

// 测试隐藏文件
TEST_F(FileStatusTest, HiddenFile) {
#ifdef _WIN32
    std::string test_file = GetTestDir() + "test_hidden.txt";
#else
    std::string test_file = GetTestDir() + ".test_hidden.txt";
#endif
    pal::file_status status = pal::get_file_status(test_file.c_str());

    // 验证隐藏属性
    EXPECT_TRUE(HasAttribute(status.attributes, pal::file_attributes::hidden));
}

// 测试符号链接（如果可用）
TEST_F(FileStatusTest, SymbolicLink) {
    std::string test_file = GetTestDir() + "test_symlink.txt";

    // 检查文件是否存在，如果不存在就跳过测试
    if (!std::filesystem::exists(test_file)) {
        GTEST_SKIP() << "Symbolic link test skipped because the link couldn't be created (requires admin rights on Windows)";
    }

    pal::file_status status = pal::get_file_status(test_file.c_str());

    // 验证是重解析点
    EXPECT_TRUE(HasAttribute(status.attributes, pal::file_attributes::reparse_point));
    EXPECT_EQ(status.reparse_tag, pal::reparse_tag::symlink);
}

// 测试不存在的文件
TEST_F(FileStatusTest, NonexistentFile) {
    std::string test_file = GetTestDir() + "nonexistent_file.txt";
    pal::file_status status = pal::get_file_status(test_file.c_str());

    // 验证返回无效标志
    EXPECT_EQ(status.attributes, pal::file_attributes::invalid);
}

// Windows特有测试
#ifdef _WIN32
// 测试系统文件
TEST_F(FileStatusTest, SystemFile) {
    // 使用更可靠的系统文件路径
    auto* system_file = R"(C:\Windows\System32\kernel32.dll)";
    // 检查文件是否存在，如果不存在就跳过测试
    if (!std::filesystem::exists(system_file)) {
        GTEST_SKIP() << "System file test skipped because the file doesn't exist";
    }
    pal::file_status status = pal::get_file_status(system_file); // NOLINT
    EXPECT_NE(status.attributes, pal::file_attributes::invalid);
}
#endif

// Linux特有测试
#if RAINY_USING_LINUX
// 测试稀疏文件
TEST_F(FileStatusTest, SparseFile) {
    std::string test_file = GetTestDir() + "test_sparse.txt";
    int fd = open(test_file.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fd >= 0) {
        (void) ftruncate(fd, 1024 * 1024); // 设定文件的逻辑大小
        close(fd);
    }
    pal::file_status status = pal::get_file_status(test_file.c_str());
    // 验证文件大小应该是1MB
    EXPECT_EQ(status.file_size, 1024 * 1024);
}

// 测试设备文件
TEST_F(FileStatusTest, DeviceFile) {
    pal::file_status status = pal::get_file_status("/dev/null");
    // 验证设备属性
    EXPECT_TRUE(HasAttribute(status.attributes, pal::file_attributes::device));
}
#endif
