# my-utils
Simple utility programs which I will use to aid with various tasks

## Building

Each utility has its own makefile that builds into `Utilities/bin/`:

```sh
make -C Utilities/rc
make -C Utilities/mgc
```

Add `Utilities/bin` to your `PATH` to run them from anywhere. `make clean` in either folder removes that utility's binary.

## rc — remote connect

A shortcut for SSHing into SIUE CS machines as `gwitske`.

```sh
rc            # ssh gwitske@home.cs.siue.edu
rc os         # ssh gwitske@os.cs.siue.edu
rc rad        # ssh gwitske@radish.cs.siue.edu (alias for radish)
rc <host>     # ssh gwitske@<host>.cs.siue.edu
rc -h         # show help
```

- Host names may only contain letters, digits, `-` and `.`. Anything else is rejected.
- `rc` runs `ssh` directly (not through a shell) and exits with ssh's exit code.

## mgc — Moodle grading copy/build

Organizes a Moodle "Download all submissions" archive into one folder per student, then compiles each student's code.

```sh
mgc copy <SourceDirectory> <TargetDirectory>
mgc build <TargetDirectory> <TargetLang> [AssignmentName]
mgc help [copy|build]
```

**copy** takes the extracted Moodle archive and copies each submission into `<TargetDirectory>/<StudentName>/`.
- The student name is the part of the file or folder name before the first `_`. Spaces are removed and commas become `_` (e.g. `Smith, John` → `Smith_John`).
- If Moodle made a folder per student, that folder's contents are copied in.
- The target is created if needed. Running `copy` again into the same target overwrites earlier copies.
- Hidden files (like `.DS_Store`) are skipped. Names that don't match the Moodle format are reported as errors.

**build** compiles the sources inside each student folder:
- `C`: all `*.c` files with `gcc`
- `C++` or `CPP`: all `*.cpp`, `*.cc` and `*.cxx` files with `g++ -std=c++17`

The executable is placed in the student's folder as `StudentName` or `StudentName_AssignmentName`. Students with no matching source files, and builds that fail, are reported and counted as errors.

**Exit codes**

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Usage error / unknown command |
| 2 | Bad or missing directory |
| 3 | Unsupported language |
| 4 | Finished, but some files or builds failed |
| 5 | Unexpected filesystem error |

Example:

```sh
mgc copy ~/Downloads/CS140-HW1-submissions ~/Grading/HW1
mgc build ~/Grading/HW1 c hw1
```
