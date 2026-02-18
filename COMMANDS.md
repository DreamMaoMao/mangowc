# MangoWC Command Reference

This document lists all available commands that can be used in keybindings, mousebindings, or via IPC (mmsg).

## How to Use Commands

### In Configuration File (config.conf)

```conf
# Syntax: bind=MODIFIERS,KEY,COMMAND,ARGUMENTS
bind=Alt,Return,spawn,foot
bind=Alt,q,killclient,
bind=Ctrl,1,view,1,0
```

### Via IPC (mmsg)

```bash
# Syntax: mmsg -d COMMAND ARGUMENTS
mmsg -d spawn firefox
mmsg -d view 3
mmsg -d reload_config
```

---

## Window Management Commands

### killclient
Close the focused window.

**Syntax:** `killclient,`

**Example:**
```conf
bind=Alt,q,killclient,
```

### focusstack
Focus next/previous window in the stack.

**Arguments:** `next` or `prev`

**Example:**
```conf
bind=Super,Tab,focusstack,next
bind=Super+Shift,Tab,focusstack,prev
```

### focusdir
Focus window in a specific direction.

**Arguments:** `left`, `right`, `up`, or `down`

**Example:**
```conf
bind=Alt,Left,focusdir,left
bind=Alt,Right,focusdir,right
bind=Alt,Up,focusdir,up
bind=Alt,Down,focusdir,down
```

### focuslast
Focus the previously focused window.

**Syntax:** `focuslast,`

**Example:**
```conf
bind=Super,grave,focuslast,
```

### exchange_client
Swap the focused window with another window in a direction.

**Arguments:** `left`, `right`, `up`, or `down`

**Example:**
```conf
bind=Super+Shift,Left,exchange_client,left
bind=Super+Shift,Right,exchange_client,right
```

### exchange_stack_client
Swap window position in the stack.

**Arguments:** Stack position parameters

**Example:**
```conf
bind=Super,s,exchange_stack_client,1
```

### zoom
Move focused window to master position.

**Syntax:** `zoom,`

**Example:**
```conf
bind=Super,Return,zoom,
```

---

## Window State Commands

### togglefloating
Toggle between floating and tiled mode.

**Syntax:** `togglefloating,`

**Example:**
```conf
bind=Alt,backslash,togglefloating,
```

### togglefullscreen
Toggle fullscreen mode.

**Syntax:** `togglefullscreen,`

**Example:**
```conf
bind=Alt,f,togglefullscreen,
```

### togglefakefullscreen
Toggle fake fullscreen (fullscreen within layout).

**Syntax:** `togglefakefullscreen,`

**Example:**
```conf
bind=Alt+Shift,f,togglefakefullscreen,
```

### togglemaximizescreen
Toggle maximized state.

**Syntax:** `togglemaximizescreen,`

**Example:**
```conf
bind=Alt,a,togglemaximizescreen,
```

### toggleglobal
Toggle global state (visible on all tags).

**Syntax:** `toggleglobal,`

**Example:**
```conf
bind=Super,g,toggleglobal,
```

### toggleoverlay
Toggle overlay state (always on top).

**Syntax:** `toggleoverlay,`

**Example:**
```conf
bind=Super,o,toggleoverlay,
```

### minimized
Minimize the focused window.

**Syntax:** `minimized,`

**Example:**
```conf
bind=Super,i,minimized,
```

### restore_minimized
Restore the last minimized window.

**Syntax:** `restore_minimized`

**Example:**
```conf
bind=Super+Shift,I,restore_minimized
```

---

## Tag (Workspace) Commands

### view
Switch to a specific tag.

**Arguments:** 
- `tag_number` (1-9)
- `follow` (0 or 1, whether to follow window movements)

**Example:**
```conf
bind=Ctrl,1,view,1,0
bind=Ctrl,2,view,2,0
```

**IPC Example:**
```bash
mmsg -d view 3
```

### tag
Move focused window to a tag and switch to it.

