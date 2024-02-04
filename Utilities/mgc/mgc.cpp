#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

std::string removeAllWhitespaceAndReplaceComma(std::string src);
std::string toupper(std::string str);
int mgcCopy(std::string sourceDirName, std::string targetDirName);
int mgcBuild(std::string targetDirectory, std::string targetLang, std::string assignmentName);


int main(int argc, char* argv[]) {
    int errct = 0;

    if (argc <= 1) {
        std::cerr << "Error: Usage: \n"
                  << "\tmgc copy <SourceDirectory> <TargetDirectory>\n"
                  << "\tmgc build <TargetDirectory> <TargetLang> [AssignmentName]\n"
                  << "\tmgc help [copy]|[build]" << std::endl;
        exit(1);
    } else if (toupper(std::string(argv[1])) == "HELP") {
        if (argc == 2) {
            std::cout << "Help: Usage: \n"
                      << "\tmgc copy <SourceDirectory> <TargetDirectory>\n"
                      << "\tmgc build <TargetDirectory> <TargetLang> [AssignmentName]\n"
                      << "\tmgc help [copy]|[build]" << std::endl;
        } else if (toupper(std::string(argv[2])) == "BUILD") {
            std::cout << "Help: Build: \n"
                      << "\nDescription: Will build executable files from source code files within in each folder contained in target directory. Target directory is expected to have been created from mgc copy call."
                      << "\nUsage: mgc build <TargetDirectory> <TargetLang> [AssignmentName]\n"
                      << "\nArgs:\n"
                      << "TargetDirectory (Required): Path to the directory created from call to mgc copy. This should contain folders labeled after the students' names.\n"
                      << "TargetLang (Required): This is the target language to build for. Currently supported are \"C\" and \"C++\" or \"CPP\". Specifying these target languages will search your system path for executables \"gcc\" and \"g++\" respectively to build the programs.\n"
                      << "AssignmentName (Optional): This string will be appended to the end of the student's name. This concatenated string will be used as the new name for the executable file.\n"
                      << "\nThis will iterate through each folder contained within the target directory, and use a system() call to invoke a command like \"gcc *.c -o StudentName_Progname\" or \"g++ *.cpp -o StudentName_Progname\" from within the directory opened during each iteration.\n"
                      << std::endl;
        } else if (toupper(std::string(argv[2])) == "COPY") {
            std::cout << "Help: Copy: \n"
                      << "\nDescription: Will take files from within source directory and place them into subfolders in target directory. Directories within target directory path will be created if they do not exist.\n"
                      << "\nUsage: mgc copy <SourceDirectory> <TargetDirectory>\n"
                      << "\nArgs:\n"
                      << "SourceDirectory (Required): Path to the directory extracted from the Moodle assignment submission archive.\n"
                      << "TargetDirectory (Required): Directory that is being created or added to. Subfolders will be added in this directory, each subfolder labeled by the student's name and containing that student's submitted files.\n"
                      << "\nSubfolder names are determined by the name of the source code files. These are expected to follow the naming convention provided with files when the \"Download all submissions\" button on Moodle is clicked. The downloaded archive must be extracted. That extracted directory is what the program expects as the source directory input for this program. All of this copying and filesystem crawling is done using the std::filesystem library, available as of C++17.\n"
                      << std::endl;
        } else {
            std::cout << "Help: Usage: \n"
                      << "\tmgc copy <SourceDirectory> <TargetDirectory>\n"
                      << "\tmgc build <TargetDirectory> <TargetLang> [AssignmentName]\n"
                      << "\tmgc help [copy]|[build]" << std::endl;
        }
        return 0;
    } else if (toupper(std::string(argv[1])) == "COPY") {
        // mgc copy
        if (argc != 4) {
            std::cerr << "Error: Usage: \n"
                      << "\tmgc copy <SourceDirectory> <TargetDirectory>\n" << std::endl;
            exit(1);
        }

        std::string sourceDirName = std::string(argv[2]);
        std::string targetDirName = std::string(argv[3]);

        const std::filesystem::path srcPath{sourceDirName};
        const std::filesystem::path tgtPath{targetDirName};
        if (!std::filesystem::exists(srcPath)) {
            std::cerr << "Error: copy: Source Directory \"" << sourceDirName << "\" does not exist.\n" << std::endl;
            exit(2);
        } else {
            std::cout << "Exists" << std::endl;
        }

        // Execute copy
        errct = mgcCopy(sourceDirName, targetDirName);

    } else if (toupper(std::string(argv[1])) == "BUILD") {
        // mgc build
        if (argc != 4 && argc != 5) {
            std::cerr << "Error: Usage: \n"
                      << "\tmgc build <TargetDirectory> <TargetLang> [AssignmentName]\n" << std::endl;
            exit(1);
        }

        std::string targetDirectory = std::string(argv[2]);
        std::string targetLang = toupper(std::string(argv[3]));
        std::string assignmentName = argc == 5 ? std::string(argv[4]) : "";

        std::filesystem::path targetDirPath(targetDirectory);
        if (!std::filesystem::exists(targetDirPath)) {
            std::cerr << "Error: Target directory \"" << targetDirectory << "\" does not exist.\n" << std::endl;
            exit(2);
        }


        std::vector<std::string> supportedLangs = {
            "C",
            "CPP", "C++"
        };

        bool inList = false;
        for (const std::string s : supportedLangs) {
            if (s == targetLang) {
                inList = true;
            }
        }
        if (!inList) {
            std::cerr << "Error: Target Language \"" << targetLang << "\" is not recognized or is not supported.\n" << std::endl;
            exit(3);
        }

        // Execute build
        errct = mgcBuild(targetDirectory, targetLang, assignmentName);

    } else {
        std::cerr << "Error: Usage: \n"
                  << "\tmgc copy <SourceDirectory> <TargetDirectory>\n"
                  << "\tmgc build <TargetDirectory> <TargetLang> [AssignmentName]\n"
                  << "\tmgc help [copy]|[build]" << std::endl;
    }

    // std::string assignmentName = std::string(argv[1]);

    std::cout << "mgc completed successfully with (" << errct << ") error(s).\n" << std::endl;
    return 0;
}

