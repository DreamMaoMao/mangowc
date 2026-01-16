# Mango Wayland Compositor Nixpup Fork
![MangoWC Logo](https://codeberg.org/nixpup/MangoWC/raw/branch/main/mangowc_logo.png)

# Demo
<video src="https://codeberg.org/nixpup/MangoWC/raw/branch/main/demo.mp4" controls>
  Your browser does not support the video tag.
</video>

# New Features
This build of MangoWC supports *vertical stacking* within the *scroller* layout, similar to how the Niri Wayland Compositor and Window Manager works. This means by binding the following actions in your `~/.config/mango/config.conf`:
```
bind=Alt,comma,stack_with_left
bind=Alt,period,unstack
bind=Alt,t,revert_size
```
You can automatically split/resize the window to the left of the currently selected window, and then move/tile the currently selected window below the window to its left via the `stack_with_left` option. Removing a window from a stack is triggered by the `unstack` action, and with the `revert_size` action you can revert a window to its original size, meaning it gets resized back to using the full vertical screen size, and gets re-integrated into the scroller layout.

## [Original MangoWC Repository](https://github.com/DreamMaoMao/mangowc)