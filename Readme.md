# Isometric Tiled Background Example

![Example](example.gif | width=200)

Efficiently rendering an isometric tiled background on the original Gameboy using the actual background layer is surprisingly difficult. Here's a little example of how to do it, along with some really awful map-generating procgen code. Map data is stored as a 1bpt bitfield so that large maps can be stored in RAM.