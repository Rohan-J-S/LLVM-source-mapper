#!/usr/bin/env python3
"""
This script adds summaries to the LLVM IR instructions in the output Markdown file
using the Groq Cloud API.
"""

import os
import sys
import re
import json
import requests
from typing import List, Dict, Any, Optional
import argparse

# Load environment variables from .env file
try:
    from dotenv import load_dotenv
    load_dotenv()
except ImportError:
    print("Warning: python-dotenv is not installed. .env file will not be loaded.")

# Configure Groq API credentials
GROQ_API_KEY = os.environ.get('GROQ_API_KEY', '')
GROQ_API_ENDPOINT = "https://api.groq.com/openai/v1/chat/completions"
MODEL = os.environ.get('GROQ_MODEL', 'llama3-70b-8192')  # Use environment variable or default to llama3-70b model

def setup_args() -> argparse.Namespace:
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Add Groq-generated summaries to LLVM IR mappings')
    parser.add_argument('input_file', help='Input markdown file with LLVM IR mappings')
    parser.add_argument('output_file', help='Output markdown file with summaries')
    parser.add_argument('--api-key', help='Groq API Key (can also be set via GROQ_API_KEY env var)')
    parser.add_argument('--model', help='Groq model to use (can also be set via GROQ_MODEL env var)')
    return parser.parse_args()

def extract_llvm_sections(file_path: str) -> Dict[int, Dict[str, Any]]:
    """Extract source line, source code and LLVM IR instructions from the markdown file."""
    with open(file_path, 'r') as f:
        content = f.read()

    # Pattern to extract table rows with source line, code, and LLVM IR instructions
    pattern = r'\|\s*(\d+)\s*\|\s*`([^`]*)`\s*\|\s*<pre>(.*?)</pre>\s*\|\s*(.*?)\s*\|'
    matches = re.findall(pattern, content, re.DOTALL)

    sections = {}
    for match in matches:
        line_num = int(match[0])
        source_code = match[1].strip()
        llvm_ir = match[2].strip()
        sections[line_num] = {
            'source_code': source_code,
            'llvm_ir': llvm_ir,
            'summary': ''
        }
    
    return sections

def generate_summary(source_code: str, llvm_ir: str) -> str:
    """Generate a summary of LLVM IR instructions using Groq API"""
    if not GROQ_API_KEY:
        print("Warning: GROQ_API_KEY is not set. Cannot generate summaries.")
        return ""
    
    headers = {
        "Authorization": f"Bearer {GROQ_API_KEY}",
        "Content-Type": "application/json"
    }
    
    prompt = f"""
    I have some LLVM IR code that was generated from C++ source code. Please provide a brief, 
    technical explanation of what these instructions are doing in 1-2 sentences.
    
    C++ source code:
    ```cpp
    {source_code}
    ```
    
    LLVM IR code:
    ```llvm
    {llvm_ir}
    ```
    
    Please provide ONLY the technical explanation without any lead-in phrases like "This code..." or "The LLVM IR...".
    Be very concise (15-25 words) and technically precise. Focus on what the LLVM IR is doing, not the C++ code. Make sure to explain each line of the LLVM IR
    """
    
    data = {
        "model": MODEL,
        "messages": [{"role": "user", "content": prompt}],
        "temperature": 0.2,  # Lower temperature for more deterministic, focused responses
        "max_tokens": 200
    }
    
    try:
        response = requests.post(GROQ_API_ENDPOINT, headers=headers, json=data)
        response.raise_for_status()  # Raise exception for non-2xx responses
        
        result = response.json()
        summary = result["choices"][0]["message"]["content"].strip()
        
        # Remove any markdown formatting from the summary
        summary = re.sub(r'^```.*\n', '', summary)
        summary = re.sub(r'\n```$', '', summary)
        
        return summary
    except Exception as e:
        print(f"Error generating summary: {e}")
        return ""

