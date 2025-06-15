# Complete Guide: Pushing LLVM Source Mapper to GitHub

## Prerequisites
- GitHub account
- Git installed on your machine
- Repository already initialized (this has been done)

## Step 1: Create Repository on GitHub
1. Go to [GitHub.com](https://github.com)
2. Click the "+" icon in the top right corner
3. Select "New repository"
4. Repository name: `llvm-source-mapper`
5. Description: `A tool to map C++ source code lines to their corresponding LLVM IR instructions`
6. Keep it **Public** (or Private if you prefer)
7. **DO NOT** initialize with README, .gitignore, or license (we already have these)
8. Click "Create repository"

## Step 2: Connect Local Repository to GitHub

```bash
# Navigate to your project directory
cd /Users/samvitgersappa/Projects/CD_Lab_EL/llvm-source-mapper

# Add GitHub remote (replace YOUR_USERNAME with your actual GitHub username)
git remote add origin https://github.com/YOUR_USERNAME/llvm-source-mapper.git

# Verify the remote was added
git remote -v
```

## Step 3: Push to GitHub

```bash
# Push to GitHub (first time)
git push -u origin main

# For subsequent pushes, you can just use:
# git push
```

## Step 4: Verify Upload
1. Go to your GitHub repository page
2. Verify all files are there:
   - `README.md`
   - `CMakeLists.txt`
   - `llvm-source-mapper.sh`
   - `src/SourceMapper.cpp`
   - `test.cpp`
   - `COMMANDS.txt`
   - `.gitignore`

## Alternative: Using SSH (Recommended for frequent use)

If you have SSH keys set up with GitHub:

```bash
# Add SSH remote instead
git remote add origin git@github.com:YOUR_USERNAME/llvm-source-mapper.git

# Push using SSH
git push -u origin main
```

## Future Updates

After making changes to your code:

```bash
# Add changes
git add .

# Commit with descriptive message
git commit -m "Your commit message describing the changes"

# Push to GitHub
git push
```

## Troubleshooting

### If you get authentication errors:
1. Make sure you're using your correct GitHub username
2. Use a Personal Access Token instead of password if prompted
3. Or set up SSH keys for easier authentication

### If remote already exists:
```bash
# Remove existing remote and add the correct one
git remote remove origin
git remote add origin https://github.com/YOUR_USERNAME/llvm-source-mapper.git
```

### If you need to rename the default branch:
```bash
# GitHub now defaults to 'main', but older repos might use 'master'
git branch -M main
```

## Repository Structure That Will Be Uploaded

```
llvm-source-mapper/
├── README.md                 # Project documentation
├── CMakeLists.txt           # Build configuration
├── llvm-source-mapper.sh    # CLI wrapper script
├── COMMANDS.txt             # Complete command guide
├── .gitignore              # Files to ignore in git
├── src/
│   └── SourceMapper.cpp     # Main source code
├── include/                 # Header files (if any)
└── test.cpp                # Example test file
```

**Note:** The `build/` directory and generated files are excluded via `.gitignore` and won't be uploaded to GitHub.
