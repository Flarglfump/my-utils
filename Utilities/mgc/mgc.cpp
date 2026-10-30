#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

// Exit codes
constexpr int kExitOk          = 0;
constexpr int kExitUsage       = 1;
constexpr int kExitBadPath     = 2;
constexpr int kExitBadLang     = 3;
constexpr int kExitWithErrors  = 4; // ran to completion, but some items failed
constexpr int kExitFatal       = 5; // unexpected filesystem/runtime error

std::string sanitizeName(const std::string& src);
std::string toUpper(std::string str);
std::string shellQuote(const std::string& s);
bool isHidden(const fs::path& p);
bool isWithin(const fs::path& child, const fs::path& parent);
std::vector<fs::directory_entry> sortedEntries(const fs::path& dir);
int mgcCopy(const fs::path& sourceDir, const fs::path& targetDir);
int mgcBuild(const fs::path& targetDir, const std::string& targetLang, const std::string& assignmentName);

static void printUsage(std::ostream& out, const char* prefix) {
    out << prefix << " Usage: \n"
        << "\tmgc copy <SourceDirectory> <TargetDirectory>\n"
        << "\tmgc build <TargetDirectory> <TargetLang> [AssignmentName]\n"
        << "\tmgc help [copy]|[build]" << std::endl;
}

static int run(int argc, char* argv[]) {
    int errct = 0;

    if (argc <= 1) {
        printUsage(std::cerr, "Error:");
        return kExitUsage;
    }

    const std::string command = toUpper(argv[1]);

    if (command == "HELP" || command == "-H" || command == "--HELP") {
        const std::string topic = argc >= 3 ? toUpper(argv[2]) : "";
        if (topic == "BUILD") {
            std::cout << "Help: Build: \n"
                      << "\nDescription: Will build executable files from source code files within in each folder contained in target directory. Target directory is expected to have been created from mgc copy call."
                      << "\nUsage: mgc build <TargetDirectory> <TargetLang> [AssignmentName]\n"
                      << "\nArgs:\n"
                      << "TargetDirectory (Required): Path to the directory created from call to mgc copy. This should contain folders labeled after the students' names.\n"
                      << "TargetLang (Required): This is the target language to build for. Currently supported are \"C\" and \"C++\" or \"CPP\". Specifying these target languages will search your system path for executables \"gcc\" and \"g++\" respectively to build the programs.\n"
                      << "AssignmentName (Optional): This string will be appended to the end of the student's name. This concatenated string will be used as the new name for the executable file.\n"
                      << "\nThis will iterate through each folder contained within the target directory and compile the source files directly inside it (C: *.c; C++: *.cpp, *.cc, *.cxx) with a command like \"gcc <files> -o StudentName_Progname\" or \"g++ -std=c++17 <files> -o StudentName_Progname\". Folders with no matching source files, and builds that fail, are reported and counted as errors.\n"
                      << std::endl;
        } else if (topic == "COPY") {
            std::cout << "Help: Copy: \n"
                      << "\nDescription: Will take files from within source directory and place them into subfolders in target directory. Directories within target directory path will be created if they do not exist. Running copy again into the same target directory overwrites previously copied files.\n"
                      << "\nUsage: mgc copy <SourceDirectory> <TargetDirectory>\n"
                      << "\nArgs:\n"
                      << "SourceDirectory (Required): Path to the directory extracted from the Moodle assignment submission archive.\n"
                      << "TargetDirectory (Required): Directory that is being created or added to. Subfolders will be added in this directory, each subfolder labeled by the student's name and containing that student's submitted files.\n"
                      << "\nSubfolder names are determined by the name of the source code files (or per-student folders), which are expected to follow the naming convention provided when the \"Download all submissions\" button on Moodle is clicked (\"Student Name_...\"). The downloaded archive must be extracted. That extracted directory is what the program expects as the source directory input for this program. If Moodle produced a folder per student, that folder's contents are copied into the student's subfolder. Hidden files (e.g. .DS_Store) are skipped. All of this copying and filesystem crawling is done using the std::filesystem library, available as of C++17.\n"
                      << std::endl;
        } else {
            printUsage(std::cout, "Help:");
        }
        return kExitOk;
    } else if (command == "COPY") {
        if (argc != 4) {
            std::cerr << "Error: Usage: \n"
                      << "\tmgc copy <SourceDirectory> <TargetDirectory>\n" << std::endl;
            return kExitUsage;
        }

        const fs::path srcPath{argv[2]};
        const fs::path tgtPath{argv[3]};

        if (srcPath.empty() || tgtPath.empty()) {
            std::cerr << "Error: copy: directory arguments must not be empty.\n" << std::endl;
            return kExitUsage;
        }
        std::error_code ec;
        if (!fs::exists(srcPath, ec)) {
            std::cerr << "Error: copy: Source Directory " << srcPath << " does not exist.\n" << std::endl;
            return kExitBadPath;
        }
        if (!fs::is_directory(srcPath, ec)) {
            std::cerr << "Error: copy: Source " << srcPath << " is not a directory. (Extract the Moodle archive first.)\n" << std::endl;
            return kExitBadPath;
        }
        if (fs::exists(tgtPath, ec) && !fs::is_directory(tgtPath, ec)) {
            std::cerr << "Error: copy: Target " << tgtPath << " exists and is not a directory.\n" << std::endl;
            return kExitBadPath;
        }
        if (fs::exists(tgtPath, ec) && fs::equivalent(srcPath, tgtPath, ec)) {
            std::cerr << "Error: copy: Source and target directories are the same.\n" << std::endl;
            return kExitBadPath;
        }

        errct = mgcCopy(srcPath, tgtPath);

    } else if (command == "BUILD") {
        if (argc != 4 && argc != 5) {
            std::cerr << "Error: Usage: \n"
                      << "\tmgc build <TargetDirectory> <TargetLang> [AssignmentName]\n" << std::endl;
            return kExitUsage;
        }

        const fs::path targetDirPath{argv[2]};
        const std::string targetLang = toUpper(argv[3]);
        const std::string assignmentName = argc == 5 ? std::string(argv[4]) : "";

        std::error_code ec;
        if (targetDirPath.empty() || !fs::exists(targetDirPath, ec)) {
            std::cerr << "Error: Target directory " << targetDirPath << " does not exist.\n" << std::endl;
            return kExitBadPath;
        }
        if (!fs::is_directory(targetDirPath, ec)) {
            std::cerr << "Error: Target " << targetDirPath << " is not a directory.\n" << std::endl;
            return kExitBadPath;
        }

        const std::vector<std::string> supportedLangs = {"C", "CPP", "C++"};
        if (std::find(supportedLangs.begin(), supportedLangs.end(), targetLang) == supportedLangs.end()) {
            std::cerr << "Error: Target Language \"" << argv[3] << "\" is not recognized or is not supported. Use C, CPP or C++.\n" << std::endl;
            return kExitBadLang;
        }

        errct = mgcBuild(targetDirPath, targetLang, assignmentName);

    } else {
        std::cerr << "Error: Unknown command \"" << argv[1] << "\"" << std::endl;
        printUsage(std::cerr, "Error:");
        return kExitUsage;
    }

    std::cout << "mgc finished with (" << errct << ") error(s).\n" << std::endl;
    return errct == 0 ? kExitOk : kExitWithErrors;
}