**Arguments:**
- `tag_number` (1-9)
- `follow` (0 or 1)

**Example:**
```conf
bind=Alt,1,tag,1,0
bind=Alt,2,tag,2,0
```

**IPC Example:**
```bash
mmsg -d tag 5
```

### tagsilent
Move focused window to a tag without switching to it.

**Arguments:** `tag_number` (1-9)

**Example:**
```conf
bind=Alt+Shift,1,tagsilent,1
```

### toggletag
Toggle tag visibility (view multiple tags).

**Arguments:** `tag_number` (1-9)

**Example:**
```conf
bind=Super,1,toggletag,1
```

### toggleview
Toggle viewing of a tag (add/remove from view).

**Arguments:** `tag_number` (1-9)

**Example:**
```conf
bind=Super+Ctrl,1,toggleview,1
```

### comboview
View multiple tags simultaneously.

**Arguments:** Comma-separated tag numbers

**Example:**
```conf
bind=Super,c,comboview,1,2,3
```

### viewtoleft
Switch to the previous tag.

**Arguments:** `wrap` (0 or 1, whether to wrap around)

**Example:**
```conf
bind=Super,Left,viewtoleft,0
```

### viewtoright
Switch to the next tag.

**Arguments:** `wrap` (0 or 1, whether to wrap around)

**Example:**
```conf
bind=Super,Right,viewtoright,0
```

### viewtoleft_have_client
Switch to previous tag that has windows.

**Arguments:** `wrap` (0 or 1)

**Example:**
```conf
bind=Ctrl,Left,viewtoleft_have_client,0
```

### viewtoright_have_client
Switch to next tag that has windows.

**Arguments:** `wrap` (0 or 1)

**Example:**
```conf
bind=Ctrl,Right,viewtoright_have_client,0
```

### tagtoleft
Move focused window to previous tag.

**Arguments:** `wrap` (0 or 1)

**Example:**
```conf
bind=Ctrl+Super,Left,tagtoleft,0
```

### tagtoright
Move focused window to next tag.

**Arguments:** `wrap` (0 or 1)

**Example:**
```conf
bind=Ctrl+Super,Right,tagtoright,0
```

### bind_to_view
Bind window to always appear on a specific tag.

**Arguments:** `tag_number`

**Example:**
```conf
bind=Super,b,bind_to_view,1
```

---

## Layout Commands

### setlayout
Set a specific layout for the current tag.

**Arguments:** Layout name

**Available layouts:**
- `tile` - Master-stack tiling
- `scroller` - Horizontal scrolling columns
- `monocle` - One window at a time
- `grid` - Grid arrangement
- `deck` - Stacked windows
- `center_tile` - Centered master
- `vertical_tile` - Vertical master-stack
- `vertical_scroller` - Vertical scrolling
- `vertical_grid` - Vertical grid

**Example:**
```conf
bind=Super,t,setlayout,tile
bind=Super,s,setlayout,scroller
bind=Super,m,setlayout,monocle
```

**IPC Example:**
```bash
mmsg -d setlayout monocle
```

### switch_layout
Cycle through available layouts.

**Syntax:** `switch_layout`

**Example:**
```conf
bind=Super,n,switch_layout
```

### setmfact
Set master area size ratio.

**Arguments:** Ratio change (e.g., `+0.05`, `-0.05`)

**Example:**
```conf
bind=Super,h,setmfact,-0.05
bind=Super,l,setmfact,+0.05
```

### incnmaster
Change number of windows in master area.

**Arguments:** Change amount (e.g., `+1`, `-1`)

**Example:**
```conf
bind=Super,equal,incnmaster,+1
bind=Super,minus,incnmaster,-1
```

---

## Scroller Layout Commands

### set_proportion
Set window width proportion in scroller layout.

**Arguments:** Width ratio (0.0-1.0)

**Example:**
```conf
bind=Alt,e,set_proportion,1.0      # Full width
bind=Alt,w,set_proportion,0.5      # Half width
```

### switch_proportion_preset
Cycle through predefined width presets.