def update_markdown_with_summaries(input_file: str, output_file: str, sections: Dict[int, Dict[str, Any]]) -> None:
    """Update the markdown file with generated summaries"""
    with open(input_file, 'r') as f:
        content = f.read()
    
    for line_num, section in sections.items():
        if section['summary']:
            # Pattern to replace the empty summary in the markdown table
            pattern = r'(\|\s*{}\s*\|\s*`[^`]*`\s*\|\s*<pre>.*?</pre>\s*\|\s*)(.*?)(\s*\|)'.format(line_num)
            replacement = r'\1{}\3'.format(section['summary'])
            content = re.sub(pattern, replacement, content, flags=re.DOTALL)
    
    with open(output_file, 'w') as f:
        f.write(content)

def write_html_output(sections: Dict[int, Dict[str, Any]], output_file: str) -> None:
    """Write the mapping and summaries as a styled HTML table."""
    with open(output_file, 'w') as f:
        f.write('<!DOCTYPE html>\n<html lang="en">\n<head>\n')
        f.write('  <meta charset="UTF-8">\n')
        f.write('  <title>LLVM Source to IR Mapping with Summaries</title>\n')
        f.write('  <style>\n')
        f.write('    body { font-family: \'Segoe UI\', Arial, sans-serif; background: #f9f9f9; margin: 0; padding: 2em; }\n')
        f.write('    table { width: 100%; border-collapse: collapse; background: #fff; box-shadow: 0 2px 8px #0001; }\n')
        f.write('    th, td { border: 1px solid #ddd; padding: 0.75em 1em; vertical-align: top; }\n')
        f.write('    th { background: #f0f0f0; font-weight: 600; }\n')
        f.write('    tr:nth-child(even) { background: #fafbfc; }\n')
        f.write('    code, pre { font-family: \'Fira Mono\', \'Consolas\', \'Menlo\', monospace; font-size: 0.98em; }\n')
        f.write('    pre { background: #f6f8fa; padding: 0.5em 1em; border-radius: 4px; margin: 0; }\n')
        f.write('    .src { color: #005cc5; }\n')
        f.write('    .summary { color: #22863a; }\n')
        f.write('    th:last-child, td:last-child { width: 22%; min-width: 200px; }\n')
        f.write('  </style>\n')
        f.write('</head>\n<body>\n')
        f.write('  <h1>LLVM Source to IR Mapping with Summaries</h1>\n')
        f.write('  <table>\n    <thead>\n      <tr>\n        <th>Source Line</th>\n        <th>Source Code</th>\n        <th>LLVM IR</th>\n        <th>Summary</th>\n      </tr>\n    </thead>\n    <tbody>\n')
        for line_num in sorted(sections.keys()):
            section = sections[line_num]
            f.write('      <tr>\n')
            f.write(f'        <td>{line_num}</td>\n')
            f.write(f'        <td><code>{section["source_code"].replace("<", "&lt;").replace(">", "&gt;")}</code></td>\n')
            f.write(f'        <td><pre>{section["llvm_ir"].replace("<", "&lt;").replace(">", "&gt;")}</pre></td>\n')
            f.write(f'        <td class="summary">{section["summary"]}</td>\n')
            f.write('      </tr>\n')
        f.write('    </tbody>\n  </table>\n</body>\n</html>\n')

def main() -> None:
    """Main function to add summaries to LLVM IR instructions"""
    args = setup_args()
    
    # Set API key from command line if provided
    global GROQ_API_KEY, MODEL
    if args.api_key:
        GROQ_API_KEY = args.api_key
        
    # Set model from command line if provided
    if args.model:
        MODEL = args.model
    
    print(f"Using Groq model: {MODEL}")
    
    if not GROQ_API_KEY:
        print("Warning: No Groq API key provided. Please set GROQ_API_KEY environment variable or use --api-key.")
    
    print(f"Processing {args.input_file}...")
    
    # Extract LLVM IR sections
    sections = extract_llvm_sections(args.input_file)
    print(f"Found {len(sections)} source lines with LLVM IR instructions.")
    
    # Generate summaries
    for line_num, section in sections.items():
        print(f"Generating summary for line {line_num}...")
        section['summary'] = generate_summary(section['source_code'], section['llvm_ir'])
    
    # Choose output format based on file extension
    if args.output_file.endswith('.html'):
        write_html_output(sections, args.output_file)
        print(f"HTML output written to {args.output_file}")
    else:
        update_markdown_with_summaries(args.input_file, args.output_file, sections)
        print(f"Summaries added to {args.output_file}")

if __name__ == "__main__":
    main()
