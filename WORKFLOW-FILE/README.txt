WHY THIS FOLDER EXISTS
----------------------
The automatic build system needs the file build-windows.yml to live at this
exact path in your GitHub repo:

    .github/workflows/build-windows.yml

That ".github" folder is included in this zip, but web browsers sometimes
skip folders starting with a dot when you drag files into GitHub.

AFTER UPLOADING EVERYTHING, CHECK:
If your repo's Actions tab shows a "Get started with GitHub Actions" welcome
page instead of a build, the .github folder was skipped. Fix it like this:

1. In your repo click:  Add file -> Create new file
2. In the name box type exactly:  .github/workflows/build-windows.yml
   (typing each / creates a folder)
3. Open the build-windows.yml file in THIS folder, copy all of it,
   paste it into the editor on GitHub.
4. Click "Commit changes". The build starts by itself.

You can delete the WORKFLOW-FILE folder from the repo afterwards (or just
leave it - it does no harm).