**Syntax:** `switch_proportion_preset,`

**Example:**
```conf
bind=Alt,x,switch_proportion_preset,
```

**Note:** Presets are defined in config as:
```conf
scroller_proportion_preset=0.5,0.8,1.0
```

### scroller_stack
Special scroller stacking behavior.

**Arguments:** Stacking parameters

**Example:**
```conf
bind=Super,s,scroller_stack,1
```

---

## Gap Commands

### incgaps
Increase or decrease all gaps.

**Arguments:** Pixel change (positive or negative)

**Example:**
```conf
bind=Alt+Shift,X,incgaps,1      # Increase by 1px
bind=Alt+Shift,Z,incgaps,-1     # Decrease by 1px
```

### togglegaps
Toggle gaps on/off.

**Syntax:** `togglegaps`

**Example:**
```conf
bind=Alt+Shift,R,togglegaps
```

### defaultgaps
Reset gaps to default values.

**Syntax:** `defaultgaps`

**Example:**
```conf
bind=Alt+Shift,D,defaultgaps
```

### incigaps
Increase/decrease inner gaps.

**Arguments:** Pixel change

**Example:**
```conf
bind=Super,equal,incigaps,1
bind=Super,minus,incigaps,-1
```

### incihgaps
Increase/decrease inner horizontal gaps.

**Arguments:** Pixel change

**Example:**
```conf
bind=Super+Shift,equal,incihgaps,1
```

### incivgaps
Increase/decrease inner vertical gaps.

**Arguments:** Pixel change

**Example:**
```conf
bind=Super+Ctrl,equal,incivgaps,1
```

### incogaps
Increase/decrease outer gaps.

**Arguments:** Pixel change

**Example:**
```conf
bind=Super+Alt,equal,incogaps,1
```

### incohgaps
Increase/decrease outer horizontal gaps.

**Arguments:** Pixel change

**Example:**
```conf
bind=Super+Alt+Shift,equal,incohgaps,1
```

### incovgaps
Increase/decrease outer vertical gaps.

**Arguments:** Pixel change

**Example:**
```conf
bind=Super+Alt+Ctrl,equal,incovgaps,1
```

---

## Floating Window Commands

### movewin
Move floating window by pixels.

**Arguments:** `x_offset,y_offset`

**Example:**
```conf
bind=Ctrl+Shift,Left,movewin,-50,+0
bind=Ctrl+Shift,Right,movewin,+50,+0
bind=Ctrl+Shift,Up,movewin,+0,-50
bind=Ctrl+Shift,Down,movewin,+0,+50
```

### resizewin
Resize floating window by pixels.

**Arguments:** `width_change,height_change`

**Example:**
```conf
bind=Ctrl+Alt,Left,resizewin,-50,+0
bind=Ctrl+Alt,Right,resizewin,+50,+0
bind=Ctrl+Alt,Up,resizewin,+0,-50
bind=Ctrl+Alt,Down,resizewin,+0,+50
```

### centerwin
Center the focused floating window.

**Syntax:** `centerwin,`

**Example:**
```conf
bind=Super,c,centerwin,
```

### smartmovewin
Smart move window (combines with layout logic).

**Arguments:** Direction and parameters

**Example:**
```conf
bind=Super,w,smartmovewin,up
```

### smartresizewin
Smart resize window (aware of layout).

**Arguments:** Direction and parameters

**Example:**
```conf
bind=Super,r,smartresizewin,right
```

### moveresize
Mouse-based move/resize.

**Arguments:** 
- `curmove` - Move with mouse
- `curresize` - Resize with mouse

**Example:**
```conf
mousebind=SUPER,btn_left,moveresize,curmove
mousebind=SUPER,btn_right,moveresize,curresize
```

---

## Special Feature Commands

### toggleoverview
Toggle overview mode (show all windows).

**Syntax:** `toggleoverview,`

**Example:**
```conf
bind=Alt,Tab,toggleoverview,
```

### toggle_scratchpad
Toggle scratchpad visibility.

