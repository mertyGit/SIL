﻿NDContentPage.OnToolTipsLoaded({7:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype7\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setTimerHandler(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PModifierQualifier first\">UINT (*</td><td class=\"PType\">timer</td><td class=\"PSymbols\">)(</td><td class=\"PName last\">SILEVENT *)</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">set timer handler</div></div>",18:"<div class=\"NDToolTip TGroup LC\"><div class=\"TTSummary\">Naming denotes the amount of bits per color and storage of bits or bytes.</div></div>",19:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype19\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setTimeval(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">UINT&nbsp;</td><td class=\"PName last\">amount</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">set timer to given amount of miliseconds</div></div>",20:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype20\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_moveLayer(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">x,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">y</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move layer to new position, relative to current position</div></div>",21:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype21\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\">SILLYR *sil_addCopy(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">relx,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td></td><td class=\"PName last\">rely</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">creates a new layer, but copies all layer information to new one.The New layer will be placed at given x,y postion</div></div>",24:"<div class=\"NDToolTip TGroup LC\"><div class=\"TTSummary\">See also CSS color codes : https://&#8203;ao&#8203;.ms&#8203;/rbg-color-codes&#8203;/</div></div>",27:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype27\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setFlags(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">flags</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">set layer flags</div></div>",34:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype34\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setView(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">minx,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">miny,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">width,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">height</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set view of layer</div></div>",38:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype38\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_toBottom(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move given layer to the top of the stack of layers</div></div>",41:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype41\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_resetView(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Reset view settings of layer to default. Default values will be the same as the surface of the layer in total. So minx,miny at 0 and width&amp;height the same as layer width and height</div></div>",43:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype43\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_nextSprite(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move view of layer to next sprite (counting from left upper corner to right down corner). If there isn\'t, sprite will be set to first again</div></div>",44:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype44\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_initSpriteSheet(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">hparts,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">vparts</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Initialize given layer as &quot;SpriteSheet&quot;, containing different images/frames, hence called \'sprites\', in a single image.&nbsp; Every sprite should have the same with and height and distance to eachother in the file.</div></div>",45:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype45\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setSprite(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">pos</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Set view of layer to sprite with index &quot;pos&quot; (counting from 0, left upper corner to right down corner).</div></div>",46:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype46\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_toTop(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move given layer to the top of the stack of layers</div></div>",50:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype50\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_swap(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer,</td></tr><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">target</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Swap position of two layers in stack with eachother</div></div>",51:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype51\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_hide(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Hide the given layer.</div></div>",52:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype52\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_show(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Unhide the given layer.</div></div>",55:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype55\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_prevSprite(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Move view of layer to previous sprite (counting from left upper corner to right down corner). If there isn\'t, sprite will be set to last sprite</div></div>",73:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype73\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_addLayerGroup(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">SILGROUP&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">group,</td></tr><tr><td class=\"PType first\">SILLYR&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">layer</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Add layer to group</div></div>",90:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype90\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\">BYTE sil_getMouse(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">x,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">int</span>&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">y</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Retrieves the latest coordinates of the mousepointer and status of mouse buttons</div></div>"});