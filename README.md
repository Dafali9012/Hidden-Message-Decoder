# Hidden-Message-Decoder
Decodes binary messages from images

- Click to add up to 4 vertices
- Hold key 1 through 4 and click to replace vertex
- Up/down arrow keys change the number of rows
- Left/right arrow keys change the number of columns
- Press T key to translate the message

Reads binary messages where characters are 6 bits long\
0 starts at ASCII "A"\
reads from top to bottom, left to right

The program reads a file namned "message.png" in the same folder\
When all vertices have been placed around the message, a grid of points will appear\
Points turn white, meaning the dark area they are placed in is recognized as a 1 bit\

Built with Raylib