**Syntax:** `toggle_scratchpad`

**Example:**
```conf
bind=Alt,z,toggle_scratchpad
```

### toggle_named_scratchpad
Toggle a named scratchpad.

**Arguments:** Scratchpad name

**Example:**
```conf
bind=Super,p,toggle_named_scratchpad,music
bind=Super,n,toggle_named_scratchpad,notes
```

**Setup named scratchpads:**
1. Open an application
2. Assign it to named scratchpad with IPC:
   ```bash
   mmsg -d toggle_named_scratchpad music
   ```

### toggle_render_border
Toggle border rendering.

**Syntax:** `toggle_render_border,`

**Example:**
```conf
bind=Super,b,toggle_render_border,
```

---

## Monitor Commands

### focusmon
Focus adjacent monitor.

**Arguments:** `left` or `right`

**Example:**
```conf
bind=Alt+Shift,Left,focusmon,left
bind=Alt+Shift,Right,focusmon,right
```

### tagmon
Move window to adjacent monitor.

**Arguments:** `left` or `right`

**Example:**
```conf
bind=Super+Alt,Left,tagmon,left
bind=Super+Alt,Right,tagmon,right
```

### viewcrossmon
View tag on different monitor.

**Arguments:** Monitor and tag parameters

**Example:**
```conf
bind=Super+Ctrl,m,viewcrossmon,1,2
```

### tagcrossmon
Move window to tag on different monitor.

**Arguments:** Monitor and tag parameters

**Example:**
```conf
bind=Super+Shift,m,tagcrossmon,1,2
```

### disable_monitor
Disable a monitor output.

**Arguments:** Output name or identifier

**Example:**
```bash
mmsg -d disable_monitor HDMI-A-1
```

### enable_monitor
Enable a monitor output.

**Arguments:** Output name or identifier

**Example:**
```bash
mmsg -d enable_monitor HDMI-A-1
```

### toggle_monitor
Toggle monitor on/off.

**Arguments:** Output name or identifier

**Example:**
```conf
bind=Super,F7,toggle_monitor,HDMI-A-1
```

---

## Virtual Output Commands

### create_virtual_output
Create a virtual monitor output.

**Arguments:** Configuration parameters

**Example:**
```bash
mmsg -d create_virtual_output 1920 1080
```

### destroy_all_virtual_output
Remove all virtual outputs.

**Syntax:** `destroy_all_virtual_output`

**Example:**
```bash
mmsg -d destroy_all_virtual_output
```

---

## Application Commands

### spawn
Launch an application.

**Arguments:** Command to execute

**Example:**
```conf
bind=Alt,Return,spawn,foot
bind=Alt,space,spawn,rofi -show drun
bind=Super,f,spawn,firefox
bind=Super,c,spawn,chromium
```

**IPC Example:**
```bash
mmsg -d spawn firefox
mmsg -d spawn "foot -e htop"
```

### spawn_shell
Launch command through shell.

**Arguments:** Shell command

**Example:**
```conf
bind=Super,s,spawn_shell,~/.config/mango/scripts/screenshot.sh
```

### spawn_on_empty
Spawn application on empty tag.

**Arguments:** Tag number and command

**Example:**
```conf
bind=Super,w,spawn_on_empty,2,firefox
```

---

## Input Commands

### setkeymode
Set keyboard mode (for modal keybindings).

**Arguments:** Mode name

**Example:**
```conf
bind=Super,k,setkeymode,resize
```

### switch_keyboard_layout
Switch between keyboard layouts.

**Syntax:** `switch_keyboard_layout`

**Example:**
```conf
bind=Super,Space,switch_keyboard_layout
```

**Note:** Define layouts in config:
```conf
xkb_rules_layout=us,ru
```

### toggle_trackpad_enable
Enable/disable trackpad.

**Syntax:** `toggle_trackpad_enable`

**Example:**
```conf
bind=Super,F9,toggle_trackpad_enable
```

---

## System Commands

### quit
Exit MangoWC.

