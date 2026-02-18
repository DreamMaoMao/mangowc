# GitHub Actions Workflows

This directory contains the GitHub Actions workflows for the MangoWC project.

## Workflows

### build.yml
**Purpose**: Builds the project to ensure code changes compile successfully.

**Triggers**:
- Push to `main` or `master` branch
- Pull requests to `main` or `master` branch
- Manual dispatch (workflow_dispatch)

**What it does**:
1. Installs system dependencies (wayland, libinput, etc.)
2. Builds wlroots 0.19 from source
3. Builds scenefx 0.4 from source
4. Configures the project with meson
5. Builds the project with ninja
6. Verifies the executables were created

**Dependencies**:
- Ubuntu latest runner
- Meson build system
- Ninja build tool
- Wayland ecosystem libraries
- wlroots 0.19
- scenefx 0.4.1

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
- All dependencies are properly installed
- The project compiles without errors
- Both main executables (`mango` and `mmsg`) are built successfully

If the build workflow fails, check:
1. Dependencies are up to date in the workflow file
2. wlroots and scenefx versions match requirements in meson.build
3. Build configuration in meson.build hasn't changed
