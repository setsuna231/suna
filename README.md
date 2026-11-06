# suna

> a unified cli for windows. cute names, lethal actions.

`suna` is a fast, no-bloat command-line tool for Windows. It replaces the clunky `Get-ChildItem` / `Remove-Item` nonsense with something that is fast to type, memorable, and actually fun to use. 

It was built out of pure spite because PowerShell takes 800ms to do what a C binary can do in 2ms.

## The Vibe
`suna` is a chibi. Innocent-looking, cute, and absolutely capable of catastrophic destruction. Think Klee from Genshin. The whole vibe is "sweet little creature that will nuke your drive if you type the wrong flag."

## Commands

| Command | Action | Windows Equivalent |
| :--- | :--- | :--- |
| `suna nuke` | Delete (moves to recycle bin by default) | `Remove-Item` |
| `suna rummage` | List directory contents | `Get-ChildItem` |
| `suna peek` | Read file contents | `Get-Content` |
| `suna forage` | Search files and content | `Select-String` / `findstr` |
| `suna poof` | Create a file or directory | `New-Item` |
| `suna scribble` | Edit a file (opens external editor) | `notepad` |
| `suna shove` | Move / rename | `Move-Item` |
| `suna twin` | Copy | `Copy-Item` |

## The Nuke Protocol
`suna nuke` is designed to be safe by default, but powerful when you need it.

- **Default:** Moves files to the Windows Recycle Bin.
- **`--obliterate`:** Bypasses the Recycle Bin and permanently deletes the file.
- **`-i-am-fucking-sure`:** Required *only* if the target is a system file, a drive root, or inside your System/User PATH. It forces you to explicitly acknowledge that you are about to do something incredibly stupid.

**WARNING:** `suna nuke --obliterate` does not securely erase data. Forensic tools can still recover the files. Do not use this for sensitive data destruction.

## Installation

### Prerequisites
- **MSYS2 (UCRT64)** environment with `gcc` and `make`.
- Windows 10 or 11.

### Build from Source
1. Clone the repository:
   `git clone https://github.com/setsuna231/suna.git`
2. Navigate into the directory:
   `cd suna`
3. Build the executable:
   `mingw32-make`
4. The binary will be located at `build/suna.exe`. Add this folder to your System PATH so you can call `suna` from anywhere.

## Examples

**Delete a file (safely):**
`suna nuke useless.txt`

**Delete a folder permanently (requires flags):**
`suna nuke C:\Windows\Temp --obliterate -i-am-fucking-sure`

**List all C files in a directory:**
`suna rummage src/*.c`

**Search for a string inside your code:**
`suna forage "TODO" src/`

**Copy multiple files into a folder:**
`suna twin *.txt backup_folder/`

## License
MIT