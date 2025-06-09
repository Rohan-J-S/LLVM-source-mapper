# LLVM Source Mapper

A Clang-based CLI tool that maps C++ source code to LLVM IR (Intermediate Representation), allowing developers to see the direct relationship between source lines and their corresponding LLVM instructions.

## What Is This Tool?

If you've ever wondered how your C++ code gets transformed into lower-level instructions that the computer can understand, this tool can help you visualize that process. LLVM IR is an intermediate representation used by the LLVM compiler infrastructure, sitting between your high-level C++ code and machine code.

The LLVM Source Mapper shows you exactly which lines of your C++ code correspond to which LLVM IR instructions. This is useful for:

- Learning how compilers work
- Understanding code optimization
- Debugging complex issues
- Exploring how high-level language features are implemented

## Features

- **Line-by-line mapping**: See exactly which LLVM IR instructions are generated from each line of your C++ code
- **Two output formats**:
  - **Annotated LLVM IR** (`.ll`): The LLVM IR code with comments showing the original source lines
  - **Markdown table view**: A side-by-side comparison of source code and LLVM IR in a readable table format
- **C++ support**: Works with all C++ files (`.cpp`, `.cc`, `.cxx`)
- **Debug information preservation**: Maintains accurate mapping between source and IR
- **Easy-to-use interface**: Simple command-line tool with clear options

## Requirements

Before using this tool, you need to have the following software installed on your system:

### Required Software

