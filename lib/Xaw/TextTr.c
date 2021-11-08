/* $XConsortium: TextTr.c,v 1.4 88/10/07 09:28:18 swick Exp $ */

char defaultTextTranslations[] =
"\
Ctrl<Key>F:	forward-character() \n\
Ctrl<Key>B:	backward-character() \n\
Ctrl<Key>D:	delete-next-character() \n\
Ctrl<Key>A:	beginning-of-line() \n\
Ctrl<Key>E:	end-of-line() \n\
Ctrl<Key>H:	delete-previous-character() \n\
Ctrl<Key>J:	newline-and-indent() \n\
Ctrl<Key>K:	kill-to-end-of-line() \n\
Ctrl<Key>L:	redraw-display() \n\
Ctrl<Key>M:	newline() \n\
Ctrl<Key>N:	next-line() \n\
Ctrl<Key>O:	newline-and-backup() \n\
Ctrl<Key>P:	previous-line() \n\
Ctrl<Key>V:	next-page() \n\
Ctrl<Key>W:	kill-selection() \n\
Ctrl<Key>Y:	unkill() \n\
Ctrl<Key>Z:	scroll-one-line-up() \n\
Meta<Key>F:	forward-word() \n\
Meta<Key>B:	backward-word() \n\
Meta<Key>I:	insert-file() \n\
Meta<Key>K:	kill-to-end-of-paragraph() \n\
Meta<Key>V:	previous-page() \n\
Meta<Key>Y:	stuff() \n\
Meta<Key>Z:	scroll-one-line-down() \n\
:Meta<Key>d:	delete-next-word() \n\
:Meta<Key>D:	kill-word() \n\
:Meta<Key>h:	delete-previous-word() \n\
:Meta<Key>H:	backward-kill-word() \n\
:Meta<Key>\\<:	beginning-of-file() \n\
:Meta<Key>\\>:	end-of-file() \n\
:Meta<Key>]:	forward-paragraph() \n\
:Meta<Key>[:	backward-paragraph() \n\
~Shift Meta<Key>Delete:		delete-previous-word() \n\
 Shift Meta<Key>Delete:		backward-kill-word() \n\
~Shift Meta<Key>Backspace:	delete-previous-word() \n\
 Shift Meta<Key>Backspace:	backward-kill-word() \n\
<Key>Right:	forward-character() \n\
<Key>Left:	backward-character() \n\
<Key>Down:	next-line() \n\
<Key>Up:	previous-line() \n\
<Key>Delete:	delete-previous-character() \n\
<Key>BackSpace:	delete-previous-character() \n\
<Key>Linefeed:	newline-and-indent() \n\
<Key>Return:	newline() \n\
<Key>:		insert-char() \n\
<FocusIn>:	focus-in() \n\
<FocusOut>:	focus-out() \n\
<Btn1Down>:	select-start() \n\
<Btn1Motion>:	extend-adjust() \n\
<Btn1Up>:	extend-end(PRIMARY, CUT_BUFFER0) \n\
<Btn2Down>:	insert-selection(PRIMARY, CUT_BUFFER0) \n\
<Btn3Down>:	extend-start() \n\
<Btn3Motion>:	extend-adjust() \n\
<Btn3Up>:	extend-end(PRIMARY, CUT_BUFFER0) \
";