int main(int argc, char* argv[]) {
    try {
        return run(argc, argv);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: filesystem: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return kExitFatal;
}

// Removes whitespace, and replaces commas and path separators with '_',
// so a student's name is always a single, shell-friendly folder name.
std::string sanitizeName(const std::string& src) {
    std::string res;
    for (char c : src) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isspace(uc)) {
            continue;
        }
        res += (c == ',' || c == '/' || c == '\\') ? '_' : c;
    }
    return res;
}

std::string toUpper(std::string str) {
    for (char& c : str) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return str;
}

// Wraps a string in single quotes for /bin/sh, escaping embedded single quotes
// (e.g. a student named O'Brien).
std::string shellQuote(const std::string& s) {
    std::string res = "'";
    for (char c : s) {
        if (c == '\'') {
            res += "'\\''";
        } else {
            res += c;
        }
    }
    res += "'";
    return res;
}

bool isHidden(const fs::path& p) {
    const std::string name = p.filename().string();
    return !name.empty() && name[0] == '.';
}

// True if child is parent or is located somewhere inside parent.
bool isWithin(const fs::path& child, const fs::path& parent) {
    std::error_code ec;
    const fs::path c = fs::weakly_canonical(child, ec);
    if (ec) return false;
    const fs::path p = fs::weakly_canonical(parent, ec);
    if (ec) return false;
    auto pit = p.begin();
    auto cit = c.begin();
    for (; pit != p.end(); ++pit, ++cit) {
        if (pit->empty()) continue; // trailing separator
        if (cit == c.end() || *cit != *pit) return false;
    }
    return true;
}

// Directory listing sorted by name, so output order is predictable.
std::vector<fs::directory_entry> sortedEntries(const fs::path& dir) {
    std::vector<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator{dir}) {
        entries.push_back(entry);
    }
    std::sort(entries.begin(), entries.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return a.path().filename() < b.path().filename();
              });
    return entries;
}

