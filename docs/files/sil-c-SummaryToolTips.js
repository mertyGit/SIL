﻿NDSummary.OnToolTipsLoaded("File:sil.c",{47:"<div class=\"NDToolTip TInformation LC\"><div class=\"TTSummary\">The standard flow of a program that is using SIL is like this:</div></div>",80:"<div class=\"NDToolTip TInformation LC\"><div class=\"TTSummary\">This will create a program that will show a small window with a white area and green filled rectange in it</div></div>",3:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype3\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\">UINT sil_initSIL(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">width,</td></tr><tr><td class=\"PType first\">UINT&nbsp;</td><td></td><td class=\"PName last\">height,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">title,</td></tr><tr><td class=\"PType first\"><span class=\"SHKeyword\">void</span>&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">hInstance</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">initialize SIL context for all other calls. It will be used to contain program related stuff like initializing logging and global variables</div></div>",5:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype5\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">void</span> sil_mainLoop()</div></div><div class=\"TTSummary\">Wait for events from timer, mouse and/or keyboard to handle.&nbsp; this loop will continiously process incoming display events and redeligate them to the appropriate program or layer handlers</div></div>",4:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype4\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">void</span> sil_quitLoop()</div></div><div class=\"TTSummary\">signal to stop sil_mainLoop. Usually to instruct SIL to clean up and quit the program.</div></div>",10:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype10\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\"><span class=\"SHKeyword\">void</span> sil_destroySIL()</div></div><div class=\"TTSummary\">Close &amp; cleanup SIL</div></div>",12:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype12\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\">UINT sil_setLog(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\"><span class=\"SHKeyword\">char</span>&nbsp;</td><td class=\"PSymbols\">*</td><td class=\"PName last\">logname,</td></tr><tr><td class=\"PType first\">BYTE&nbsp;</td><td></td><td class=\"PName last\">flags</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Initialize logging parameters</div></div>",13:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype13\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">const char</span> * sil_err2Txt(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">UINT&nbsp;</td><td class=\"PName last\">errorcode</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">Optional function to convert any internal SIL error code to a more human understandable form.</div></div>",7:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype7\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setTimerHandler(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PModifierQualifier first\">UINT (*</td><td class=\"PType\">timer</td><td class=\"PSymbols\">)(</td><td class=\"PName last\">SILEVENT *)</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">set timer handler</div></div>",19:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype19\" class=\"NDPrototype WideForm\"><div class=\"PSection PParameterSection CStyle\"><table><tr><td class=\"PBeforeParameters\"><span class=\"SHKeyword\">void</span> sil_setTimeval(</td><td class=\"PParametersParentCell\"><table class=\"PParameters\"><tr><td class=\"PType first\">UINT&nbsp;</td><td class=\"PName last\">amount</td></tr></table></td><td class=\"PAfterParameters\">)</td></tr></table></div></div><div class=\"TTSummary\">set timer to given amount of miliseconds</div></div>",2:"<div class=\"NDToolTip TFunction LC\"><div id=\"NDPrototype2\" class=\"NDPrototype\"><div class=\"PSection PPlainSection\">UINT sil_getTimeval()</div></div><div class=\"TTSummary\">If you have trouble remembering things, retrieve the amount of miliseconds you did set your timer to</div></div>"});