# Isometric Tiled Background Example

<image src="example.gif" height="380" />

Efficiently rendering an isometric tiled background on the original Gameboy using the actual background layer is surprisingly difficult. Here's a little example of how to do it, along with some really awful map-generating procgen code. Map data is stored as a 1bpt bitfield so that large maps can be stored in RAM.

## How it works

<image src="scroll_example.gif" height="380" />

Isometric tiles are addressed with north being up-and-left, south being down-and-right, east being right-and-up, and west being left-and-down. Isometric tiles are rendered in 4x2 tile chunks using 8x8px background tiles. The Gameboy's video hardware conveniently allows for scrolling the background layer pixel-by-pixel in the X or Y direction, but unfortunately the background is only 32x32 tiles large.

This is hardly enough space for a large map, and we can store a _very large map_ if we use bitfields to determine if a tile is traversible or not. A comparable map of 32x32 isometric tiles would be 128 8x8 tiles wide and 64 8x8 tiles tall. The obvious solution is to re-arrange all of the background tiles every frame, but updating the VRAM just after the vertical blanking period is critical and we only have 4mHz.

However, we have a workaround! Whenever we move, only a limited subset of the tiles on-screen actually change- the rest simply shift left/right/up/down. Keeping this in mind, we can update the background tiles for the _new tiles that would be displayed only_, and shift the rest of the tiles left/right/up/down according to player camera movement. This allows us to use the background layer as a kind of rolling buffer and takes background layer updates from an N^2 operation to 2N.