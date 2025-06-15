#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <curl/curl.h> // For HTTP requests to Groq API
#include <nlohmann/json.hpp> // For JSON parsing/serialization

#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DebugInfoMetadata.h" // Add explicit include for DILocation
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/DebugInfo/DIContext.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"

// Define using directive for nlohmann::json
using json = nlohmann::json;

// Using explicit namespace qualifiers instead of using namespace directives
// to avoid ambiguous references

// Command line options
static llvm::cl::OptionCategory SourceMapperCategory("Source Mapper Options");
static llvm::cl::opt<std::string> OutputFormat(
    "format",
    llvm::cl::desc("Output format: 'll' for annotated LLVM IR or 'md' for Markdown side-by-side view"),
    llvm::cl::init("ll"),
    llvm::cl::cat(SourceMapperCategory));

static llvm::cl::opt<bool> EnableGenAISummary(
    "summarize",
    llvm::cl::desc("Enable GenAI-based summary of IR instructions (experimental)"),
    llvm::cl::init(false),
    llvm::cl::cat(SourceMapperCategory));

static llvm::cl::opt<std::string> OutputFile(
    "o",
    llvm::cl::desc("Output file path"),
    llvm::cl::value_desc("filename"),
    llvm::cl::cat(SourceMapperCategory));

// Helper class to manage temporary files
class TempFileManager {
private:
    std::vector<std::string> tempFiles;

public:
    ~TempFileManager() {
        for (const auto& file : tempFiles) {
            llvm::sys::fs::remove(file);
        }
    }

    std::string createTempFile(const std::string& prefix, const std::string& suffix) {
        llvm::SmallString<128> tempFilePath;
        std::error_code EC = llvm::sys::fs::createTemporaryFile(prefix, suffix, tempFilePath);
        if (EC) {
            llvm::errs() << "Error creating temporary file: " << EC.message() << "\n";
            exit(1);
        }
        tempFiles.push_back(tempFilePath.str().str());
        return tempFilePath.str().str();
    }
};

// Structure to store source line to IR mapping
struct SourceLineMapping {
    int sourceLine;
    std::string sourceFile;
    std::string sourceLineContent;
    std::vector<std::string> irInstructions;
    std::string summary; // For GenAI summary
};

// Helper function to run shell commands
std::string executeCommand(const std::string& command) {
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        llvm::errs() << "Error executing command: " << command << "\n";
        return "";
    }
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    return result;
}

// Read file content into a string
std::string readFileContent(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        llvm::errs() << "Error opening file: " << filePath << "\n";
        return "";
    }
    
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    return buffer.str();
}

// Get specific line from file content
std::string getLineFromFile(const std::string& filePath, int lineNum) {
    std::ifstream file(filePath);
    std::string line;
    int currentLine = 1;
    
    while (std::getline(file, line)) {
        if (currentLine == lineNum) {
            return line;
        }
        currentLine++;
    }
    
    return "";
}

// Generate LLVM IR with debug info from source file
std::string generateIR(const std::string& sourceFile, TempFileManager& tempManager) {
    std::string irFile = tempManager.createTempFile("ir", ".ll");
    
    // Generate LLVM IR with debug information
    std::string command = "clang++ -S -emit-llvm -g -O0 -o " + irFile + " " + sourceFile;
    std::string output = executeCommand(command);
    
    if (!llvm::sys::fs::exists(irFile)) {
        llvm::errs() << "Failed to generate IR file\n";
        llvm::errs() << "Command output: " << output << "\n";
        exit(1);
    }
    
    return irFile;
}

// Extract debug info and map source lines to IR instructions
std::map<int, SourceLineMapping> mapSourceToIR(const std::string& irFile, const std::string& sourceFile) {
    std::map<int, SourceLineMapping> lineMapping;
    
    // Initialize LLVM context and load IR file
    llvm::LLVMContext context;
    llvm::SMDiagnostic err;
    std::unique_ptr<llvm::Module> module = llvm::parseIRFile(irFile, err, context);
    
    if (!module) {
        llvm::errs() << "Error parsing IR file: " << err.getMessage() << "\n";
        exit(1);
    }
    
    // Get the source file basename for comparison
    std::string sourceBasename = llvm::sys::path::filename(sourceFile).str();
    
    // Iterate through all functions and instructions to collect debug info
    for (auto &F : *module) {
        if (F.isDeclaration()) continue;
        
        for (auto &BB : F) {
            for (auto &I : BB) {
                const llvm::DebugLoc &debugLoc = I.getDebugLoc();
                if (debugLoc) {
                    unsigned line = debugLoc.getLine();
                    
                    // Get the filename through DILocation
                    const llvm::DILocation *DILoc = debugLoc.get();
                    std::string filename = "";
                    if (DILoc) {
                        filename = DILoc->getFilename().str();
                    }
                    
                    // Only include instructions from the original source file
                    if (filename == sourceBasename) {
                        std::string instStr;
                        llvm::raw_string_ostream rso(instStr);
                        I.print(rso);
                        
                        if (lineMapping.find(line) == lineMapping.end()) {
                            SourceLineMapping mapping;
                            mapping.sourceLine = line;
                            mapping.sourceFile = filename;
                            mapping.sourceLineContent = getLineFromFile(sourceFile, line);
                            lineMapping[line] = mapping;
                        }
                        
                        lineMapping[line].irInstructions.push_back(instStr);
                    }
                }
            }
        }
    }
    
    return lineMapping;
}

