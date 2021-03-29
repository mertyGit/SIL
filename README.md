![SIL LOGO](https://github.com/mertyGit/SIL/blob/master/docs/sillogo.png)

TOC
* [About](#about)
* [Layers](#layers?)

## Release Information

Status: Still kinda Beta. Everything works but I'm still busy rearranging the code and functions to make it easier and simpler to work with.

TODO:
- [ ] Clean up code, remove code/internal functions not needed anymore
- [ ] Clean up names of some functions. Some became.very.long.it.almost.looks.like.java.class.notations. Don't want to depend on IDE autofill
- [ ] Writing/Extending Documentation. All programmers love writing documentation. Really. 
- [ ] Rotating layers 90 degrees without wasting much memory.
- [ ] Testing SDL version on more platforms then windows.
- [ ] Grouping of layers, move/hide a single group with one command instead of custom loop
- [ ] "Headless" display. Only output can be a PNG

POSSIBLE WISHLIST:
- [ ] Keyhandling for X-Windows is clumpsy. Annoying auto-repeating hacks and all. Have to fix it..
- [ ] Keyhandling doesn't make difference in pressing left of right Shift, Ctrl or Alt
- [ ] Creating an example simple game using SIL
- [ ] More platforms ! ESP32+Displays TTGO environments are next on my list



## About

**S**imple **I**nteractive **L**ayers, or "**SIL**" for short, is a small collection of C-files to create decent looking GUI's, without the dependency of memory exhausting bloated libraries or complex frameworks that do require a steep learning curve to overcome. It is especially targeted at non-hardware accelerated, low resolution touch-screen displays, driven by small SOC's or single board computers, like raspberry PI or ESP32, but can run with same look and feel on 'normal' windows and Linux desktops, to speed up development and testing without having to flash/upload it and test it on the platform target.
Under the hood, it is using "plain framebuffers" - a large memory buffer that holds all pixel color information in a straight non-compressed manner.  

### Layers ?

Ah yes, the only basic concept you will have to understand to use this GUI. I will not bother you with *widgets, sprites, textures, backplanes* and *windows*. With SIL -almost- *everything is done on layers*. This makes designing a GUI, without touching a programming environment, so much easier ; just grab Gimp, Paint.Net , Photoshop or any other graphic program and allmost all of them works with the concept of layers. Export each layer to a .PNG file, write code to initialize your program and add a layer per PNG file, and you have your basic setup for the program, looking as good as intended. Now you can hide, move, filter, draw on these layers from your program. Even handle events about mouse/touch or keyboard presses. Everytime you ask SIL to update the screen, it will transform all layers (from bottom till top) to a single image that will send out to the "display" (a real display or a window in Windows or Linux).

![Layer Explanation](https://github.com/mertyGit/SIL/blob/master/docs/layersexplanation.png)

A layer is able to contain pixels that are transparant, thus showing pixels of the underlying layer(s). It is also possible to set the transparency of the whole layer, this way you can "blend" pixels of multiple layers. Layers can be turned off, but also adjust to only show a smaller part of the layer, a *view* . Every layer has its own *[framebuffer](https://en.wikipedia.org/wiki/Framebuffer)* which can have a different byte packing / color space, saving memory or making it easier to adapt an image format with the structure. For example, even if a display can handle 24bit colors, you don't need to reserve memory for 24bit colors for every layer you create. If you are happy with less colors or don't need transparency, you can do with even 8 bit, saving you memory. The whole framebuffer handling and conversion is hidden as part of basic handling & drawing functions, but you can have direct access to it if you want to speed things up. 


### Why ?

The -original- reason I wrote this, is because I needed a touchscreen GUI for my self-made CNC machine, using a raspberry PI and a 7" touchscreen display but was thinking it replacing it later with a more low-level microprocessor board, like ESP32's. Most GUI libraries where either mouse or windows based, to complicated to implement or works only on a specific platform and programming language (-version). A lot of them where designed for normal PC environments and took too much of a toll of memory and CPU. So, options left where free, open-source GUI-kinda-drawing primitives written for small systems, usaully targeted at arduino (-environment). 

But, looking at these solutions, a specific display driver was needed and all drawing primitives - put pixel, draw line and so on -  were incorporated in this specific display drivers. Sometimes a lot sometimes only a few operations. Also, most of the time, this driver or module was written for a specific SOC or in micropython or circuitpython, which is great for beginners, but also comes with additional performance en memory degradation (and incompatibility).I'm pretty confident in my C-coding skills and is still the easiest way to port code to whatever-environment-I-want.

As final argument, most of the time, most GUI libraries just looks....meh... No anti-aliasing nor full color (even if the display supports this) no alpha blending.  I know, on an system like an Arduino UNO you can't do much with speed and memory so you can't afford the luxury....But why shouldn't it be on better equiped environments ? If you don't need the speed of accelerated graphics hardware, but at least you have decent memory, why not make it look good as it is on your average PC ? Even doing so, why not design, develop and test the stuff on your own PC and only as final step cross-compile to target environment and deploy it ?

So, I needed "something in the middle" and preferably re-usable for broad scala of displays and environments without much hassle and that became SIL.

### Speed ?

So, since it just written to supply a 2D touch GUI, in some cases presented by not-so-fast I2C displays, speed wasn't much a concern developing this. Ease of use and adaptability was higher on my priority list. I can't notice if I press a button and display isn't updated in the first 100ms or so. But out of curiousity, I wanted to compare it with hardware-accelerated - read: GPU's - environments, but didn't want to learn the indepths of Direct3D, OpenGL or Vulcan (yet), so I used SDL for it (and yes, *that* needs an additional library) If you are looking for more speed, check SDL version. But keep in mind that basic concepts of layers-over-layers being drawn via direct framebuffers and updating the whole screen everytime is quite flexible, simple and direct, but not the fastest way to work. So...It is not a game engine or usable for streaming video or something (though, I can get around 30FPS on my large windows desktop if I really push it and adjust some SDL options)

### But what about .... 

But...there are multiple and multiple libraries out there....one must be doing a much, much better effort then this one. I'm pretty sure it is, actually, I peeked / borrowed code from a few of those. But I also like to tinker with it, adding new functionalities I, and mostly only myself, need and just try out graphic manipulations and wanted to have a simple environment for this for all the platforms and devices I work with. Besides...as an old fart that started programming C more then 35 years ago, I like to write "bare bone C", and if I have time, maybe move large sections of it to assembly, but I then need to target Intel, ARM and RISC-V environments... but who knows...

### Advantages
* **Small**, and you can strip out more if you do not need some functionality (like filters, fonts, logging). I try to keep unnecesary memory allocations to a minimum. 
* **Portable** ! By sticking to standard ANSI-C and don't do multithreading wizardy, if you find a C compiler for your environment, it will compile.
* **Exact look-and-feel for every platform** (besides from display colordepth indifferences)
* **Easy to use**; no frameworks, weird libraries, strange configurator or build scripts, just use the *.c and *.h files together with your own files and development environment
* Is able to handle and translate **most common color-palet/packing methods** from *32bit RGBA* to *223 bit RGB* under the hood.
* **Single threaded**, no dealings with race conditions, locking and handling multithreading on platforms that doesn't support it. Of course, you can make you own program multithreaded or even put the display/updating/events stuff inside a seperate thread.

### Disadvantages
* It is not lightning fast; It still will be much faster in comparision with intepreted languages, but the choice of *framebuffers-per-layer-with-multiple-colorspaces* and updating *whole screen* everytime something changes, makes it not really suitable for high-speed games or video streaming/altering purposes.
* Texthandling; keyboard support en drawing text on layers is mostly written for shortkeys or entering configuration values, but not for writing long texts with scrollbars, cursors and so on. Although you can build on top of it, if you want to. 
* Static display/window size; Usually it is written to write directly to a display with given resolution. So no windowing rescaling / auto adjustments and scrollbars. 
* It is not written in  ....*Insert your own language here*.... . However, without doubt it isn't hard to integrate it with any language, since most languages do still use C(++)   libraries underneath. 

### Derived work, Inspiration and Praises
As most programmers, I'm sometimes too lazy to reinvent the wheel so why not use the broad available knowledge and experience out there :wink:. Since it is still open source, I used -part of- code or concepts of
* [LodePNG](https://lodev.org/lodepng/) from Lode Vandevenne - Reading & Writing PNG's _without the need of external libraries_ is a breeze with this excellent piece of code. Using his code SIL can load PNG's directly into layers without much effort and can save the whole display as a single PNG.
* -Adjusted- Anti-Aliasing algorithm "[thickline](https://github.com/ArminJo/STMF3-Discovery-Demos/blob/master/lib/graphics/src/thickLine.cpp)" written by Armin Joachimsmeyer (@ArminJo), also for an interseting platform, the STMF3. There are a lot examples of Xialin Wu's algorithm examples on the internet, but most of them are unusable if you want to draw thicker lines, leaving you strange moire or weird artifacts in the lines.
* I used a paper of Dusheng Wang about [Anti-Aliasing drawing lines](http://wscg.zcu.cz/WSCG2006/Papers_2006/Poster/B11-full.pdf). I used this simple concept to work out my own "thick circle" algorithm.
* For the non-aliased line and circle drawing I used the [Bresenham](https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/) algorithm's
* Using the BMF font format (and tool) from [Angelcode](https://www.angelcode.com/products/bmfont/) for crisp _anti aliased_ fonts without resorting to cumbersome and slow .ttf font renderers. In short: all \*.fnt files are accompanied with a single file containing all characters you want from given font cramped together in a single .png. BMF font file will give you required information about the dimensions and locations within that png file and also information about kerning, spacing and so on.

## Building
Its just a bunch of .c and .h files. Just compile & link them together with your own "main.c" program - or what you have - . See Makefile and examples directory how to do that if you are not sure. Only include one of the "...display.c" files for the target environment. 
Didn't try it in any IDE, I just used to program it in WSL2, linux in windows on the commandline. 
At this moment, the following environments are supported:
* winGDIdisplay.c : Windows 64bit (might work with 32bit too) environment, using WIN32 API and "plain old" GDI interface 
* winSDLdisplay.c : Windows 64bit (might also work with other environments SDL is ported to) SDL will give you hardware acceleration. All layers will be placed as separate textures in videoram, making updating much, much faster.
* x11display.c : Linux X-Windows environment. I used to write a lot of programs using XLib profesionally. Now I'm remembered why I hated it that much.
* lnxFBdisplay.c : Using Framebuffer of linux environment (/dev/fb and /dev/event2 (touchscreen) should be present), like raspberry PI. Saves the unneeded overhead of X Windows

Use the directive -D SIL_LIVEDANGEROUS to throw away guardrails and speed up your program if you dare to run with no-checking on uninitialized structs, out of bound arrays and NULL pointers. My code will work, but does yours ? ... 🤞

