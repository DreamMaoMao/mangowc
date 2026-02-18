# GitHub Actions Workflows

This directory contains the GitHub Actions workflows for the MangoWC project.

## Workflows

### build.yml

**Purpose**: Builds the project on Arch Linux to ensure code changes compile
successfully.

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

1. Runs in Arch Linux container (archlinux:latest)
2. Updates system and installs all dependencies via pacman
3. Installs wlroots from official Arch repositories
4. Installs scenefx from AUR (Arch User Repository)
5. Configures the project with meson
6. Builds the project with ninja
7. Verifies the executables were created

**Build Strategy**:

- Uses Arch Linux for up-to-date system packages
- All dependencies installed via pacman or AUR (no source builds)
- wlroots installed from official Arch repositories
- scenefx installed from AUR

**Dependencies**:

- Arch Linux container (archlinux:latest)
- Meson build system
- Ninja build tool
- All system packages from pacman (wayland, libinput, wlroots, mesa, etc.)
- scenefx from AUR

### build-nixos.yml

**Purpose**: Builds the project on NixOS using Nix flakes to ensure code
changes work correctly in the NixOS ecosystem.

**Triggers**:

- Push to `main` or `master` branch (only when code files change)
- Pull requests to `main` or `master` branch (only when code files change)
- Manual dispatch (workflow_dispatch)

**Path filters** (only runs when these change):

- Source files: `**.c`, `**.h`, `**.cpp`, `**.scm`
- Build files: `meson.build`, `meson_options.txt`, `flake.nix`
- Nix files: `nix/**`
- Protocol definitions: `protocols/**`
- Workflow file itself: `.github/workflows/build-nixos.yml`

**What it does**:

1. Runs on Ubuntu with Nix installed
2. Installs Nix with flakes and nix-command experimental features
3. Builds the project using `nix build` with the repository's flake.nix
4. Verifies that executables (mango and mmsg) are created and executable
5. Tests basic executable functionality

**Build Strategy**:

- Uses Nix flakes for reproducible builds
- All dependencies managed through Nix
- Leverages the repository's flake.nix configuration
- Dependencies from nixpkgs-unstable

**Dependencies**:

- Nix package manager with flakes support
- All dependencies defined in flake.nix and nix/default.nix
- scenefx from upstream flake
- wlroots 0.19 from nixpkgs

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
- Excludes dependency directories (wayland, wlroots, scenefx) to only lint
  repository files

### lock.yml

**Purpose**: Automatically locks inactive issues and PRs to keep the repository
  clean.

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

The build workflows ensure that:

- Only run when actual code or build configuration changes
- All dependencies are properly installed
- The project compiles without errors on both Arch Linux and NixOS
- Both main executables (`mango` and `mmsg`) are built successfully

The docs workflow ensures that:

- Only runs when markdown documentation changes
- Documentation follows consistent formatting
- Markdown files are well-formed and free of common issues

If a build workflow fails, check:

1. Dependencies are up to date in the workflow file
2. wlroots and scenefx versions match requirements in meson.build
3. Build configuration in meson.build hasn't changed
4. For NixOS builds: flake.nix and nix/default.nix are correctly configured
