Run the helper script `push_to_github.ps1` to commit and push the project to your GitHub repo.

Usage:

1. Open PowerShell and change to the project root:

```powershell
cd "C:\Users\ASUS\Projects\matching_engine_cpp"
Run the helper scripts to commit and push the project to your GitHub repo.

SSH-based (recommended if you have an SSH key):

```powershell
cd "C:\Users\ASUS\Projects\matching_engine_cpp"
.\scripts\push_to_github.ps1 -RemoteUrl "git@github.com:AdityaMehta2006/Matching_engine.git" -CommitMessage "Add project files"
```

To overwrite an existing `origin` remote:

```powershell
.\scripts\push_to_github.ps1 -ForceRemote -RemoteUrl "git@github.com:AdityaMehta2006/Matching_engine.git"
```

HTTPS-based (use this if you prefer HTTPS or do not have SSH set up):

```powershell
cd "C:\Users\ASUS\Projects\matching_engine_cpp"
.\scripts\push_to_github_https.ps1 -RemoteUrl "https://github.com/AdityaMehta2006/Matching_engine.git" -CommitMessage "Add project files"
```

Notes:
- Ensure `git` is installed on your machine: https://git-scm.com/download/win
- For SSH auth: add your public key to GitHub (https://github.com/settings/ssh/new)
- For HTTPS auth: create a Personal Access Token (PAT) with `repo` scope and use it as your password when prompted, or install Git Credential Manager to cache credentials.

If you run either script and get an error, paste the exact output here and I'll help you resolve it.
