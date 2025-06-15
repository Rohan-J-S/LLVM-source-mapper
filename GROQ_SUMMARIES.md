# LLVM Source Mapper with Groq AI-Generated Summaries

This document explains how to use the LLVM Source Mapper with Groq AI to generate summaries of LLVM IR instructions.

## Prerequisites

To use the Groq AI summaries feature, you need:

1. Everything required for the regular LLVM Source Mapper
2. Python 3.6+
3. The `requests` Python package (`pip install requests`)
4. A Groq API key (sign up at [https://console.groq.com](https://console.groq.com))

## Setting Up Your API Key

You have two options for setting your Groq API key:

### Option 1: Using the .env file (recommended)

1. Create a `.env` file in the project root:
   ```
   # Groq API configuration
   GROQ_API_KEY=your_groq_api_key_here
   
   # Optional: Model configuration
   GROQ_MODEL=llama3-70b-8192  # Default model to use for summaries
   # Uncomment to use a different model
   # GROQ_MODEL=mixtral-8x7b-32768
   ```

2. The script will automatically load this file when run

### Option 2: Environment variable

Alternatively, you can set your API key as an environment variable:
```bash
export GROQ_API_KEY='your-groq-api-key'
```

## How to Generate Summaries

Run the `generate_with_summaries.sh` script:
```bash
./generate_with_summaries.sh test.cpp --output=test_with_summaries.md --model=llama3-70b-8192
```

This will:
- Run the LLVM Source Mapper to generate the initial mapping
- Call the Groq API to generate summaries for each LLVM IR instruction set
- Save the result to the specified output file

## Command Line Options

- `<source_file>`: C++ source file to analyze (required)
- `--output=filename`: Output file (default: output_with_summaries.md)
- `--model=model_name`: Groq model to use (default: from .env file or llama3-70b-8192)

## Available Models

The following Groq models are available for generating summaries:

- `llama3-70b-8192` (default): Recommended for best quality summaries
- `llama3-8b-8192`: Faster but less detailed summaries
- `mixtral-8x7b-32768`: Good alternative with different training data

```
./generate_with_summaries.sh <source_file> [--output=filename]
```

- `<source_file>`: The C++ source file to process
- `--output=filename`: (Optional) Output file name (default: output_with_summaries.md)

## How It Works

1. The script first runs the standard LLVM Source Mapper to generate a Markdown file with source-to-IR mappings
2. Then it extracts each source line and its corresponding LLVM IR
3. It sends these pairs to the Groq API with a prompt to explain what the IR instructions do
4. The summaries are added to the Summary column in the Markdown table
5. The final result is saved to the specified output file

## Example Summary

For a C++ line like `return a + b;`, the Groq API might generate a summary like:

"Loads values from memory, performs integer addition, returns the result"

This helps developers understand what the LLVM IR instructions are doing without having to analyze them manually.

## Troubleshooting

- **No summaries generated**: Check that your Groq API key is set correctly and that you have internet connectivity
- **Python errors**: Make sure Python 3.6+ is installed and you have the `requests` package
- **Rate limits**: The Groq API has rate limits. If you're processing a large file, you might hit these limits

## Advanced Usage

You can also use the Python script directly:

```bash
python3 add_groq_summaries.py input.md output.md --api-key=your-api-key
```

This is useful if you already have a Markdown file with LLVM IR mappings and just want to add summaries.