std::string removeAllWhitespaceAndReplaceComma(std::string src) {
    std::string res = "";
    for (char c : src) {
        if (!isspace(c)) {
            if (c == ',') {
                res += '_';
            } else {
                res += c;
            }       
        }
    }
    return res;
}

std::string toupper(std::string str) {
    std::string newStr(str);
    for (char& c : newStr) {
        c = toupper(c);
    }
    return newStr;
}

int mgcCopy(std::string sourceDirName, std::string targetDirName) {
    int errct = 0;
    std::cout << "Target DirName: " << targetDirName << std::endl;
    // Create overall target directory
    const std::filesystem::path targetDir{targetDirName};
    if (!std::filesystem::create_directories(targetDir)) {
        std::cerr << "Error: could not create or write to target directory \"" << targetDirName << "\"\n" << std::endl;
        exit(2);
    }

    const std::filesystem::path sourceDir{sourceDirName};
    for (auto const& dir_entry : std::filesystem::directory_iterator{sourceDir}) {
        std::string srcPathName = dir_entry.path().filename().string();
        int where = srcPathName.find_first_of('_');
        std::cout << "Checking file \"" << srcPathName << "\"..." << std::endl;
        if (where == std::string::npos) {
            std::cerr << "\tError: Invalid filename format" << std::endl;
            errct++;
            continue;
        }

        // Create target subdirectory
        std::string studName = srcPathName.substr(0, where);
        std::cout << "\tStudName: " << studName << std::endl;
        std::string targetSubDirName = removeAllWhitespaceAndReplaceComma(targetDirName + "/" + studName);
        const std::filesystem::path targetSubDirPath{targetSubDirName};
        std::cout << "\tTarget subdirectory path: \"" << targetSubDirName << "\"" << std::endl;
        std::filesystem::create_directories(targetSubDirPath);

        std::string targetfilename = dir_entry.path().filename().string();
        std::filesystem::path targetFilePath{targetSubDirName + "/" + targetfilename};
        
        // Copy source file into new subdirectory
        std::cout << "\tSource: " << dir_entry.path() << std::endl;
        std::cout << "\tDest: " << targetFilePath << std::endl;
        if (! std::filesystem::copy_file(dir_entry.path(), targetFilePath)) {
            std::cerr << "\tError: Unable to copy file to subdirectory" << std::endl;
            errct++;
        };
    }

    return errct;
}

int mgcBuild(std::string targetDirectory, std::string targetLang, std::string assignmentName) {
    int errct = 0;

    const std::filesystem::path targetDirPath{targetDirectory};
    for (auto const& dir_entry : std::filesystem::directory_iterator{targetDirPath}) {
        const std::filesystem::path subDirPath = dir_entry.path();

        if (!std::filesystem::is_directory(subDirPath)) {
            continue;
        }

        std::string studName =  subDirPath.filename().string();
        std::string targetExeName = studName + (!assignmentName.empty() ? "_" + assignmentName : "");
        std::string cmdStr = "";

        if (targetLang == "C") {
            cmdStr = "gcc \"" + subDirPath.string() + "\"/*.c -o \"" + subDirPath.string() + "/" + targetExeName + "\"";
        } else if (targetLang == "CPP" || targetLang == "C++") {
            cmdStr = "g++ -std=c++17 \"" + subDirPath.string() + "\"/*.cpp -o \"" + subDirPath.string() + "/" + targetExeName + "\"";
        } else {
            std::cerr << "Error: Target Language \"" << targetLang << "\" is not recognized or is not supported.\n" << std::endl;
            exit(3);
        }

        if (!cmdStr.empty()) {
            std::cout << "Executing system(" << cmdStr << ");\n" << std::endl;
            system(cmdStr.c_str());
        } else {
            std::cerr << "Error: could not build command string\n" << std::endl;
            errct++;
        }
    }

    return errct;
}