﻿NDSummary.OnToolTipsLoaded("File:layer.c",{54:"<div class=\"NDToolTip TInformation LC\"><div class=\"TTSummary\">Layer are the main elements of SIL. Although &quot;the real thing&quot; is handled by framebuffers under every layer and pushed out to the display via platform specific display routines, as user you should not be knowing anything about that and just focus on the layer routines and if needed - primitive - drawing, imageloading and textdrawing routines.</div></div>",6:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype6\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\">SILLYR *sil_addLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">relx,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PName last\">rely,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td class=\"PName last\">width,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td class=\"PName last\">height,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td class=\"PName last\">type</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Create a layer and put to top of stack of layers</div></div>",25:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype25\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setHoverHandler(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"first\"></td><td class=\"PType\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PModifierQualifier first\">UINT (*</td><td class=\"PType\">hover</td><td class=\"PSymbols\">)(</td><td class=\"PName last\">SILEVENT *)</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set handler for &quot;hover&quot;, when mousepointer is above the given layer, it will trigger the given hoverhandler. Handlers will receive the event, including targeted layer and should return \'0\' for &quot;do nothing&quot; or \'1\' for &quot;update display&quot;.</div></div>",26:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype26\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setClickHandler(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"first\"></td><td class=\"PType\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PModifierQualifier first\">UINT (*</td><td class=\"PType\">click</td><td class=\"PSymbols\">)(</td><td class=\"PName last\">SILEVENT *)</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set handler for &quot;click&quot;, when mousepointer is above the given layer AND mousebutton has been pressed, it will trigger the given clickhandler.&nbsp; Handlers will receive the event, including targeted layer and should return \'0\' for &quot;do nothing&quot; or \'1\' for &quot;update display&quot;.</div></div>",22:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype22\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setKeyHandler(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"first\"></td><td class=\"PType\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">UINT&nbsp;</td><td></td><td class=\"PName last\">key,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">BYTE&nbsp;</td><td></td><td class=\"PName last\">modifiers,</td></tr><tr><td class=\"first\"></td><td class=\"PType\">BYTE&nbsp;</td><td></td><td class=\"PName last\">flags,</td></tr><tr><td class=\"PModifierQualifier first\">UINT (*</td><td class=\"PType\">keypress</td><td class=\"PSymbols\">)(</td><td class=\"PName last\">SILEVENT *)</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set handler for keyboard events.</div></div>",17:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype17\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setDragHandler(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"first\"></td><td class=\"PType\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PModifierQualifier first\">UINT (*</td><td class=\"PType\">drag</td><td class=\"PSymbols\">)(</td><td class=\"PName last\">SILEVENT *)</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set handler for &quot;drag&quot;.</div></div>",20:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype20\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_moveLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">x,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">y</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move layer to new position, relative to current position</div></div>",28:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype28\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_placeLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">x,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">y</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">place layer to new position, relative to upper left corner of display</div></div>",29:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype29\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_putPixelLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">x,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">y,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">red,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">green,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">blue,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">alpha</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Draw a pixel on given location inside a layer.</div></div>",30:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype30\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_blendPixelLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">x,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">y,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">red,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">green,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">blue,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">alpha</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Draw a pixel on given location inside a layer and blend it with existing pixel</div></div>",31:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype31\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_getPixelLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">x,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">y,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">red,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">green,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">blue,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">alpha</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">get red,green,blue and alpha information for pixel on given location inside a layer.</div></div>",27:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype27\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setFlags(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">flags</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">set layer flags</div></div>",33:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype33\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_clearFlags(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">flags</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">clear one or more flags from layer</div></div>",32:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype32\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\">UINT sil_checkFlags(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">flags</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">check if one or more flags are set for given layer</div></div>",35:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype35\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\">SILLYR *sil_getBottom()</div></div><div class=\"TTSummary\">get layer on the bottom of the stack</div></div>",36:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype36\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\">SILLYR *sil_getTop()</div></div><div class=\"TTSummary\">get the top most layer</div></div>",37:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype37\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_destroyLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">remove layer from stack and delete it</div></div>",56:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype56\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setAlphaLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">float</span>&nbsp;</td><td></td><td class=\"PName last\">alpha</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">set alpha blending factor for layer</div></div>",34:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype34\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setView(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">minx,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">miny,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">width,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">height</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set view of layer</div></div>",41:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype41\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_resetView(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Reset view settings of layer to default. Default values will be the same as the surface of the layer in total. So minx,miny at 0 and width&amp;height the same as layer width and height</div></div>",39:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype39\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\">UINT sil_resizeLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">minx,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">miny,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">width,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">height</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">resize the given layer to new dimensions and offset.&nbsp; Offset are the distances to add from pixellocation in original layer and the one in the resized one.</div></div>",40:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype40\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\">SILLYR *sil_PNGtoNewLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">filename,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">x,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">y</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Creat a new layer based on given PNG filename and place it on location x,y</div></div>",44:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype44\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_initSpriteSheet(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">hparts,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">vparts</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Initialize given layer as &quot;SpriteSheet&quot;, containing different images/frames, hence called \'sprites\', in a single image.&nbsp; Every sprite should have the same with and height and distance to eachother in the file.</div></div>",43:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype43\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_nextSprite(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move view of layer to next sprite (counting from left upper corner to right down corner). If there isn\'t, sprite will be set to first again</div></div>",55:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype55\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_prevSprite(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move view of layer to previous sprite (counting from left upper corner to right down corner). If there isn\'t, sprite will be set to last sprite</div></div>",45:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype45\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setSprite(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">pos</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set view of layer to sprite with index &quot;pos&quot; (counting from 0, left upper corner to right down corner).</div></div>",46:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype46\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_toTop(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move given layer to the top of the stack of layers</div></div>",38:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype38\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_toBottom(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move given layer to the top of the stack of layers</div></div>",57:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype57\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_toAbove(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">target</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move given layer just above target layer in stack</div></div>",49:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype49\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_toBelow(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">target</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move given layer to the bottom of the stack of layers</div></div>",50:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype50\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_swap(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">target</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Swap position of two layers in stack with eachother</div></div>",21:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype21\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\">SILLYR *sil_addCopy(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">relx,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">rely</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">creates a new layer, but copies all layer information to new one.The New layer will be placed at given x,y postion</div></div>",16:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype16\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\">SILLYR *sil_addInstance(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">relx,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">rely</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Create an extra instance of given layer. New layer will share the same framebuffer as the original, so all drawings and filters on it will be the same as the original, however, the new layer can have different position, view, visability or handlers.</div></div>",51:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype51\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_hide(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Hide the given layer.</div></div>",52:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype52\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_show(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Unhide the given layer.</div></div>",53:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype53\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_clearLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">clear all contents of layer</div></div>"});