#include <catch2/catch.hpp>

#include <filesystem>
#include <fstream>

#include "../src/shared_utils.hpp"

namespace fs = std::filesystem;

class TemporaryDirectoryFixture
{
  public:
    TemporaryDirectoryFixture(const std::string& subdir)
        : tmpDirectory(fs::temp_directory_path() / subdir) {
        fs::create_directories(tmpDirectory);
    }

    ~TemporaryDirectoryFixture() {
        fs::remove_all(tmpDirectory);
    }

    fs::path tmpDirectory;
};

class SymlinkFixture: TemporaryDirectoryFixture
{
  public:
    SymlinkFixture(const std::string& subdir)
        : TemporaryDirectoryFixture(subdir)
        , dirname(tmpDirectory / "dir")
        , symlinkDirname(tmpDirectory / "dir.symlink")
        , filename(tmpDirectory / "file")
        , symlinkFilename(tmpDirectory / "file.symlink")
        , doesntExist(tmpDirectory / "doesnt_exist")
        , symlinkDoesntExist(tmpDirectory / "doesnt_exist.symlink") {
        fs::create_directories(dirname);
        fs::create_directory_symlink(dirname, symlinkDirname);

        {
            std::ofstream f(filename);
            f << "foo";
        }
        fs::create_symlink(filename, symlinkFilename);
        fs::create_symlink(doesntExist, symlinkDoesntExist);
    }

    fs::path dirname;
    fs::path symlinkDirname;

    fs::path filename;
    fs::path symlinkFilename;

    fs::path doesntExist;
    fs::path symlinkDoesntExist;
};


TEST_CASE("morphio::is_regular_file", "[filesystem]") {
    auto fixture = SymlinkFixture("test_utilities.cpp");

    REQUIRE(!morphio::is_regular_file(fixture.dirname.string()));
    REQUIRE(!morphio::is_regular_file(fixture.symlinkDirname.string()));
    REQUIRE(morphio::is_regular_file(fixture.filename.string()));
    REQUIRE(morphio::is_regular_file(fixture.symlinkFilename.string()));
    REQUIRE(!morphio::is_directory(fixture.doesntExist.string()));
    REQUIRE(!morphio::is_directory(fixture.symlinkDoesntExist.string()));
}


TEST_CASE("morphio::is_directory", "[filesystem]") {
    auto fixture = SymlinkFixture("test_utilities.cpp");

    REQUIRE(morphio::is_directory(fixture.dirname.string()));
    REQUIRE(morphio::is_directory(fixture.symlinkDirname.string()));
    REQUIRE(!morphio::is_directory(fixture.filename.string()));
    REQUIRE(!morphio::is_directory(fixture.symlinkFilename.string()));
    REQUIRE(!morphio::is_directory(fixture.doesntExist.string()));
    REQUIRE(!morphio::is_directory(fixture.symlinkDoesntExist.string()));
}


TEST_CASE("morphio::join_path", "[filesystem]") {
    SECTION("join_path('', 'foo')", "[filesystem]") {
        std::string expected = "foo";
        std::string actual = morphio::join_path("", "foo");
        REQUIRE(expected == actual);
    }

    SECTION("join_path('bar', '/foo')", "[filesystem]") {
        std::string expected = "/foo";
        std::string actual = morphio::join_path("bar", "/foo");
        REQUIRE(expected == actual);
    }

    SECTION("join_path('bar', 'foo')", "[filesystem]") {
        std::string expected = "bar/foo";
        std::string actual = morphio::join_path("bar", "foo");
        REQUIRE(expected == actual);
    }
}