int mgcCopy(const fs::path& sourceDir, const fs::path& targetDir) {
    int errct = 0;
    int copied = 0;
    std::error_code ec;

    std::cout << "Target directory: " << targetDir << std::endl;

    // Create overall target directory (it is fine if it already exists)
    fs::create_directories(targetDir, ec);
    if (ec || !fs::is_directory(targetDir)) {
        std::cerr << "Error: could not create or write to target directory " << targetDir
                  << (ec ? ": " + ec.message() : "") << "\n" << std::endl;
        exit(kExitBadPath);
    }

    for (const auto& entry : sortedEntries(sourceDir)) {
        const fs::path& srcPath = entry.path();
        const std::string srcName = srcPath.filename().string();

        if (isHidden(srcPath)) {
            continue; // .DS_Store and friends
        }
        if (isWithin(targetDir, srcPath)) {
            continue; // target directory lives inside the source directory
        }

        std::cout << "Checking \"" << srcName << "\"..." << std::endl;

        const std::string::size_type where = srcName.find('_');
        if (where == std::string::npos || where == 0) {
            std::cerr << "\tError: Invalid filename format (expected \"Student Name_...\")" << std::endl;
            errct++;
            continue;
        }

        const std::string studName = sanitizeName(srcName.substr(0, where));
        if (studName.empty()) {
            std::cerr << "\tError: Could not determine student name" << std::endl;
            errct++;
            continue;
        }
        std::cout << "\tStudName: " << studName << std::endl;

        // Create target subdirectory
        const fs::path studDir = targetDir / studName;
        fs::create_directories(studDir, ec);
        if (ec) {
            std::cerr << "\tError: Unable to create subdirectory " << studDir << ": " << ec.message() << std::endl;
            errct++;
            continue;
        }

        if (entry.is_directory(ec)) {
            // Moodle per-student folder: copy its contents into the student's folder
            std::cout << "\tSource folder: " << srcPath << "\n\tDest: " << studDir << std::endl;
            fs::copy(srcPath, studDir,
                     fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
        } else {
            const fs::path destPath = studDir / srcPath.filename();
            std::cout << "\tSource: " << srcPath << "\n\tDest: " << destPath << std::endl;
            fs::copy_file(srcPath, destPath, fs::copy_options::overwrite_existing, ec);
        }

        if (ec) {
            std::cerr << "\tError: Unable to copy into subdirectory: " << ec.message() << std::endl;
            errct++;
        } else {
            copied++;
        }
    }

    if (copied == 0 && errct == 0) {
        std::cerr << "Warning: no submissions found in " << sourceDir << std::endl;
    }
    return errct;
}

int mgcBuild(const fs::path& targetDir, const std::string& targetLang, const std::string& assignmentName) {
    int errct = 0;
    int built = 0;
    std::error_code ec;

    const bool isC = (targetLang == "C");
    const std::vector<std::string> exts = isC ? std::vector<std::string>{".c"}
                                              : std::vector<std::string>{".cpp", ".cc", ".cxx"};
    const std::string compiler = isC ? "gcc" : "g++ -std=c++17";
    const std::string exeSuffix = assignmentName.empty() ? "" : "_" + sanitizeName(assignmentName);

    for (const auto& dir_entry : sortedEntries(targetDir)) {
        const fs::path subDirPath = dir_entry.path();

        if (!dir_entry.is_directory(ec) || isHidden(subDirPath)) {
            continue;
        }

        const std::string studName = subDirPath.filename().string();

        // Collect source files directly inside the student's folder
        std::vector<fs::path> sources;
        for (const auto& f : sortedEntries(subDirPath)) {
            if (!f.is_regular_file(ec) || isHidden(f.path())) continue;
            const std::string ext = f.path().extension().string();
            if (std::find(exts.begin(), exts.end(), ext) != exts.end()) {
                sources.push_back(f.path());
            }
        }

        if (sources.empty()) {
            std::cerr << "Error: " << studName << ": no " << (isC ? ".c" : ".cpp/.cc/.cxx")
                      << " source files found, skipping\n" << std::endl;
            errct++;
            continue;
        }

        const fs::path exePath = subDirPath / (studName + exeSuffix);
        std::string cmdStr = compiler;
        for (const auto& src : sources) {
            cmdStr += " " + shellQuote(src.string());
        }
        cmdStr += " -o " + shellQuote(exePath.string());

        std::cout << "Executing system(" << cmdStr << ");\n" << std::endl;
        std::cout.flush();
        const int status = std::system(cmdStr.c_str());
        if (status != 0) {
            std::cerr << "Error: " << studName << ": build failed\n" << std::endl;
            errct++;
        } else {
            built++;
        }
    }

    if (built == 0 && errct == 0) {
        std::cerr << "Warning: no student folders found in " << targetDir << std::endl;
    }
    return errct;
}
