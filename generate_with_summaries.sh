#!/bin/bash

# generate_with_summaries.sh
# This script runs the LLVM Source Mapper and then adds Groq-generated summaries to the output

# Show usage if no arguments provided
if [ $# -lt 1 ]; then
    echo "Usage: ./generate_with_summaries.sh <source_file> [--output=filename] [--model=model_name]"
    echo ""
    echo "Options:"
    echo "  <source_file>      C++ source file (.cpp, .cc, .cxx)"
    echo "  --output=filename  Output file (default: output_with_summaries.md)"
    echo "  --model=model_name Groq model to use (default from .env or llama3-70b-8192)"
    echo ""
    echo "Note: API key is loaded from .env file or GROQ_API_KEY environment variable."
    exit 1
fi

# Default output file
SOURCE_FILE=""
OUTPUT_FILE="output_with_summaries.md"
TEMP_OUTPUT="temp_output.md"
MODEL=""

# Parse arguments
for arg in "$@"; do
    if [[ $arg == --output=* ]]; then
        OUTPUT_FILE="${arg#*=}"
    elif [[ $arg == --model=* ]]; then
        MODEL="${arg#*=}"
    else
        # Assume it's the source file
        SOURCE_FILE="$arg"
    fi
done

# Check if source file is provided
if [ -z "$SOURCE_FILE" ]; then
    echo "Error: No source file specified."
    exit 1
fi

# Check if source file exists
if [ ! -f "$SOURCE_FILE" ]; then
    echo "Error: Source file '$SOURCE_FILE' does not exist."
    exit 1
fi

# Check if Python 3 is installed
if ! command -v python3 &> /dev/null; then
    echo "Error: Python 3 is required but not installed. Please install Python 3."
    exit 1
fi

# Check if required Python packages are installed
python3 -c "import requests" &> /dev/null
if [ $? -ne 0 ]; then
    echo "Installing required Python packages..."
    pip3 install requests
fi

# Load API key from .env file if it exists
if [ -f ".env" ]; then
    echo "Loading API key from .env file..."
    export $(grep -v '^#' .env | xargs)
fi

# Check if GROQ_API_KEY is set
if [ -z "$GROQ_API_KEY" ]; then
    echo "Warning: GROQ_API_KEY is not set in .env file or environment."
    echo "Please add your API key to the .env file or set it with: export GROQ_API_KEY='your-api-key'"
    echo "Continuing without API key - summaries will not be generated."
fi

echo "Generating LLVM IR mapping for $SOURCE_FILE..."

# Run the LLVM Source Mapper to generate Markdown output
./llvm-source-mapper.sh "$SOURCE_FILE" --format=md --output="$TEMP_OUTPUT"

if [ $? -ne 0 ]; then
    echo "Error: Failed to run LLVM Source Mapper."
    exit 1
fi

echo "Adding summaries using Groq API..."

# Run the Python script to add summaries
if [ -n "$MODEL" ]; then
    echo "Using model: $MODEL"
    python3 add_groq_summaries.py "$TEMP_OUTPUT" "$OUTPUT_FILE" --model="$MODEL"
else
    # Use default model from .env or hardcoded default in the Python script
    python3 add_groq_summaries.py "$TEMP_OUTPUT" "$OUTPUT_FILE"
fi

if [ $? -ne 0 ]; then
    echo "Error: Failed to add summaries."
    # Save the intermediate file in case of error
    mv "$TEMP_OUTPUT" "${OUTPUT_FILE}.no_summaries"
    echo "Intermediate file saved as ${OUTPUT_FILE}.no_summaries"
    exit 1
fi

# Remove the temporary file
rm -f "$TEMP_OUTPUT"

echo "Done! Output saved to $OUTPUT_FILE"
