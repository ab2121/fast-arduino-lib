This directory contains various documentation material that did not fit anywhere else:

1. How to setup a development environment for FastArduino library or a project using it?
2. Official Coding Guidelines for the project
3. Steps to follow to perform a new release
4. List of header files and their related namespaces
5. List of all ISR registration macros
6. List of all FastArduino examples with their size (code, and data) on all officially supported MCU
7. List of all FastArduino examples with their size (code, and data) **delta** compared with list 6. (this is useful during refactoring in order to check that code size and data size do not increase when they should not)

Most of these documents are manually updated as needed, except sizes lists that get generated automatically by FastArduino build scripts.

All non-text documents are produced with, hence readable by, LibreOffice, no need for a Windows station with M$ Office for these!

Sizes list, generated by FastArduino build scripts, are actually simple text files in tab-delimited format, despite their `.xlsx` extension, and must be opened as such from Libreoffice.