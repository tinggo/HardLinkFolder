# HardLinkFolder
Recursive hard link all files within a folder and multithread employed

1. Using STL API only, so all platform should be supported.
2. Requires C++17 minimum.
3. Auto detect thread count by taking the return value of STL API hardware_concurrency.

Usage:
HardLinkFolder [source path] [linkage path]
