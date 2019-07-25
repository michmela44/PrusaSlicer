#ifndef TEST_OPTIONS_HPP
#include <string>

/// Directory path, passed in from the outside, for the path to the test inputs dir.
constexpr auto* testfile_dir {"/home/tamas/Workspace/prusa3d/Slic3r/src-upstream-master/src/test/inputs/"};

inline std::string testfile(std::string filename) {
    std::string result;
    result.append(testfile_dir);
    result.append(filename);
    return result;
}

#endif // TEST_OPTIONS_HPP
