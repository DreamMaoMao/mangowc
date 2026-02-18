# MangoWC Usage Guide

A practical guide to using MangoWC in your daily workflow.

## Table of Contents

- [Getting Started](#getting-started)
- [Understanding Tags](#understanding-tags)
- [Working with Layouts](#working-with-layouts)
- [Window Management Patterns](#window-management-patterns)
- [Using the Scratchpad](#using-the-scratchpad)
- [Overview Mode](#overview-mode)
- [Multi-Monitor Workflows](#multi-monitor-workflows)
- [Customization Tips](#customization-tips)
- [Common Workflows](#common-workflows)

---

## Getting Started

### Your First Session

1. **Launch MangoWC** from your display manager or run `mango` from TTY
2. **Open a terminal:** Press `Alt + Return`
3. **Open an application:** Press `Alt + Space` for the launcher
4. **Navigate windows:** Use `Alt + Arrow Keys`
5. **Close a window:** Press `Alt + Q`

### Essential First-Day Shortcuts

These are the absolute minimum you need to be productive:

```
Alt + Return         → Terminal
Alt + Space          → App launcher
Alt + Q              → Close window
Alt + Arrow Keys     → Focus different window
Ctrl + 1-9           → Switch tags (workspaces)
Alt + \              → Toggle floating
Super + R            → Reload config
Super + M            → Exit MangoWC
```

---

## Understanding Tags

### What Are Tags?

Tags are like workspaces, but more flexible. Think of them as labels you can apply to windows.

**Key differences from workspaces:**

- A window can have multiple tags
- You can view multiple tags at once
- Each tag can have its own layout

### Basic Tag Usage

**Switch to a tag:**

```
Ctrl + 1    → View tag 1
Ctrl + 2    → View tag 2
... and so on
```

**Move window to a tag:**

```
Alt + 1     → Move current window to tag 1 (and switch to it)
Alt + 2     → Move current window to tag 2 (and switch to it)
```

**Navigate through tags:**

```
Super + Left/Right         → Previous/next tag
Ctrl + Left/Right          → Previous/next tag that has windows
```

### Advanced Tag Techniques

**View multiple tags simultaneously:**

1. Switch to tag 1: `Ctrl + 1`
2. Toggle tag 2 visibility: `Super + 2` (if configured)
3. Now you see windows from both tags

**Silent tag movement** (move window without following):

```conf
# In config.conf
bind=Alt+Shift,1,tagsilent,1
```

### Organizing Your Workflow with Tags

**Example organization:**

```
Tag 1: Browsers          (monocle layout)
Tag 2: Code editors      (tile or center_tile layout)
Tag 3: Terminals         (scroller layout)
Tag 4: Communication     (grid layout)
Tag 5: Media             (monocle layout)
Tag 6-9: Project-specific
```

**Setting layouts per tag in config.conf:**

```conf
tagrule=id:1,layout_name:monocle
tagrule=id:2,layout_name:center_tile
tagrule=id:3,layout_name:scroller
tagrule=id:4,layout_name:grid
tagrule=id:5,layout_name:monocle
```

---

## Working with Layouts

### Available Layouts

MangoWC includes 9 built-in layouts:

#### 1. Tile (Master-Stack)

```
┌────────┬────┐
│        │ 1  │
│ Master ├────┤
│        │ 2  │
└────────┴────┘
```

**Best for:** General multitasking, coding with docs
**Switch to:** Cycle with `Super + N` (or add `bind=Super,t,setlayout,tile` to config)

#### 2. Scroller (Horizontal)

```
┌───┬────┬───┬───┐
│ 1 │  2 │ 3 │ 4 │...
└───┴────┴───┴───┘
```

**Best for:** Terminals, wide content, many windows
**Switch to:** Cycle with `Super + N` (or add `bind=Super,s,setlayout,scroller` to config)

**Scroller-specific controls:**

- `Alt + E` - Set current window to full width
- `Alt + X` - Cycle through width presets (0.5, 0.8, 1.0)

#### 3. Monocle (Fullscreen Stack)

```
┌──────────────┐
│              │
│   Window 1   │
│   (fullscrn) │
└──────────────┘
```

**Best for:** Focus work, browsing, media
**Switch to:** Cycle with `Super + N`
**Navigate:** Use `Super + Tab` to cycle through windows

#### 4. Grid

```
┌─────┬─────┐
│  1  │  2  │
├─────┼─────┤
│  3  │  4  │
└─────┴─────┘
```

**Best for:** Viewing many windows simultaneously
**Switch to:** Cycle with `Super + N`

#### 5. Deck (Card Stack)

```
┌──────────────┐
│  Window 1    │  ← Visible
│ (2, 3 hidden)│
└──────────────┘
```

**Best for:** Cycling through options
**Switch to:** Cycle with `Super + N`

#### 6. Center Tile

```
┌──┬──────┬──┐
│1 │      │2 │
│  │Master│  │
│3 │      │4 │
└──┴──────┴──┘
```

**Best for:** Symmetrical workflow, coding
**Switch to:** Cycle with `Super + N`

#### 7. Vertical Tile

```
┌──────────────┐
│    Master    │
├──┬───┬───┬───┤
│1 │ 2 │ 3 │ 4 │
└──┴───┴───┴───┘
```

**Best for:** Ultra-wide monitors
**Switch to:** Cycle with `Super + N`

### Layout Controls

**Cycle through all layouts:**

```
Super + N    → Next layout
```

**Adjust layout parameters:**

For **tile/center_tile** layouts:

```
Super + H       → Decrease master size  (add to config)
Super + L       → Increase master size  (add to config)
Super + Equal   → More windows in master (add to config)
Super + Minus   → Fewer windows in master (add to config)
```

Add to config.conf:

```conf
bind=Super,h,setmfact,-0.05
bind=Super,l,setmfact,+0.05
bind=Super,equal,incnmaster,+1
bind=Super,minus,incnmaster,-1
```

For **scroller** layout:

```
Alt + E      → Set window to full width
Alt + X      → Cycle width presets
```

---

## Window Management Patterns

### Focus Management

**Directional focus:**

```
Alt + ←→↑↓   → Focus window in direction
```

**Stack focus:**

```
Super + Tab          → Next window in stack
Super + Shift + Tab  → Previous window in stack
```

**Focus last window:**

```
Super + `    → Focus previously focused window (add to config)
```

### Moving Windows

**Swap with neighbors:**

```
Super + Shift + ←→↑↓   → Swap positions
```

**Move to master:**

```
Super + Return   → Move focused window to master position (add to config)
```

Add to config:

```conf
bind=Super,Return,zoom,
```

**Move between tags:**

```
Alt + 1-9              → Move to tag (and follow)
Ctrl + Super + ←→      → Move to prev/next tag
```

### Floating Windows

**Toggle floating:**

```
Alt + \    → Toggle floating/tiling
```

**Move floating windows:**

```
Ctrl + Shift + ←→↑↓   → Move by pixels
Super + Left-drag      → Move with mouse
```

**Resize floating windows:**

```
Ctrl + Alt + ←→↑↓     → Resize by pixels
Super + Right-drag     → Resize with mouse
```

**Center floating window:**

```
Super + C    → Center window (add to config)
```

Add to config:

```conf
bind=Super,c,centerwin,
```

### Window States

**Common states:**

```
Alt + \              → Floating
Alt + F              → Fullscreen
Alt + Shift + F      → Fake fullscreen
Alt + A              → Maximized
Super + G            → Global (visible all tags)
Super + O            → Overlay (always on top)
```

**Minimize/Restore:**

```
Super + I            → Minimize window
Super + Shift + I    → Restore minimized
```

---

## Using the Scratchpad

### What is the Scratchpad?

The scratchpad is a hidden workspace for temporary windows. It's perfect for:

- Calculator
- Music player
- Notes/Todo list
- System monitor
- Password manager

### Basic Scratchpad Usage

1. **Open an application** (e.g., terminal with calculator)

   ```bash
   Alt + Return
   # Then run: qalc (or any calculator)
   ```

2. **Move to scratchpad**

   ```
   Alt + Z
   ```

3. **Window disappears** - it's now in the scratchpad

4. **Toggle scratchpad** to show/hide

   ```
   Alt + Z    → Shows scratchpad over current work
   Alt + Z    → Hides it again
   ```

### Named Scratchpads

You can have multiple named scratchpads:

**Setup in config.conf:**

```conf
bind=Super,p,toggle_named_scratchpad,music
bind=Super,n,toggle_named_scratchpad,notes
bind=Super,c,toggle_named_scratchpad,calc
```

**Usage:**

1. Open application (e.g., spotify)
2. Press `Super + P` - assigns it to "music" scratchpad
3. Press `Super + P` again - shows/hides music scratchpad
4. Other scratchpads are independent

### Scratchpad Tips

**Automatic scratchpad applications:**

Create a script to launch apps directly into scratchpad:

```bash
#!/bin/bash
# ~/.config/mango/scripts/scratchpad-calc.sh

# Open calculator in scratchpad
foot -e qalc &
sleep 0.5
mmsg -d toggle_scratchpad
```

Bind it:

```conf
bind=Super,equal,spawn_shell,~/.config/mango/scripts/scratchpad-calc.sh
```

**Customize scratchpad appearance:**

In config.conf:

```conf
scratchpad_width_ratio=0.8    # 80% of screen width
scratchpad_height_ratio=0.9   # 90% of screen height
scratchpadcolor=0x516c93ff    # Border color for scratchpad windows
```

---

## Overview Mode

### What is Overview?

Overview mode shows all windows at once, similar to GNOME's Activities or macOS Mission Control.

### Using Overview

**Toggle overview:**

```
Alt + Tab    → Toggle overview mode
```

**In overview mode:**

- Click any window to focus it
- Drag windows to rearrange
- Windows are organized visually
- All tags are shown

**Hotarea trigger:**
Move your mouse cursor to the top edge of the screen to automatically trigger overview.

**Configure hotarea:**

```conf
# In config.conf
hotarea_size=10        # Pixels from edge to trigger
enable_hotarea=1       # 1=enabled, 0=disabled
```

### Overview Settings

```conf
# In config.conf
ov_tab_mode=0          # Show windows as tabs (0=off, 1=on)
overviewgappi=5        # Gap between windows in overview
overviewgappo=30       # Gap from screen edges
```

---

## Multi-Monitor Workflows

### Basic Multi-Monitor Commands

**Focus different monitor:**

```
Alt + Shift + ←→    → Focus adjacent monitor
```

**Move window to other monitor:**

```
Super + Alt + ←→    → Move window to adjacent monitor
```

### Multi-Monitor Strategies

**Strategy 1: Dedicated monitors per task**

- Monitor 1: Development (Tag 1-3)
- Monitor 2: Communication (Tag 4-6)
- Monitor 3: Media/Reference (Tag 7-9)

**Strategy 2: Mirror similar tags**

- Both monitors show same tags
- Each monitor has different layouts
- Use global windows to span both

**Strategy 3: Independent workspaces**

- Each monitor is independent
- Use monitor focus shortcuts frequently
- Keep related work on same monitor

### Monitor Configuration

MangoWC auto-detects monitors. For custom setup:

**Disable a monitor:**

```bash
mmsg -d disable_monitor HDMI-A-1
```

**Enable a monitor:**

```bash
mmsg -d enable_monitor HDMI-A-1
```

**Find monitor names:**

```bash
mmsg -o
```

---

## Customization Tips

### Modify Keybindings

Edit `~/.config/mango/config.conf`:

```conf
# Change default terminal
bind=Alt,Return,spawn,alacritty        # Instead of foot

# Add custom application shortcuts
bind=Super,f,spawn,firefox
bind=Super,c,spawn,chromium
bind=Super,e,spawn,thunar

# Add focus last window
bind=Super,grave,focuslast,

# Add center window
bind=Super,c,centerwin,
```

After editing, press `Super + R` to reload.

### Customize Animations

```conf
# In config.conf

# Speed up animations
animation_duration_open=200     # Faster (default: 400)
animation_duration_close=400    # Faster (default: 800)

# Disable animations
animations=0

# Change animation type
animation_type_open=zoom        # Or "slide"
animation_type_close=zoom
```

### Customize Appearance

```conf
# In config.conf

# Colors (RGBA hex: 0xRRGGBBAA)
focuscolor=0x89b4faff          # Blue focused border
bordercolor=0x313244ff         # Dark unfocused border

# Borders and gaps
borderpx=3                     # Border width
gappih=10                      # Inner horizontal gap
gappiv=10                      # Inner vertical gap
gappoh=15                      # Outer horizontal gap
gappov=15                      # Outer vertical gap

# Rounded corners
border_radius=10               # Corner radius

# Window effects
blur=1                         # Enable blur
shadows=1                      # Enable shadows
focused_opacity=1.0            # Focused window opacity
unfocused_opacity=0.95         # Unfocused window opacity
```

### Custom Scripts

Create `~/.config/mango/scripts/` directory:

**Example: Screenshot script**

```bash
#!/bin/bash
# ~/.config/mango/scripts/screenshot.sh

grim -g "$(slurp)" - | satty --filename - --fullscreen
```

**Bind it:**

```conf
bind=Print,none,spawn_shell,~/.config/mango/scripts/screenshot.sh
```

**Example: Volume control**

```bash
#!/bin/bash
# ~/.config/mango/scripts/volume.sh

case $1 in
    up) pamixer -i 5 ;;
    down) pamixer -d 5 ;;
    mute) pamixer -t ;;
esac
```

**Bind it:**

```conf
bind=NONE,XF86AudioRaiseVolume,spawn_shell,~/.config/mango/scripts/volume.sh up
bind=NONE,XF86AudioLowerVolume,spawn_shell,~/.config/mango/scripts/volume.sh down
bind=NONE,XF86AudioMute,spawn_shell,~/.config/mango/scripts/volume.sh mute
```

---

## Common Workflows

### Workflow 1: Web Development

**Setup:**

```
Tag 1: Browser (monocle)        → Testing
Tag 2: Code editor (center_tile) → Development
Tag 3: Terminals (scroller)      → Commands
Scratchpad: Documentation, notes
```

**Workflow:**

1. Start on Tag 2, write code
2. `Ctrl + 1` → View in browser
3. `Ctrl + 3` → Run build/dev server
4. `Alt + Z` → Toggle docs when needed
5. Use `Super + Left/Right` to cycle between tags

### Workflow 2: Writing and Research

**Setup:**

```
Tag 1: Writing app (monocle)
Tag 2: Research browser (tile)
Tag 3: Reference PDFs (grid)
Scratchpad: Notes, citations
```

**Workflow:**

1. Research on Tags 2-3
2. `Ctrl + 1` → Switch to writing
3. `Super + G` on notes window → Make it global
4. Notes now visible on all tags

### Workflow 3: Communication and Work

**Setup:**

```
Tag 1: Email (monocle)
Tag 2: Slack/Discord (tile)
Tag 3: Work documents (tile)
Tag 4: Calendar (monocle)
Global: Music player
```

**Workflow:**

1. Open music player → `Super + G` (make global)
2. Start each app on its tag
3. Use `Ctrl + Left/Right` to jump between active tags
4. `Alt + Tab` for quick overview

### Workflow 4: System Administration

**Setup:**

```
Tag 1: Terminals (scroller)     → Multiple SSH sessions
Tag 2: Monitoring (grid)         → htop, iotop, etc.
Tag 3: Browser (monocle)         → Documentation
Scratchpad: Calculator, quick commands
```

**Workflow:**

1. Open terminals in scroller layout
2. `Alt + X` to adjust column widths
3. `Ctrl + 2` for monitoring overview
4. `Alt + Z` for quick calculations

### Workflow 5: Video/Audio Editing

**Setup:**

```
Tag 1: Editor (monocle)          → Full-screen editing
Tag 2: File browser (tile)       → Asset management
Tag 3: Preview (monocle)         → Testing output
```

**Workflow:**

1. Edit on Tag 1 in monocle (maximum space)
2. `Ctrl + 2` → Grab assets
3. `Ctrl + 3` → Preview output
4. Use `Alt + F` for true fullscreen when needed

---

## Quick Reference Card

### Essential Shortcuts

```
Alt + Return         Terminal
Alt + Space          Launcher
Alt + Q              Close window
Super + R            Reload config
Super + M            Exit

Alt + ←→↑↓           Focus direction
Super + Shift + ←→↑↓ Swap windows
Alt + \              Toggle floating
Alt + F              Fullscreen
Super + N            Change layout

Ctrl + 1-9           View tag
Alt + 1-9            Move to tag
Super + ←→           Prev/next tag

Alt + Tab            Overview
Alt + Z              Scratchpad
Super + G            Global window
```

### When Things Go Wrong

```
Super + R            Reload config (fixes most issues)
Super + M            Exit MangoWC
Ctrl + Alt + F2      Switch to TTY (if GUI breaks)
```

---

## Next Steps

Now that you understand the basics:

1. **Customize your config** - Edit `~/.config/mango/config.conf`
2. **Set up autostart** - Create `~/.config/mango/autostart.sh`
3. **Explore layouts** - Find what works for your workflow
4. **Learn IPC** - Automate with `mmsg` (see COMMANDS.md)
5. **Join the community** - [Discord](https://discord.gg/CPjbDxesh5)

**Additional Resources:**

- [COMMANDS.md](COMMANDS.md) - Complete command reference
- [config.conf](config.conf) - Commented configuration file
- [MangoWC Wiki](https://github.com/DreamMaoMao/mango/wiki/) - Comprehensive documentation
- [Website](https://mangowc.vercel.app/docs) - Online documentation

---

**Happy tiling! 🥭**
