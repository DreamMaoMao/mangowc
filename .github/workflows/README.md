# GitHub Actions Workflows

This directory contains the GitHub Actions workflows for the MangoWC project.

## Workflows

### build.yml

**Purpose**: Builds the project to ensure code changes compile successfully.

**Triggers**:

- Push to `main` or `master` branch (only when code files change)
- Pull requests to `main` or `master` branch (only when code files change)
- Manual dispatch (workflow_dispatch)

**Path filters** (only runs when these change):

- Source files: `**.c`, `**.h`, `**.cpp`, `**.scm`
- Build files: `meson.build`, `meson_options.txt`, `flake.nix`
- Protocol definitions: `protocols/**`
- Workflow file itself: `.github/workflows/build.yml`

**What it does**:

1. Installs system dependencies (libinput, libdrm, etc.)
2. Builds wayland 1.23.1 from source
3. Builds wlroots 0.19.0 from source
4. Builds scenefx 0.4.1 from source
5. Uses `meson subprojects download` to fetch any required subproject dependencies
6. Configures the project with meson
7. Builds the project with ninja
8. Verifies the executables were created

**Build Strategy**:

- All dependencies (wayland, wlroots, scenefx) are built from their source repositories
- Uses `meson subprojects download` before each meson setup to fetch required subprojects
- Allows meson wrap mode for automatic subproject handling (no --wrap-mode=nodownload)

**Dependencies**:

- Ubuntu latest runner
- Meson build system
- Ninja build tool
- Wayland 1.23.1 (built from source)
- wlroots 0.19.0 (built from source)
- scenefx 0.4.1 (built from source)

### docs.yml

**Purpose**: Validates markdown documentation for style and formatting consistency.

**Triggers**:

- Push to `main` or `master` branch (only when markdown files change)
- Pull requests to `main` or `master` branch (only when markdown files change)
- Manual dispatch (workflow_dispatch)

**Path filters** (only runs when these change):

- Markdown files: `**.md`
- Workflow file itself: `.github/workflows/docs.yml`

**What it does**:

- Lints markdown files in the repository using markdownlint-cli2
- Checks for common markdown formatting issues
- Ensures documentation follows consistent style guidelines
- Excludes dependency directories (wayland, wlroots, scenefx) to only lint repository files

### lock.yml

**Purpose**: Automatically locks inactive issues and PRs to keep the repository clean.

**Triggers**:

- Scheduled daily at 12:30 UTC
- Manual dispatch

**What it does**:

- Locks issues, PRs, and discussions that have been closed for 30 days
- Adds a comment explaining why the thread was locked

### stale.yml

**Purpose**: Automatically closes issues that have been manually marked as stale.

**Triggers**:

- Scheduled daily at 12:30 UTC

**What it does**:

- Closes issues marked with the "stale" label after 7 days of inactivity
- Adds "automatic-closing" label when closing
- Does not automatically mark issues as stale (only processes manually marked ones)

## Development Notes

The build workflow ensures that:

- Only runs when actual code or build configuration changes
- All dependencies are properly installed
- The project compiles without errors
- Both main executables (`mango` and `mmsg`) are built successfully

The docs workflow ensures that:

- Only runs when markdown documentation changes
- Documentation follows consistent formatting
- Markdown files are well-formed and free of common issues

If the build workflow fails, check:

1. Dependencies are up to date in the workflow file
2. wlroots and scenefx versions match requirements in meson.build
3. Build configuration in meson.build hasn't changed
