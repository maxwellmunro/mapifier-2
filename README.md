# Mapifier 2

Mind map making software, mosty a copy of [Mapifier](https://github.com/maxwellmunro/mapifier), with bug fixes and extra features

## Dependencies

- SDL2
- SDL2_ttf
- SDL2_gfx
- boost-serialization

## Build

Ensure the `mainFont.ttf` is a valid font file in `$HOME/.mapifier/res/`

Run the makefile, `make` with g++ and dependencies installed

## Features

- Node creation, deletion, linking and unlinking
- Colours
- Select children
- Togglable text wrapping, centered text and node shape
- Colour copy / paste
- Accidental quit protection, (use Ctrl+Shift+Alt+Q to force quit, rather than default close keybind)
- Multiselect
- Panning / zooming

## Controls

### Mouse
- Left: select / move
- Shift+Left: toggle select / mulltiselect
- Alt+Left: move children
- Right: pan
- Scroll: zoom

### Keybinds
- Return: stop typing filename
- Escape: stop typing filename and stop setting parent(s)
- Ctrl+N: New node
- Ctrl+Backspace: clear nodes text
- Ctrl+R: set node(s) Random colours
- Ctrl+S: Set node(s) parents
- Ctrl+=: select children
- Ctrl+1: toggle text wrapping
- Ctrl+2: toggle node shape
- Ctrl+C: toggle Centered text
- Delete: DELETE node
- Ctrl+W: Write (save) current map
- Ctrl+O: Open current map
- Ctrl+A: copy colour
- Ctrl+Z: paste colour
- Ctrl+Shift+Alt+Q: force Quit
- Arrow Keys: Move node
- Shift/Ctrl + Arrow: Move node faster