**Syntax:** `quit`

**Example:**
```conf
bind=Super,m,quit
```

**IPC Example:**
```bash
mmsg -d quit
```

### reload_config
Reload configuration file.

**Syntax:** `reload_config`

**Example:**
```conf
bind=Super,r,reload_config
```

**IPC Example:**
```bash
mmsg -d reload_config
```

### chvt
Change virtual terminal.

**Arguments:** VT number

**Example:**
```conf
bind=Ctrl+Alt,F1,chvt,1
bind=Ctrl+Alt,F2,chvt,2
```

### setoption
Set a configuration option at runtime.

**Arguments:** Option name and value

**Example:**
```bash
mmsg -d setoption animations 0
mmsg -d setoption blur 1
mmsg -d setoption borderpx 2
```

---

## Complete Example Configuration

Here's a complete example showing various commands in use:

```conf
# Applications
bind=Alt,Return,spawn,foot
bind=Alt,space,spawn,rofi -show drun
bind=Super,f,spawn,firefox
bind=Super,e,spawn,thunar

# Window management
bind=Alt,q,killclient,
bind=Alt,Left,focusdir,left
bind=Alt,Right,focusdir,right
bind=Super+Shift,Left,exchange_client,left
bind=Super+Shift,Right,exchange_client,right

# Window states
bind=Alt,backslash,togglefloating,
bind=Alt,f,togglefullscreen,
bind=Alt,a,togglemaximizescreen,
bind=Super,g,toggleglobal,
bind=Super,i,minimized,

# Tags
bind=Ctrl,1,view,1,0
bind=Ctrl,2,view,2,0
bind=Alt,1,tag,1,0
bind=Alt,2,tag,2,0
bind=Super,Left,viewtoleft,0
bind=Super,Right,viewtoright,0

# Layouts
bind=Super,n,switch_layout
bind=Super,t,setlayout,tile
bind=Super,s,setlayout,scroller

# Special features
bind=Alt,Tab,toggleoverview,
bind=Alt,z,toggle_scratchpad

# Gaps
bind=Alt+Shift,X,incgaps,1
bind=Alt+Shift,Z,incgaps,-1
bind=Alt+Shift,R,togglegaps

# System
bind=Super,r,reload_config
bind=Super,m,quit
```

---

## Using Commands via IPC

All commands can be executed via the `mmsg` IPC tool:

### Basic Syntax
```bash
mmsg -d COMMAND ARGUMENTS
```

### Examples

**Window management:**
```bash
mmsg -d killclient
mmsg -d togglefloating
mmsg -d togglefullscreen
```

**Tag switching:**
```bash
mmsg -d view 3
mmsg -d tag 5
mmsg -d viewtoright
```

**Layout control:**
```bash
mmsg -d setlayout monocle
mmsg -d switch_layout
```

**Application launching:**
```bash
mmsg -d spawn firefox
mmsg -d spawn "foot -e htop"
```

**System control:**
```bash
mmsg -d reload_config
mmsg -d setoption animations 1
```

### Scripting with mmsg

**Example 1: Toggle between two layouts**
```bash
#!/bin/bash
LAYOUT=$(mmsg -L | head -1 | grep -o "tile\|monocle")
if [ "$LAYOUT" = "tile" ]; then
    mmsg -d setlayout monocle
else
    mmsg -d setlayout tile
fi
```

**Example 2: Move all windows to tag 1**
```bash
#!/bin/bash
for i in {2..9}; do
    mmsg -d view $i
    while mmsg -c | grep -q "clients:"; do
        mmsg -d tag 1
    done
done
mmsg -d view 1
```

**Example 3: Cycle through tags with windows**
```bash
#!/bin/bash
mmsg -d viewtoright_have_client
```

---

## See Also

- [config.conf](config.conf) - Configuration file with all keybindings
- [README.md](README.md) - General documentation and quick start guide
- [MangoWC Wiki](https://github.com/DreamMaoMao/mango/wiki/) - Comprehensive online documentation
