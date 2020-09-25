# diff-dir

## Purpose

`diff-dir` is a command line tool to compare 2 directories and get the differences.
For example, you can use it to see the changes between a directory and its last backup, or to compare 2 btrfs snapshots.

## Features

- compact but detailed view of the differences, ordered by path
- status mode: gives no output, the status code indicates if the directories are equivalent (status=0) or different (status=1) (useful for scripts)
- filter capability to ignore some patterns
- optionally compare metadata: owner (uid) and group (gid), permissions
- use modification time and size of files to avoid comparison of the file content
- multithread capability: different threads can be used to compare the directories and file content to speed-up the comparison (mostly useful on SSD or when metadata is already in cache)

Note on modification time:
- if the files on both sides have the same size and the same modification time, they are assumed to be the same: **the content is NOT checked**.
- if they have the same size but different modification time, the files contents are compared and the file is reported as different only if their content differs.

## Output

The output contains information only on the differences. The files or directories that are common are not displayed.

The output gives on each line:
- a compact information on the difference(s)
- the relative path of the difference

### File type information

The type of the file is encoded on one character
Character | File type
-|-
`f` | regular file
`d` | directory
`l` | symbolic link
`b` | bloc device
`c` | character device
`F` | FIFO
`s` | socket
`-` | no file

### File type difference

When a file exists only on one side, or when the file exists on both sides with different types, the output shows the file type on both sides separated by a `!`.

Example:
```
l ! f  conflicting_type
d ! -  path/to/dir_left_side
- ! f  some_dir/file_right_side
```

### File difference

When the file exists on both sides with the same type, the output shows:
- the common file type on the first letter
- one or several indicators indicating what is different:
  - -/s/c: **s**ize or **c**ontent
  - -/o: **o**wnership (uid or gid)
  - -/p: **p**ermissions

If an indicator has no meaning for a given file type, it is replaced by a space.

Example:
```
f s--  file_with_different_size
f c--  file_with_same_size_different_content
l c-   link_with_different_target
d  o-  /path/to/dir_with_different_ownership
f --p  /path/to/file_same_content_different_permissions
f sop  /path/to/file_with_all_differences
```

## Options

Option | Description
-|-
-h, --help | help message
-v, --version | print version
-s, --status | give no output, return 1 on first identified difference, 0 if no difference found
-i, --ignore path_pattern | ignore paths matching the given pattern - can be set multiple times
-m, --metadata | check and report metadata differences (ownership, permissions)
-t, --thread | use multiple threads to speed-up the comparison
-B, --buffer size | size of the buffers used for content comparison
-d, --debug | print debug information on stderr during the diff

## Build dependencies

- https://github.com/jarro2783/cxxopts lightweight C++ option parser library, included in cxxopts folder
- cmake
- google test framework for the unit tests