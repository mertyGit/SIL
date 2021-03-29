# Simple Interactive Layers (SIL)

## About SIL

**S**imple **I**nteractive **L**ayers, or "**SIL**" for short, is a small collection of C-files to create decent looking GUI's, without the dependency of memory exhausting bloated libraries or complex frameworks that do require a steep learning curve to overcome. It is especially targeted at non-hardware accelerated, low resolution touch-screen displays, driven by small SOC's or single board computers, like raspberry PI or ESP32, but can run with same look and feel on 'normal' windows and Linux desktops, to speed up development and testing without having to flash/upload it and test it on the platform target.
Under the hood, it is using "plain framebuffers" - a large memory buffer that holds all pixel color information in a straight non-compressed manner.  

### Layers ?

Ah yes, the only basic concept you will have to understand to use this GUI. I will not bother you with *widgets,sprites,textures,backplanes* and *windows*. With SIL -almost- *everything is done on layers*. This makes designing a GUI, without touching a programming environment, so much easier ; just grab Gimp, Paint.Net , Photoshop or any other graphic program and allmost all of them works with the concept of layers. Export each layer to a .PNG file, write code to initialize your program and add a layer per PNG file, and you have your basic setup for the program, looking as good as intended. Now you can hide, move, filter, draw on these layers from your program. Even handle events about mouse/touch or keyboard presses. Everytime you ask SIL to update the screen, it will transform all layers (from bottom till top) to a single image that will send out to the "display" (a real display or a window in Windows or Linux).



A layer is able to contain pixels that are transparant, thus showing pixels of the underlying layer(s). It is also possible to set the transparency of the whole layer, this way you can "blend" pixels of multiple layers. Layers can be turned off, but also adjust to only show a smaller part of the layer, a "view" . Every layer has its own framebuffer which can have a different byte packing / color space, saving memory or making it easier to adapt an image format with the structure. For example, if a display can handle 24bit colors, you don't need to reserve memory for every layer for 24bit colors, if you are happy with less colors or don't need transparency, you can save precious memory. The whole framebuffer handling is hidden as part of basic handling & drawing functions, but you can have direct access to it if you want to speed things up. 


### Why ?

The -original- reason I wrote this, is because I needed a touchscreen GUI for my self-made CNC machine, using raspberry PI and a 7" touchscreen display but was thinking it replacing it later with a more low-level microprocessor board, like ESP32's. Most GUI's are mouse / windows based, where to complicated or works only on a specific platform and programming language (-version) and just simply to bloated that tops my CPU to 90% //only for running a simple GUI with buttons and texts//, processing time I needed to control my CNC motors and monitor safety systems (!). So, options left where free, open-source GUI-kinda-drawing primitives written for small systems, usaully targeted at arduino (-environment)

But, looking at these solutions, a specific display driver was needed and all drawing primitives - put pixel, draw line and so on -  were incorporated in this specific display drivers. Sometimes a lot sometimes only a few operations. Also, most of the time, this driver or module  was written for a specific SOC or in micropython or circuitpython, which is great for beginners, but also comes with additional performance en memory degradation (and incompatibility). I want to write my applications in any language it suites without worrying much about portability , device dependencies or debugging porting errors in stuff-I-don't-need... 

As final argument, most of the time, it just looks....meh... No anti-aliasing no full color (even if the display supports this) no alpha blending.  I know, on an arduino UNO you can't do much with speed and memory so you can't afford the luxury....But why shouldn't it be on better equiped environments ? If you don't need the speed of accelerated graphics hardware, but at least you have decent memory, why not make look good as if on your average PC ? Even doing so, why not design, develop and test the stuff on your own PC and only as final step cross-compile to target environment and deploy it.
So, I needed "something in the middle" and preferably re-usable for broad scala of displays and environments and that became SIL

### Speed ?

So, since it just a 2D touch GUI, in some cases presented by not-so-fast I2C displays, speed wasn't a concern. I can't notice if I press a button and display isn't updated in the first 100ms or so. But out of curiousity, I wanted to compare it with hardware-accelerated - read: GPU's - environments, but didn't want to learn the indepths of Direct3D, OpenGL or Vulcan (yet), so I used SDL for it (and yes, that needs an additional library) If you are looking for more speed, check SDL version. But keep in mind that basic concepts of layers-over-layers being drawn via direct framebuffers and updating the whole screen everytime is quite flexible, but not the fastest way to work. So...It is not a game engine or usable for streaming video or something (though, I can get around 30FPS on my heavy windows desktop if I really push it and adjust some SDL options)

### But what about .... 

But...there are tons, and tons of libraries out there....one must be doing a much, much better effort then this one. I'm pretty sure it is, actually, I peeked / borrowed code from a few of those. But I also like to tinker with it, adding new functionalities I, and mostly only myself, need and just try out graphic manipulations and have a simple environment for this for all the platforms and devices I work with. Besides...as an old fart that started programming C 35 years ago, I like to write "bare bone C", and if I have time, maybe move large sections of it to assembly, but I then need to target Intel, ARM and RISC-V environments... who knows...

### Advantages
* Small, and you can strip out more if you do not need some functionality (like filters, fonts, logging). I try to keep unnecesary memory allocations to a minimum. 
* Portable ! By sticking to standard ANSI-C and single threaded, if you find a C compiler for your environment, it will compile.
* Exact look-and-feel for every platform (besides from colordepth differences)
* Easy to use; no frameworks, weird libraries, strange configurator or build scripts, just use the *.c and *.h files together with your own files and development environment
* Is able to handle and translate most common color-pallet/packing methods from 32bit RGBA to 223 bit RGB under the hood.
* Single threaded, no dealings with race conditions, locking and handling multithreading on platforms that doesn't support it. Of course, you can make you own program multithreaded or even put the display/updating/events stuff inside a seperate thread.

### Disadvantages
* It is not lightning fast; It will be much faster in comparision with intepreted languages, but the choice of framebuffers-per-layer-with-multiple-colorspaces and updating whole screen everytime, makes it not suitable for high-speed games or video streaming/altering purposes.
* Texthandling; keyboard support en drawing text on layers is mostly written for shortkeys or entering configuration values, but not for writing long texts with scrollbars, cursors and so on. Although you can build on top of it, if you want to. 
* Static size; Usually it is written to write directly to a display with given resolution. So no windowing rescaling / auto adjustments and scrollbars.
* It is not written in  ....//Insert your own language here //.... . However, without doubt it isn't hard to integrate it with any language, since most languages do still use C(++)   libraries underneath. (and no, don't ask me for it...)



