# Prototype MFC File Hasher

This project was created largely as a GUI wrapper for some file hashing classes that I wrote, and to act as a prototype so I could figure out the architecture for a larger scale project which will use MFCs SDI interface rather than relying on simple dialogs, and will be built as a shell extension to file explorer. Though that project remains a work in progress.

## Features

The program is very simple, select a directory using the menu and click on a file to see the hash values for that file. It should be robust enough to handle files of any size, at least I've tested it against the largest files I could find on my system and not run into any problems.
![image](https://private-user-images.githubusercontent.com/46231321/630830683-f52f840d-cce2-4982-8094-3de98fae06dd.png?jwt=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3ODU3OTQyMTcsIm5iZiI6MTc4NTc5MzkxNywicGF0aCI6Ii80NjIzMTMyMS82MzA4MzA2ODMtZjUyZjg0MGQtY2NlMi00OTgyLTgwOTQtM2RlOThmYWUwNmRkLnBuZz9YLUFtei1BbGdvcml0aG09QVdTNC1ITUFDLVNIQTI1NiZYLUFtei1DcmVkZW50aWFsPUFLSUFWQ09EWUxTQTUzUFFLNFpBJTJGMjAyNjA4MDMlMkZ1cy1lYXN0LTElMkZzMyUyRmF3czRfcmVxdWVzdCZYLUFtei1EYXRlPTIwMjYwODAzVDIxNTE1N1omWC1BbXotRXhwaXJlcz0zMDAmWC1BbXotU2lnbmF0dXJlPTdjYTMyM2RhM2M5NDY3OTdjYjk0MDIxNDc2YzVkOTFjZTJlMzMxMDU2MGUwODRiZTQ1ZTg3YmFhZjQxMWJlNDcmWC1BbXotU2lnbmVkSGVhZGVycz1ob3N0JnJlc3BvbnNlLWNvbnRlbnQtdHlwZT1pbWFnZSUyRnBuZyJ9.3D62QJrE2BdR0-n2p74KanCSHYnjFkZd9P_ltgP1EL0)
## Build Dependencies

The GUI is built using Microsoft Foundation Classes, so it is only buildable on Windows, and will require the following:
1. Visual Studio with the "Desktop Development with C++" workload and the "C++ MFC for x64/x86" optional feature (note that this is not selected by default).
2. The latest version of [CMake](https://cmake.org/download/).

## Build Instructions

1. Clone the repo
```shell
git clone https://github.com/Prendem/MFCHashFolders.git
```
2. Run the build script, this will generate both the Visual Studio solution files and compile the application
```Powershell
.\build.ps1
```