// Output in annotated LLVM IR format
void outputAnnotatedIR(const std::map<int, SourceLineMapping>& lineMapping, std::ostream& out) {
    out << "; LLVM IR with source mapping\n\n";
    
    for (const auto& entry : lineMapping) {
        int line = entry.first;
        const SourceLineMapping& mapping = entry.second;
        
        out << "\n; Source line " << line << ": " << mapping.sourceLineContent << "\n";
        
        for (const auto& inst : mapping.irInstructions) {
            out << inst << "\n";
        }
        
        if (!mapping.summary.empty()) {
            out << "; Summary: " << mapping.summary << "\n";
        }
    }
}

// Output in Markdown side-by-side format with improved formatting
void outputMarkdownView(const std::map<int, SourceLineMapping>& lineMapping, std::ostream& out) {
    out << "# Source to LLVM IR Mapping\n\n";
    
    // Define column widths for better readability
    out << "<style>\n";
    out << "table {\n  width: 100%;\n  table-layout: fixed;\n  overflow-wrap: break-word;\n}\n";
    out << "th:first-child {\n  width: 10%;\n}\n";
    out << "th:nth-child(2) {\n  width: 20%;\n}\n";
    out << "th:nth-child(3) {\n  width: 60%;\n}\n";
    out << "th:last-child {\n  width: 10%;\n}\n";
    out << "</style>\n\n";
    
    out << "| Source Line | Source Code | LLVM IR | Summary |\n";
    out << "| ----------: | ----------- | ------- | ------- |\n";
    
    for (const auto& entry : lineMapping) {
        int line = entry.first;
        const SourceLineMapping& mapping = entry.second;
        
        // Escape pipe characters in the source to maintain Markdown table structure
        std::string escapedSource = mapping.sourceLineContent;
        std::string::size_type pos = 0;
        while ((pos = escapedSource.find("|", pos)) != std::string::npos) {
            escapedSource.replace(pos, 1, "\\|");
            pos += 2;
        }
        
        // Output the source line and source code
        out << "| " << line << " | `" << escapedSource << "` | ";
        
        // Handle the LLVM IR instructions - each on its own line with proper formatting
        if (!mapping.irInstructions.empty()) {
            out << "<pre>";
            for (size_t i = 0; i < mapping.irInstructions.size(); i++) {
                std::string inst = mapping.irInstructions[i];
                
                // Escape pipe characters in each instruction
                std::string::size_type irPos = 0;
                while ((irPos = inst.find("|", irPos)) != std::string::npos) {
                    inst.replace(irPos, 1, "\\|");
                    irPos += 2;
                }
                
                // Add the instruction with proper indentation
                out << inst;
                
                // Add a line break if not the last instruction
                if (i < mapping.irInstructions.size() - 1) {
                    out << "\n";
                }
            }
            out << "</pre>";
        } else {
            out << " ";
        }
        
        // Add summary column
        out << " | ";
        if (!mapping.summary.empty()) {
            out << mapping.summary;
        }
        out << " |\n";
    }
}

// Main entry point
int main(int argc, const char **argv) {
    auto ExpectedParser = clang::tooling::CommonOptionsParser::create(argc, argv, SourceMapperCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    clang::tooling::CommonOptionsParser& OptionsParser = ExpectedParser.get();
    
    // Get the source files from the command line
    const auto& sourcePaths = OptionsParser.getSourcePathList();
    if (sourcePaths.empty()) {
        llvm::errs() << "Error: No source files specified.\n";
        return 1;
    }
    
    // Currently, we only handle one source file at a time
    std::string sourceFile = sourcePaths[0];
    if (sourcePaths.size() > 1) {
        llvm::errs() << "Warning: Only processing the first source file: " << sourceFile << "\n";
    }
    
    // Check if the input file is a C++ file
    std::string extension = llvm::sys::path::extension(sourceFile).str();
    if (extension != ".cpp" && extension != ".cc" && extension != ".cxx") {
        llvm::errs() << "Error: Only C++ files (.cpp, .cc, .cxx) are supported.\n";
        return 1;
    }
    
    // Create a manager for temporary files
    TempFileManager tempManager;
    
    // Generate LLVM IR with debug info
    std::string irFile = generateIR(sourceFile, tempManager);
    
    // Map source lines to IR
    auto lineMapping = mapSourceToIR(irFile, sourceFile);
    
    // If GenAI summary is enabled, this would be where we add summaries
    if (EnableGenAISummary) {
        // This is a placeholder for actual GenAI integration
        llvm::errs() << "Note: GenAI summary is enabled but not implemented in this version.\n";
    }
    
    // Determine output stream
    std::ofstream outputFileStream;
    std::ostream* out = &std::cout;
    
    if (!OutputFile.empty()) {
        outputFileStream.open(OutputFile);
        if (!outputFileStream.is_open()) {
            llvm::errs() << "Error: Could not open output file: " << OutputFile << "\n";
            return 1;
        }
        out = &outputFileStream;
    }
    
    // Generate output in the specified format
    if (OutputFormat == "md") {
        outputMarkdownView(lineMapping, *out);
    } else {
        // Default to 'll' format
        outputAnnotatedIR(lineMapping, *out);
    }
    
    // Close output file if needed
    if (outputFileStream.is_open()) {
        outputFileStream.close();
    }
    
    return 0;
}