- **LLVM and Clang** (version 13.0 or later)
  - These provide the compiler infrastructure that our tool uses
  - On macOS: `brew install llvm`
  - On Ubuntu: `sudo apt install llvm clang`
  - On Windows: Download from the [LLVM Download Page](https://releases.llvm.org/download.html)

- **CMake** (version 3.13.4 or later)
  - This is the build system we use to compile the tool
  - On macOS: `brew install cmake`
  - On Ubuntu: `sudo apt install cmake`
  - On Windows: Download from the [CMake Website](https://cmake.org/download/)

- **C++17 compatible compiler**
  - On macOS: Xcode Command Line Tools (`xcode-select --install`)
  - On Ubuntu: GCC 7+ (`sudo apt install g++`)
  - On Windows: Visual Studio 2019+ or MinGW-w64

### Checking Your Installations

You can verify your installations by running:

```bash
# Check LLVM version
llvm-config --version

# Check Clang version
clang --version

# Check CMake version
cmake --version
```

## Installation Guide

### Step 1: Clone or Download This Repository

```bash
# Using Git
git clone https://github.com/yourusername/llvm-source-mapper.git
cd llvm-source-mapper

# Or download and extract the ZIP file from the repository and navigate to the extracted folder
```

### Step 2: Build the Tool

```bash
# Create a build directory
mkdir -p build
cd build

# Configure with CMake
# This will find your LLVM installation and prepare the build files
cmake -DLLVM_DIR=$(llvm-config --cmakedir) ..

# Build the tool
# This will compile the source code into an executable
make
```

If the build fails with errors about missing LLVM/Clang libraries:

1. Make sure LLVM and Clang are properly installed
2. Try setting the PATH environment variable to include the LLVM bin directory:
   ```bash
   export PATH="/path/to/llvm/bin:$PATH"
   ```
3. On macOS specifically, you might need additional environment variables:
   ```bash
   export LDFLAGS="-L/path/to/llvm/lib"
   export CPPFLAGS="-I/path/to/llvm/include"
   ```

### Step 3: Test Your Installation

```bash
# Make sure you're in the main project directory
cd ..  # If you're still in the build directory

# Make the script executable
chmod +x llvm-source-mapper.sh

# Test with the provided test.cpp file
./llvm-source-mapper.sh test.cpp
```

You should see output showing the LLVM IR mapping for the test file.

## Usage Guide

### Using the Convenient Shell Script

The easiest way to use the Source Mapper is through the provided shell script:

```bash
./llvm-source-mapper.sh <source_file> [--format=ll|md] [--output=filename]
```

#### Options Explained:

- `<source_file>`: The path to your C++ source file (required)
  - Example: `./llvm-source-mapper.sh my_program.cpp`

- `--format=ll|md`: Choose the output format (optional)
  - `ll`: Annotated LLVM IR format (default)
  - `md`: Markdown table format for easier reading
  - Example: `./llvm-source-mapper.sh my_program.cpp --format=md`

- `--output=filename`: Save output to a file instead of displaying it (optional)
  - Example: `./llvm-source-mapper.sh my_program.cpp --output=output.md`

- `--help`: Display help information
  - Example: `./llvm-source-mapper.sh --help`

### Direct Binary Usage (Advanced)

If you prefer, you can use the compiled binary directly:

```bash
./build/source-mapper <source_file> -format=<format> -o=<output_file>
```

Example:
```bash
./build/source-mapper my_program.cpp -format=md -o=output.md
```

## Examples with Explanation

### Example 1: Simple Addition Function

Let's say you have a simple C++ file called `add.cpp`:

```cpp
int add(int a, int b) {
    return a + b;
}
```

Run the tool:
```bash
./llvm-source-mapper.sh add.cpp
```

You'll get output like:

```
; LLVM IR with source mapping

; Source line 2: return a + b;
  %3 = load i32, ptr %1, align 4, !dbg !22  ; Load value of 'a' into register %3
  %4 = load i32, ptr %2, align 4, !dbg !23  ; Load value of 'b' into register %4
  %5 = add nsw i32 %3, %4, !dbg !24         ; Add the values and store in register %5
  ret i32 %5, !dbg !25                      ; Return the result
```

#### Explanation:

1. The first line shows the original C++ code: `return a + b;`
2. The LLVM IR instructions show:
   - Loading the values of variables `a` and `b` from memory
   - Adding them together
   - Returning the result

### Example 2: Using Markdown Format for Readability

For a more readable output, use the Markdown format:

```bash
./llvm-source-mapper.sh add.cpp --format=md
```

This will generate a table like:

| Source Line | Source Code | LLVM IR | Summary |
| ----------: | ----------- | ------- | ------- |
| 2 | `return a + b;` | `%3 = load i32, ptr %1, align 4, !dbg !22<br>%4 = load i32, ptr %2, align 4, !dbg !23<br>%5 = add nsw i32 %3, %4, !dbg !24<br>ret i32 %5, !dbg !25<br>` |  |

### Example 3: Saving the Output to a File

To save the output for later reference:

```bash
./llvm-source-mapper.sh add.cpp --format=md --output=add_mapping.md
```

This will create a file called `add_mapping.md` containing the mapping results.

## Understanding LLVM IR (A Brief Introduction)

LLVM IR might look intimidating at first, but here's a brief guide to understanding the basics:

### Common LLVM IR Instructions

- `alloca`: Allocates memory on the stack
- `load`: Reads a value from memory
- `store`: Writes a value to memory
- `add`, `sub`, `mul`, `div`: Arithmetic operations
- `icmp`: Integer comparison
- `br`: Branch (conditional or unconditional)
- `call`: Function call
- `ret`: Return from function

### Memory Addressing

- `ptr %1`: A pointer to a memory location (often storing a variable)
- `align 4`: Memory alignment information

### Debug Information

- `!dbg !22`: Debug metadata reference (helps map to source locations)

## Common Issues and Solutions

### Problem: "Command not found" when running llvm-config

**Solution**: Make sure LLVM is installed and in your PATH:
```bash
export PATH="/path/to/llvm/bin:$PATH"
```

### Problem: Compilation errors related to C++ standard

**Solution**: Make sure you're using a C++17 compatible compiler:
```bash
# Check your GCC version
g++ --version

# If needed, specify a newer compiler when running CMake
cmake -DCMAKE_CXX_COMPILER=g++-9 ..
```

### Problem: Can't find LLVM libraries during build

**Solution**: Specify the LLVM directory explicitly:
```bash
cmake -DLLVM_DIR=/path/to/llvm/lib/cmake/llvm ..
```

### Problem: The tool doesn't show all my source code

**Solution**: Make sure your code is compiled with debug information:
```bash
# The tool already does this internally, but if you're using custom compilation:
clang++ -g -O0 -c myfile.cpp
```

## Advanced Usage

### Processing Multiple Files

Currently, the tool processes one file at a time. For multiple files, you can:

```bash
# Create a bash loop
for file in *.cpp; do
  ./llvm-source-mapper.sh "$file" --output="${file%.cpp}_ir.md"
done
```

### Working with Complex Projects

For complex projects with multiple files and dependencies:

1. Compile individual files to see their IR mappings
2. Focus on specific functions or areas of interest
3. Use the Markdown output for better readability

## Future Improvements

We plan to enhance this tool with:

- GenAI-based summarization of IR instructions
- Support for additional languages (C, Objective-C)
- Interactive web-based viewer for mappings
- Better handling of optimized code
- Integration with existing LLVM/Clang tools

## Further Learning Resources

- [LLVM Language Reference](https://llvm.org/docs/LangRef.html)
- [Clang Documentation](https://clang.llvm.org/docs/index.html)
- [LLVM for Beginners](https://www.youtube.com/watch?v=J5xExRGaIIY) (YouTube)
- [Compiler Explorer](https://godbolt.org/) - An online tool to see assembly output

## License and Contributing

This project is open-source software.

We welcome contributions! If you'd like to improve this tool:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## Support

If you encounter any issues or have questions, please:

1. Check the common issues section above
2. Open an issue on the GitHub repository
3. Reach out to the maintainers

---

*Note: This tool is primarily educational and designed to help understand the relationship between source code and compiler intermediate representation. It may not capture all optimizations or transformations performed by the full compilation pipeline.*