;;; -*- Mode: LISP; Syntax: Common-lisp; Package: XLIB; Base: 10; Lowercase: Yes -*-

;; This file contains some of the system dependent code for CLX

;;;
;;;			 TEXAS INSTRUMENTS INCORPORATED
;;;				  P.O. BOX 2909
;;;			       AUSTIN, TEXAS 78769
;;;
;;; Copyright (C) 1987 Texas Instruments Incorporated.
;;;
;;; Permission is granted to any individual or institution to use, copy, modify,
;;; and distribute this software, provided that this complete copyright and
;;; permission notice is maintained, intact, in all copies and supporting
;;; documentation.
;;;
;;; Texas Instruments Incorporated provides this software "as is" without
;;; express or implied warranty.
;;;

(in-package 'xlib :use '(lisp))

(export '(
	  default-error-handler
	  define-condition))

#+explorer
(zwei:define-indentation event-case (1 1))

;;; Number of seconds to wait for a reply to a server request
(defparameter *reply-timeout* nil) 

#-(or clx-overlapping-arrays (not clx-little-endian))
(progn
  (defconstant *word-0* 0)
  (defconstant *word-1* 1)

  (defconstant *long-0* 0)
  (defconstant *long-1* 1)
  (defconstant *long-2* 2)
  (defconstant *long-3* 3))

#-(or clx-overlapping-arrays clx-little-endian)
(progn
  (defconstant *word-0* 1)
  (defconstant *word-1* 0)

  (defconstant *long-0* 3)
  (defconstant *long-1* 2)
  (defconstant *long-2* 1)
  (defconstant *long-3* 0))

;;; Set some compiler-options for often used code

(eval-when (eval compile load)

(defconstant *buffer-speed* 3 "Speed compiler option for buffer code.")
(defconstant *buffer-safety* 0 "Safety compiler option for buffer code.")

(defmacro declare-bufmac ()
  `(declare (optimize (speed ,*buffer-speed*) (safety ,*buffer-safety*))))

;;; It's my impression that in lucid there's some way to make a declaration
;;; called fast-entry or something that causes a function to not do some
;;; checking on args. Sadly, we have no lucid manuals here.  If such a
;;; declaration is available, it would be a good idea to make it here when
;;; *buffer-speed* is 3 and *buffer-safety* is 0.
(defmacro declare-buffun ()
  `(declare (optimize (speed ,*buffer-speed*) (safety ,*buffer-safety*))))

)

(proclaim '(inline card8->int8 int8->card8
		   card16->int16 int16->card16
		   card32->int32 int32->card32))

(defun card8->int8 (x)
  (declare (type card8 x))
  (declare-values int8)
  (declare-buffun)
  (the int8 (if (logbitp 7 x)
		(the int8 (- x #x100))
	      x)))

(defun int8->card8 (x)
  (declare (type int8 x))
  (declare-values card8)
  (declare-buffun)
  (the card8 (ldb (byte 8 0) x)))

(defun card16->int16 (x)
  (declare (type card16 x))
  (declare-values int16)
  (declare-buffun)
  (the int16 (if (logbitp 15 x)
		 (the int8 (- x #x10000))
		 x)))

(defun int16->card16 (x)
  (declare (type int16 x))
  (declare-values card16)
  (declare-buffun)
  (the card16 (ldb (byte 16 0) x)))

#-genera
(defun card32->int32 (x)
  (declare (type card32 x))
  (declare-values int32)
  (declare-buffun)
  (the int32 (if (logbitp 31 x)
		 (the int32 (- x #x100000000))
		 x)))

#+genera
(defun card32->int32 (x)
  (macrolet ((signify (x)
	       ;; 7.1 is defective
	       (if (= (sys:%logldb (byte 32 0) #x80000000) #x80000000)
		   `(if (logbitp 31 ,x)
			(sys:%logdpb (ldb (byte 8 24) ,x) (byte 8 24) (ldb (byte 24 0) ,x))
			,x)
		   `(sys:%logldb (byte 32 0) ,x))))
    (signify x)))

(defun int32->card32 (x)
  (declare (type int32 x))
  (declare-values card32)
  (declare-buffun)
  (the card32 (ldb (byte 32 0) x)))

(proclaim '(inline aref-card8 aset-card8 aref-int8 aset-int8))

(defun aref-card8 (a i)
  (declare (type buffer-bytes a)
	   (type array-index i))
  (declare-values card8)
  (declare-buffun)
  (the card8 (aref a i)))

(defun aset-card8 (v a i)
  (declare (type card8 v)
	   (type buffer-bytes a)
	   (type array-index i))
  (declare-buffun)
  (setf (aref a i) v))

(defun aref-int8 (a i)
  (declare (type buffer-bytes a)
	   (type array-index i))
  (declare-values int8)
  (declare-buffun)
  (card8->int8 (aref a i)))

(defun aset-int8 (v a i)
  (declare (type int8 v)
	   (type buffer-bytes a)
	   (type array-index i))
  (declare-buffun)
  (setf (aref a i) (int8->card8 v)))

#+clx-overlapping-arrays
(proclaim '(inline aref-card16 aref-int16 aref-card32 aref-int32 aref-card29
		   aset-card16 aset-int16 aset-card32 aset-int32 aset-card29))

#+(and clx-overlapping-arrays genera)
(progn

(defun aref-card16 (a i)
  (aref a i))

(defun aset-card16 (v a i)
  (setf (aref a i) v))

(defun aref-int16 (a i)
  (card16->int16 (aref a i)))

(defun aset-int16 (v a i)
  (setf (aref a i) (int16->card16 v))
  v)

(defun aref-card32 (a i)
  (int32->card32 (aref a i)))

(defun aset-card32 (v a i)
  (setf (aref a i) (card32->int32 v)))

(defun aref-int32 (a i) (aref a i))

(defun aset-int32 (v a i)
  (setf (aref a i) v))

(defun aref-card29 (a i) (aref a i))

(defun aset-card29 (v a i)
  (setf (aref a i) v))

)

#+(and clx-overlapping-arrays (or explorer lambda cadr))
(progn

(defun aref-card16 (a i)
  (aref a i))

(defun aset-card16 (v a i)
  (setf (aref a i) v))

(defun aref-int16 (a i)
  (card16->int16 (aref a i)))

(defun aset-int16 (v a i)
  (setf (aref a i) (int16->card16 v))
  v)

(defun aref-card32 (a i)
  (aref a i))

(defun aset-card32 (v a i)
  (setf (aref a i) v))

(defun aref-int32 (a i)
  (card32->int32 (aref a i)))

(defun aset-int32 (v a i)
  (setf (aref a i) (int32->card32 v))
  v)

(defun aref-card29 (a i)
  (aref a i))

(defun aset-card29 (v a i)
  (setf (aref a i) v))

)

#+excl
(progn
  
  (defun aref-card16 (a i)
    (declare (type buffer-bytes a)
	     (type array-index i))
    (declare-values card16)
    (declare-buffun)
    (the card16 (sys:memref a #.(comp::mdparam 'comp::md-svector-data0-adj) i
			    :unsigned-word)))
  
  (defun aset-card16 (v a i)
    (declare (type card16 v)
	     (type buffer-bytes a)
	     (type array-index i))
    (declare-buffun)
    (setf (sys:memref a #.(comp::mdparam 'comp::md-svector-data0-adj) i
		      :unsigned-word) v))
  
  (defun aref-int16 (a i)
    (declare (type buffer-bytes a)
	     (type array-index i))
    (declare-values int16)
    (declare-buffun)
    (the int16 (sys:memref a #.(comp::mdparam 'comp::md-svector-data0-adj) i
			   :signed-word)))
  
  (defun aset-int16 (v a i)
    (declare (type int16 v)
	     (type buffer-bytes a)
	     (type array-index i))
    (declare-buffun)
    (setf (sys:memref a #.(comp::mdparam 'comp::md-svector-data0-adj) i
		      :signed-word) v))
  
  (defun aref-card32 (a i)
    (declare (type buffer-bytes a)
	     (type array-index i))
    (declare-values card32)
    (declare-buffun)
    (the card32 (sys:memref a #.(comp::mdparam 'comp::md-svector-data0-adj) i
			    :unsigned-long)))
    
  (defun aset-card32 (v a i)
    (declare (type card32 v)
	     (type buffer-bytes a)
	     (type array-index i))
    (declare-buffun)
    (setf (sys:memref a #.(comp::mdparam 'comp::md-svector-data0-adj) i
		      :unsigned-long) v))

  (defun aref-int32 (a i)
    (declare (type buffer-bytes a)
	     (type array-index i))
    (declare-values int32)
    (declare-buffun)
    (the int32 (sys:memref a #.(comp::mdparam 'comp::md-svector-data0-adj) i
			   :signed-long)))
    
  (defun aset-int32 (v a i)
    (declare (type int32 v)
	     (type buffer-bytes a)
	     (type array-index i))
    (declare-buffun)
    (setf (sys:memref a #.(comp::mdparam 'comp::md-svector-data0-adj) i
		      :signed-long) v))

  (defun aref-card29 (a i)
    ;; Do I need to mask off a few bits here?  XXX
    (declare (type buffer-bytes a)
	     (type array-index i))
    (declare-values card29)
    (declare-buffun)
    (the card29 (sys:memref a #.(comp::mdparam 'comp::md-svector-data0-adj) i
			    :unsigned-long)))

  (defun aset-card29 (v a i)
    (declare (type card29 v)
	     (type buffer-bytes a)
	     (type array-index i))
    (declare-buffun)
    (setf (sys:memref a #.(comp::mdparam 'comp::md-svector-data0-adj) i
		      :unsigned-long) v))

)

#-(or excl clx-overlapping-arrays)
(progn

(defun aref-card16 (a i)
  (declare (type buffer-bytes a)
	   (type array-index i))
  (declare-values card16)
  (declare-buffun)
  (the card16
       (logior (the card16
		    (ash (the card8 (aref a (index+ i *word-1*))) 8))
	       (the card8
		    (aref a (index+ i *word-0*))))))

(defun aset-card16 (v a i)
  (declare (type card16 v)
	   (type buffer-bytes a)
	   (type array-index i))
  (declare-buffun)
  (setf (aref a (index+ i *word-1*)) (the card8 (ldb (byte 8 8) v))
	(aref a (index+ i *word-0*)) (the card8 (ldb (byte 8 0) v)))
  v)

(defun aref-int16 (a i)
  (declare (type buffer-bytes a)
	   (type array-index i))
  (declare-values int16)
  (declare-buffun)
  (the int16
       (logior (the int16
		    (ash (the int8 (aref-int8 a (index+ i *word-1*))) 8))
	       (the card8
		    (aref a (index+ i *word-0*))))))

(defun aset-int16 (v a i)
  (declare (type int16 v)
	   (type buffer-bytes a)
	   (type array-index i))
  (declare-buffun)
  (setf (aref a (index+ i *word-1*)) (the card8 (ldb (byte 8 8) v))
	(aref a (index+ i *word-0*)) (the card8 (ldb (byte 8 0) v)))
  v)

(defun aref-card32 (a i)
  (declare (type buffer-bytes a)
	   (type array-index i))
  (declare-values card32)
  (declare-buffun)
  (the card32
       (logior (the card32
		    (ash (the card8 (aref a (index+ i *long-3*))) 24))
	       (the card29
		    (ash (the card8 (aref a (index+ i *long-2*))) 16))
	       (the card16
		    (ash (the card8 (aref a (index+ i *long-1*))) 8))
	       (the card8
		    (aref a (index+ i *long-0*))))))

(defun aset-card32 (v a i)
  (declare (type card32 v)
	   (type buffer-bytes a)
	   (type array-index i))
  (declare-buffun)
  (setf (aref a (index+ i *long-3*)) (the card8 (ldb (byte 8 24) v))
	(aref a (index+ i *long-2*)) (the card8 (ldb (byte 8 16) v))
	(aref a (index+ i *long-1*)) (the card8 (ldb (byte 8 8) v))
	(aref a (index+ i *long-0*)) (the card8 (ldb (byte 8 0) v)))
  v)

(defun aref-int32 (a i)
  (declare (type buffer-bytes a)
	   (type array-index i))
  (declare-values int32)
  (declare-buffun)
  (the int32
       (logior (the int32
		    (ash (the int8 (aref-int8 a (index+ i *long-3*))) 24))
	       (the card29
		    (ash (the card8 (aref a (index+ i *long-2*))) 16))
	       (the card16
		    (ash (the card8 (aref a (index+ i *long-1*))) 8))
	       (the card8
		    (aref a (index+ i *long-0*))))))

(defun aset-int32 (v a i)
  (declare (type int32 v)
	   (type buffer-bytes a)
	   (type array-index i))
  (declare-buffun)
  (setf (aref a (index+ i *long-3*)) (the card8 (ldb (byte 8 24) v))
	(aref a (index+ i *long-2*)) (the card8 (ldb (byte 8 16) v))
	(aref a (index+ i *long-1*)) (the card8 (ldb (byte 8 8) v))
	(aref a (index+ i *long-0*)) (the card8 (ldb (byte 8 0) v)))
  v)

(defun aref-card29 (a i)
  (declare (type buffer-bytes a)
	   (type array-index i))
  (declare-values card29)
  (declare-buffun)
  (the card29
       (logior (the card29
		    (ash (the card8 (aref a (index+ i *long-3*))) 24))
	       (the card29
		    (ash (the card8 (aref a (index+ i *long-2*))) 16))
	       (the card16
		    (ash (the card8 (aref a (index+ i *long-1*))) 8))
	       (the card8
		    (aref a (index+ i *long-0*))))))

(defun aset-card29 (v a i)
  (declare (type card29 v)
	   (type buffer-bytes a)
	   (type array-index i))
  (declare-buffun)
  (setf (aref a (index+ i *long-3*)) (the card8 (ldb (byte 8 24) v))
	(aref a (index+ i *long-2*)) (the card8 (ldb (byte 8 16) v))
	(aref a (index+ i *long-1*)) (the card8 (ldb (byte 8 8) v))
	(aref a (index+ i *long-0*)) (the card8 (ldb (byte 8 0) v)))
  v)

)

(defsetf aref-card8 (a i) (v)
  `(aset-card8 ,v ,a ,i))

(defsetf aref-int8 (a i) (v)
  `(aset-int8 ,v ,a ,i))

(defsetf aref-card16 (a i) (v)
  `(aset-card16 ,v ,a ,i))

(defsetf aref-int16 (a i) (v)
  `(aset-int16 ,v ,a ,i))

(defsetf aref-card32 (a i) (v)
  `(aset-card32 ,v ,a ,i))

(defsetf aref-int32 (a i) (v)
  `(aset-int32 ,v ,a ,i))

(defsetf aref-card29 (a i) (v)
  `(aset-card29 ,v ,a ,i))

;;; Other random conversions

(defun rgb-val->card16 (value)
  (declare (type float value))
  (declare-buffun)
  ;; Convert VALUE from float to card16
  (the card16 (identity (truncate (the float value) #.(/ 1.0 #xffff)))))

(defun card16->rgb-val (value) 
  (declare (type card16 value))
  (declare-buffun)
  ;; Convert VALUE from card16 to float
  (the float (/ (the card16 value) #.(float #xffff))))

(defun radians->int16 (value)
  ;; Short floats are good enough
  (declare (type float value))
  (declare-values int16)
  (declare-buffun)
  (the int16 (identity (round (* value 180.0s0 64.0s0) #.(coerce pi 'short-float)))))

(defun int16->radians (value)
  ;; Short floats are good enough
  (declare (type int16 value))
  (declare-values short-float)
  (declare-buffun)
  (the short-float (* value #.(coerce (/ pi 180.0 64.0) 'short-float))))

;;; Character transformation

;;; This stuff transforms chars to ascii codes in card8's and back.
;;; You might have to hack it a little to get it to work for your machine.

(eval-when (eval compile)
(defparameter *char-to-ascii-alist*
	      '#.`(#-lispm
		   ;; The normal ascii codes for the control characters.
		   ,@`((#\Return . 13)
		       (#\Linefeed . 10)
		       (#\Rubout . 127)
		       (#\Page . 12)
		       (#\Tab . 9)
		       (#\Backspace . 8)
		       (#\Newline . 10)
		       (#\Space . 32))
		   ;; One the lispm, #\Newline is #\Return, but we'd really like
		   ;; #\Newline to translate to ascii code 10, so we swap the
		   ;; Ascii codes for #\Return and #\Linefeed. We also provide
		   ;; mappings from the counterparts of these control characters
		   ;; so that the character mapping from the lisp machine
		   ;; character set to ascii is invertible.
		   #+lispm
		   ,@`((#\Return . 10)   (,(code-char  10) . ,(char-code #\Return))
		       (#\Linefeed . 13) (,(code-char  13) . ,(char-code #\Linefeed))
		       (#\Rubout . 127)  (,(code-char 127) . ,(char-code #\Rubout))
		       (#\Page . 12)     (,(code-char  12) . ,(char-code #\Page))
		       (#\Tab . 9)       (,(code-char   9) . ,(char-code #\Tab))
		       (#\Backspace . 8) (,(code-char   8) . ,(char-code #\Backspace))
		       (#\Newline . 10)  (,(code-char  10) . ,(char-code #\Newline))
		       (#\Space . 32)    (,(code-char  32) . ,(char-code #\Space)))
		   ;; The rest of the common lisp charater set with the normal
		   ;; ascii codes for them.
		   (#\! . 33) (#\" . 34) (#\# . 35) (#\$ . 36)
		   (#\% . 37) (#\& . 38) (#\' . 39) (#\( . 40)
		   (#\) . 41) (#\* . 42) (#\+ . 43) (#\, . 44)
		   (#\- . 45) (#\. . 46) (#\/ . 47) (#\0 . 48)
		   (#\1 . 49) (#\2 . 50) (#\3 . 51) (#\4 . 52)
		   (#\5 . 53) (#\6 . 54) (#\7 . 55) (#\8 . 56)
		   (#\9 . 57) (#\: . 58) (#\; . 59) (#\< . 60)
		   (#\= . 61) (#\> . 62) (#\? . 63) (#\@ . 64)
		   (#\A . 65) (#\B . 66) (#\C . 67) (#\D . 68)
		   (#\E . 69) (#\F . 70) (#\G . 71) (#\H . 72)
		   (#\I . 73) (#\J . 74) (#\K . 75) (#\L . 76)
		   (#\M . 77) (#\N . 78) (#\O . 79) (#\P . 80)
		   (#\Q . 81) (#\R . 82) (#\S . 83) (#\T . 84)
		   (#\U . 85) (#\V . 86) (#\W . 87) (#\X . 88)
		   (#\Y . 89) (#\Z . 90) (#\[ . 91) (#\\ . 92)
		   (#\] . 93) (#\^ . 94) (#\_ . 95) (#\` . 96)
		   (#\a . 97) (#\b . 98) (#\c . 99) (#\d . 100)
		   (#\e . 101) (#\f . 102) (#\g . 103) (#\h . 104)
		   (#\i . 105) (#\j . 106) (#\k . 107) (#\l . 108)
		   (#\m . 109) (#\n . 110) (#\o . 111) (#\p . 112)
		   (#\q . 113) (#\r . 114) (#\s . 115) (#\t . 116)
		   (#\u . 117) (#\v . 118) (#\w . 119) (#\x . 120)
		   (#\y . 121) (#\z . 122) (#\{ . 123) (#\| . 124)
		   (#\} . 125) (#\~ . 126)))


(pushnew :clx-ascii *features*)
(dolist (pair *char-to-ascii-alist*)
  (when (not (= (char-code (car pair)) (cdr pair)))
    (return (setq *features* (delete :clx-ascii *features*)))))

)

(proclaim '(inline char->card8 card8->char))

#-clx-ascii
(progn
  
(defparameter *char-to-card8-translation-table*
	      '#.(let ((array (make-array
				(let ((max-char-code 255))
				  (dolist (pair *char-to-ascii-alist*)
				    (setq max-char-code
					  (max max-char-code (char-code (car pair)))))
				  (1+ max-char-code))
				:element-type 'card8)))
		   (dotimes (i (length array))
		     (setf (aref array i) (mod i 256)))
		   (dolist (pair *char-to-ascii-alist*)
		     (setf (aref array (char-code (car pair))) (cdr pair)))
		   array))

(defparameter *card8-to-char-translation-table*
	      '#.(let ((array (make-string 256)))
		   (dotimes (i (length array))
		     (setf (aref array i) (code-char (mod i 256))))
		   (dolist (pair *char-to-ascii-alist*)
		     (setf (aref array (cdr pair)) (car pair)))
		   array))

(defun char->card8 (char)
  (declare (type string-char char))
  (declare-buffun)
  (the card8 (aref (the (simple-array card8 (*)) *char-to-card8-translation-table*)
		   (the array-index (char-code char)))))

(defun card8->char (card8)
  (declare (type card8 card8))
  (declare-buffun)
  (the string-char (aref (the simple-string *card8-to-char-translation-table*) card8)))

(defun check-character-mapping-consistency ()
  (dotimes (i 256)
    (unless (= i (char->card8 (card8->char i)))
      (warn "The card8->char mapping is not invertible through char->card8.  Info:~%~S"
	    (list i (card8->char i) (char->card8 (card8->char i))))
      (return nil)))
  (dotimes (i (length *char-to-card8-translation-table*))
    (let ((char (code-char i)))
      (unless (eql char (card8->char (char->card8 char)))
	(warn "The char->card8 mapping is not invertible through card8->char.  Info:~%~S"
	      (list char (char->card8 char) (card8->char (char->card8 char))))
	(return nil)))))

(check-character-mapping-consistency)

)

#+clx-ascii
(progn
 
(defun char->card8 (char)
  (declare (type string-char char))
  (declare-buffun)
  (the card8 (char-code char)))

(defun card8->char (card8)
  (declare (type card8 card8))
  (declare-buffun)
  (the string-char (code-char card8)))

(eval-when (eval compile)
  (setq *features* (delete :clx-ascii *features*)))

)


;;-----------------------------------------------------------------------------
;; Process Locking
;;
;;	Common-Lisp doesn't provide process locking primitives, so we define
;;	our own here, based on Zetalisp primitives.  Holding-Lock is very
;;	similar to with-lock on The TI Explorer, and a little more efficient
;;	than with-process-lock on a Symbolics.

#+excl
(defun make-process-lock ()
  (if (excl::scheduler-running-p)
      (mp::make-process-lock)))

#+imach
(defun make-process-lock ()
  (process:make-lock "CLX Lock" :recursive t))

#-(or excl imach)
(defun make-process-lock ()
  nil)

#+imach
(defmacro holding-lock ((locator &optional whostate) &body body)
  whostate
  `(process:with-lock (,locator)
     ,@body))

#+(and lispm (not imach))
(defmacro holding-lock ((locator &optional whostate) &body body)
  ; This macro is for use in a multi-process environment.
  (let ((lock (gensym)) (have-lock (gensym)))
    `(let* ((,lock (zl:locf ,locator))
	    (,have-lock (eq (car ,lock) sys:current-process)))
       (unwind-protect 
	   (progn (unless ,have-lock
		    ;; Redundant, but saves time if not locked.
		    (or #+explorer
			(si:%store-conditional ,lock nil sys:current-process)
			#-explorer
			(sys:store-conditional ,lock nil sys:current-process)
			(sys:process-lock ,lock ,@(when whostate `(nil ,whostate)))))
		  ,@body)
	 (unless ,have-lock
	   #+explorer
	   (si:%store-conditional ,lock sys:current-process nil)
	   #-explorer
	   (sys:store-conditional ,lock sys:current-process nil))))))

#+excl
(defun event-listen-error ()
  (error "The only CLX function call allowed from a process wait \
       function is event-listen with timeout 0."))

#+excl
;;
;; Note that there is a special hack here.  If the current process is nil it
;; means we're running in the scheduler stack group, which means in turn that
;; we're running a process wait function.  This wait functions should *always*
;; be: (event-listen display 0).  So if we are running in the scheduler and the
;; lock isn't already being held, just run the body without trying to grab the
;; lock.  If the lock *is* already being held we have to throw out of the
;; event-listen.
;;
(defmacro holding-lock ((locator &optional whostate) &body body)
  ;; This macro is for use in a multi-process environment.
  (let ((lock (gensym)) (curproc (gensym)) (locker (gensym))
	(scheduling (gensym)) (obtained-lock (gensym)))
    
    `(let ((,scheduling (excl::scheduler-running-p))
	   ,lock ,curproc ,locker ,obtained-lock)
       
       (declare (special *inside-event-listen-catch*
			 mp::*current-process*))
       
       (when ,scheduling
	 (setq ,lock ,locator)
	 (setq ,curproc mp::*current-process*; nil if in scheduler (wait fun)
	       ,locker (mp::process-lock-locker ,lock)))
       
       (unwind-protect
	    (progn
	      (when ,scheduling
		(if (and (null ,curproc) ,locker)
		    (if (and (boundp '*inside-event-listen-catch*)
			     *inside-event-listen-catch*)
			(throw 'event-listen :would-block)
		      (event-listen-error)))
		(when (not (eq ,locker ,curproc))
		  (mp::process-lock ,lock ,curproc
				    ,@(when whostate `(,whostate)))
		  (setq ,obtained-lock t)))
	      ,@body)
	 (if ,obtained-lock
	     (mp::process-unlock ,lock ,curproc))))))

;; Lucid has a process locking mechanism as well under release 3.0
#+lcl3.0
(defmacro holding-lock ((locator &optional whostate) &body body)
  whostate ;not implemented in lcl.
  `(lcl:with-process-lock (,locator)
     ,@body))

;; If you're not sharing DISPLAY objects within a multi-processing
;; shared-memory environment, this is sufficient
#-(or lispm excl lcl3.0)
(defmacro holding-lock ((locator &optional whostate) &body body)
  locator whostate ;; not used
  `(progn ,@body))

#+(and lispm (not imach))
(defmacro atomic-push (item reference)
  `(sys:without-interrupts (push ,item ,reference)))

#+(and lispm (not imach))
(defmacro atomic-pop (list)
  `(sys:without-interrupts (pop ,list)))

#+imach
(defmacro atomic-push (item reference)
  `(process:atomic-push ,item ,reference))

#+imach
(defmacro atomic-pop (reference)
  `(process:atomic-pop ,reference))

#+lcl3.0
(defmacro atomic-push (item reference)
  `(lcl:with-interrupts-deferred (push ,item ,reference)))

#+lcl3.0
(defmacro atomic-pop (list)
  `(lcl:with-interrupts-deferred (pop ,list)))

;; If you don't have multi-processing or push is atomic, this is sufficient
#-(or lispm lcl3.0)
(defmacro atomic-push (item reference)
  `(push ,item ,reference))

;; If you don't have multi-processing or pop is atomic, this is sufficient
#-(or lispm lcl3.0)
(defmacro atomic-pop (list)
  `(pop ,list))

#+excl
(defvar *inside-event-listen-catch* nil)

#+excl
(defmacro wrap-event-listen (form &body body)
  ;; If we are running a process wait function (in the scheduler stack group)
  ;; and the input lock is held by another process, return nil.
  `(let ((*inside-event-listen-catch* t))
     (unless (eq :would-block (catch 'event-listen ,form))
       . ,body)))

#-excl
(defmacro wrap-event-listen (form &body body)
  `(progn ,form . ,body))

;;;-----------------------------------------------------------------------------
;;; IO Error Recovery
;;;	All I/O operations are done within a WRAP-BUF-OUTPUT macro.
;;;	It prevents multiple mindless errors when the network craters.
;;;
#+comment ;; #+lispm
(defmacro wrap-buf-output (buffer &body body)
  ;; Error recovery wrapper
  `(unless (buffer-dead ,buffer)
     (sys:condition-case ()
	 (progn ,@body)
       (sys:network-error (setf (buffer-dead ,buffer) t)))))

;;#-lispm
(defmacro wrap-buf-output (buffer &body body)
  ;; Error recovery wrapper
  `(unless (buffer-dead ,buffer)
     ,@body))

;;;-----------------------------------------------------------------------------
;;; System dependent IO primitives
;;;	Functions for opening, reading writing forcing-output and closing 
;;;	the stream to the server.
;;;-----------------------------------------------------------------------------

;;; open-x-stream - create a stream for communicating to the appropriate X
;;; server

#-(or explorer genera lucid kcl excl)
(defun open-x-stream (host display protocol)
  host display protocol ;; unused
  (error "OPEN-X-STREAM not implemented yet."))

#+genera
(progn

;;; TCP and DNA are both layered products, so try to work with either one.

(when (fboundp 'tcp:add-tcp-port-for-protocol)
  (tcp:add-tcp-port-for-protocol :x-window-system 6000))

(when (fboundp 'dna:add-dna-contact-id-for-protocol)
  (dna:add-dna-contact-id-for-protocol :x-window-system "X0"))

(net:define-protocol :x-window-system (:x-window-system :byte-stream)
  (:invoke-with-stream ((stream :characters nil :ascii-translation nil))
    stream))
)

#+genera
(defun open-x-stream (host display protocol)
  (setf host (net:parse-host host))
  ;; If PROTOCOL is NIL (the default), we use the generic network system to choose a network
  ;; protocol. Since the GNS has no way to communicate the display number, this only works for
  ;; display 0.  For other displays, we blindly default to TCP.
  ;;
  ;; To take advantage of this, add a service triple to the service host such as:
  ;;   X-WINDOW-SYSTEM TCP X-WINDOW-SYSTEM
  ;; or
  ;;   X-WINDOW-SYSTEM DNA X-WINDOW-SYSTEM
  (when (and (null protocol) (zerop display))
    (return-from open-x-stream
      (let ((neti:*invoke-service-automatic-retry* t))
	(net:invoke-service-on-host :x-window-system host))))
  (ccase protocol
    ((:tcp nil)
     (tcp:open-tcp-stream host (+ *x-tcp-port* display) nil
			  :direction :io
			  :characters nil
			  :ascii-translation nil))
    ((:dna)
     (dna:open-dna-bidirectional-stream host (format nil "X~D" display)
					:characters nil :ascii-translation nil))))

#+explorer
(defun open-x-stream (host display protocol)
  protocol ;; unused
  (ip:open-stream host
		  :remote-port (+ *x-tcp-port* display)
		  :direction :bidirectional
		  :characters t
		  :timeout-after-open nil))

#+lucid
(defun open-x-stream (host display protocol)
  protocol ;; unused
  (let ((fd (connect-to-server host display)))
    (when (minusp fd)
      (error "Failed to connect to server: ~A ~D" host display))
    (user::make-lisp-stream :input-handle fd
			    :output-handle fd
			    :element-type 'unsigned-byte
			    #-lcl3.0 :stream-type #-lcl3.0 :ephemeral)))

#+kcl
(defun open-x-stream (host display protocol)
  protocol ;; unused
  (let ((stream (tcp:open-tcp-stream host display)))
    (if (streamp stream)
	stream
      (error "Cannot connect to server: ~A:~D" host display))))

#+excl
;;
;; Note that since we don't use the CL i/o facilities to do i/o, the display
;; input and output "stream" is really a file descriptor (fixnum).
;;
(defun open-x-stream (host display protocol)
  (declare (ignore protocol));; unused
  (let ((fd (connect-to-server (string host) display)))
    (when (minusp fd)
      (error "Failed to connect to server: ~A ~D" host display))
    fd))

;;; buffer-read-default - read data from the X stream

#+(or genera explorer)
(defun buffer-read-default (display vector start end timeout)
  ;; returns non-NIL if EOF encountered
  ;; Returns :TIMEOUT when timeout exceeded
  (declare (type display display)
	   (type buffer-bytes vector)
	   (type array-index start end)
	   (type (or null number) timeout))
  (declare-buffun)
  (let ((stream (display-input-stream display))
	(eofp nil))
    (when timeout
      (unless (sys:process-wait-with-timeout
		  "X Server"
		  (round (* timeout 60.)) stream :listen)
	(setq eofp :timeout)))
    (unless eofp
      (multiple-value-setq (nil eofp)
	(funcall stream :string-in nil vector start end)))
    eofp))

#+excl
;;
;; This is used so an 'eq' test may be used to find out whether or not we can
;; safely throw this process out of the CLX read loop.
;;
(defparameter *read-whostate* "blocked on read from X server")

#+excl
(defun listen-fd (fd)
  "nil if no bytes available, t on bytes available or error"
  (declare (type fixnum fd))
  (declare-buffun)
  (not (zerop (the fixnum (c-howmany-bytes fd)))))

#+excl
(defmacro with-interrupt-checking-on (&body body)
  `(locally (declare (optimize (safety 1)))
     ,@body))

#+excl
;;
;; Changed 10/10/88 to not wait for all characters to be available before
;; going out to C to do the read.  This is because the X server *does* try
;; to write out the full request at once, but the kernel (on the sun4/4.0, at
;; least) apparently doesn't guarantee writing at least 4096 bytes to the
;; socket. (In my test case it wrote 3987 bytes, a very strange number indeed).
;;
(defun buffer-read-default (display vector start end timeout)
  (declare (type display display)
	   (type buffer-bytes vector)
	   (type array-index start end)
	   (type (or null number) timeout))
  (declare-buffun)
    
  (let* ((howmany (- end start))
	 (fd (display-input-stream display))
	 (numread (c-read-bytes fd vector start howmany)))
    (declare (fixnum howmany fd numread))
	  
    (cond ((minusp numread)
	   t)					;; error
	  ((eql numread howmany)
	   nil)					;; success
	  (t
	   ;; We were able to read some or none of the request.
	   (when (> numread 0)
	     (decf howmany numread)
	     (incf start numread))
	   
	   (cond ((and timeout (zerop timeout))
 	  ;; If there aren't enough and timeout == 0, timeout.
	   :timeout)
	  
		 ;; Otherwise if the scheduler is running let it do timeouts.
	  ((excl::scheduler-running-p)
	   (unwind-protect
		(progn
		  (mp::mpwatchfor fd)
			 (do ()
			     ((zerop howmany) nil)
		  (if (null timeout)
		      (mp::process-wait *read-whostate*
						 #'listen-fd fd)
		    ;; Otherwise we have a timeout to wait for.
		    ;; This doesn't work under 2.0.
		    #+allegro
		    (if (eql (mp::process-wait-with-timeout
			      *read-whostate* timeout
				       #'listen-fd fd)
			     'mp::with-timeout-internal)
			(return-from buffer-read-default :timeout))
		    #-allegro
		    (mp::process-wait *read-whostate*
					       #'listen-fd fd))
			   ;; Now there are more characters available.
			   (setq numread
			     (c-read-bytes fd vector start howmany))
			   (if (minusp numread)
			       (return-from buffer-read-default t))   ;; error
			   (decf howmany numread)
			   (incf start numread)))
		    (mp::mpunwatchfor fd)))
	  
		 ;; Otherwise we have to handle timeouts by hand, and call a
		 ;; special c read function that will return on interrupt.
	  (t
		  (with-interrupt-checking-on 
	   (if (null timeout)
			(do ()
			    ((zerop howmany) nil)
			  (setq numread (c-read-bytes-interruptible
					 fd vector start howmany))
			  (excl:if* (plusp numread)
			     then (decf howmany numread)
				  (incf start numread)
			   elseif (eql numread -1)
			     then (return-from buffer-read-default t))) ; error
		    
		      ;; Otherwise there is a timed timeout. This is a hack. XX
	     (dotimes (i (round timeout) :timeout)
			(declare (fixnum i))
			(if (null (listen-fd fd))
		   (sleep 1)
		 (return-from buffer-read-default
			    (minusp (the fixnum
					 (c-read-bytes fd vector start howmany)
					 )))))))))))))

;;; WARNING:
;;;	CLX performance will suffer if your lisp uses read-byte for
;;;	receiving all data from the X Window System server.
;;;	You are encouraged to write a specialized version of
;;;	buffer-read-default that does block transfers.
#-(or genera explorer excl)
(defmacro CL-read-bytes (stream vector start end)
  `(do* ((i ,start (index+ i 1))
	 (c nil))
	((index>= i ,end) nil)
     (declare (type array-index i)
	      (type (or null card8) c))
     (setq c (read-byte ,stream nil nil))
     (if c
	 (setf (aref ,vector i) c)
	 (return t))))

;; Poll for input every *buffer-read-polling-time* SECONDS.
#-(or genera explorer excl)
(defparameter *buffer-read-polling-time* 0.5)

#-(or genera explorer excl)
(defun buffer-read-default (display vector start end timeout)
  (declare (type display display)
	   (type buffer-bytes vector)
	   (type array-index start end)
	   (type (or null (rational 0 *) (float 0.0 *)) timeout))
  (declare-buffun)
  (let ((stream (display-input-stream display)))
    (declare (type stream stream))
    (cond ((or (null timeout)			; timeout = NIL
	       (listen stream))			; OR input waiting
	   (cl-read-bytes stream vector start end))
	  ((zerop timeout)			; timeout = 0 
	   :timeout)				; no input (we listened above)
	  (t					; timeout > 0, so poll until time is up.
	   (multiple-value-bind (npoll fraction)
	       (truncate timeout *buffer-read-polling-time*)
	     (if (or (listen stream)		; listen first
		     (dotimes (i npoll)		; Sleep for a time, then listen again
		       (sleep *buffer-read-polling-time*)
		       (when (listen stream) (return t)))
		     (when (plusp fraction)
		       (sleep fraction)		; Sleep a fraction of a second
		       (listen stream)))	; and listen one last time
		 (cl-read-bytes stream vector start end)
		 :timeout))))))

;;; buffer-write--default - write data to the X stream

#+(or genera explorer)
(defun buffer-write-default (vector display start end)
  ;; The default buffer write function for use with common-lisp streams
  (declare (type buffer-bytes vector)
	   (type display display)
	   (type array-index start end))
  (declare-buffun)
  (write-string vector (display-output-stream display) :start start :end end))

#+excl
(defun buffer-write-default (vector display start end)
  (declare (type buffer-bytes vector)
	   (type display display)
	   (type array-index start end))
  (declare-buffun)
  (if (minusp (the fixnum (c-write-bytes (display-output-stream display)
					 vector start end)))
      (error "X write failed:  socket dead!")))

;;; WARNING:
;;;	CLX performance will be severely degraded if your lisp uses
;;;	write-byte to send all data to the X Window System server.
;;;	You are STRONGLY encouraged to write a specialized version
;;;	of buffer-write-default that does block transfers.

#-(or genera explorer excl)
(defun buffer-write-default (vector display start end)
  ;; The default buffer write function for use with common-lisp streams
  (declare (type buffer-bytes vector)
	   (type display display)
	   (type array-index start end))
  (declare-buffun)
  (with-vector (vector buffer-bytes)
    (do ((stream (display-output-stream display))
	 (index start (index+ index 1)))
	((index>= index end))
      (declare (type stream stream)
	       (type array-index index))
      (write-byte (aref vector index) stream))))

;;; buffer-force-output-default - force output to the X stream

#+excl
(defun buffer-force-output-default (display)
  ;; The default buffer force-output function for use with common-lisp streams
  (declare (type display display))
  (if (minusp (the fixnum (c-flush-bytes (display-output-stream display))))
      (error "X write failed:  socket dead!")))

#-excl
(defun buffer-force-output-default (display)
  ;; The default buffer force-output function for use with common-lisp streams
  (declare (type display display))
  (force-output (display-output-stream display)))

;;; buffer-close-default - close the X stream


#+excl
(defun buffer-close-default (display &key abort)
  ;; The default buffer close function for use with common-lisp streams
  (declare (type display display))
  (declare-buffun)
  (excl::filesys-checking-close (display-output-stream display)))

#-excl
(defun buffer-close-default (display &key abort)
  ;; The default buffer close function for use with common-lisp streams
  (declare (type display display))
  (declare-buffun)
  (close (display-output-stream display) :abort abort))

;;;-----------------------------------------------------------------------------
;;; System dependent speed hacks
;;;-----------------------------------------------------------------------------

;;
;; WITH-STACK-LIST is used by WITH-STATE as a memory saving feature.
;; If your lisp doesn't have stack-lists, and you're worried about
;; consing garbage, you may want to re-write this to allocate and
;; initialize lists from a resource.
;;
#+lispm
(defmacro with-stack-list ((var &rest elements) &body body)
  `(sys:with-stack-list (,var ,@elements) ,@body))

#+lispm
(defmacro with-stack-list* ((var &rest elements) &body body)
  `(sys:with-stack-list* (,var ,@elements) ,@body))

#-lispm
(defmacro with-stack-list ((var &rest elements) &body body)
  ;; SYNTAX: (WITH-STACK-LIST (var exp1 ... expN) body)
  ;; Equivalent to (LET ((var (MAPCAR #'EVAL '(exp1 ... expN)))) body)
  ;; except that the list produced by MAPCAR resides on the stack and
  ;; therefore DISAPPEARS when WITH-STACK-LIST is exited.
  `(let ((,var (list ,@elements))) ,@body))

#-lispm
(defmacro with-stack-list* ((var &rest elements) &body body)
  ;; SYNTAX: (WITH-STACK-LIST* (var exp1 ... expN) body)
  ;; Equivalent to (LET ((var (APPLY #'LIST* (MAPCAR #'EVAL '(exp1 ... expN))))) body)
  ;; except that the list produced by MAPCAR resides on the stack and
  ;; therefore DISAPPEARS when WITH-STACK-LIST is exited.
  `(let ((,var (list* ,@elements))) ,@body))

(proclaim '(inline buffer-replace))

#+lispm
(defun buffer-replace (buf1 buf2 start1 end1 &optional (start2 0))
  (declare (type vector buf1 buf2)
	   (type array-index start1 end1 start2))
  (sys:copy-array-portion buf2 start2 (length buf2) buf1 start1 end1))

#+excl
(defun buffer-replace (target-sequence source-sequence target-start
				       target-end &optional (source-start 0))
  (declare (type buffer-bytes target-sequence source-sequence)
	   (type array-index target-start target-end source-start)
	   (optimize (speed 3) (safety 0)))
  
  (let ((source-end (length source-sequence)))
    (declare (type array-index source-end))
    
    (excl::if* (and (eq target-sequence source-sequence)
		    (> target-start source-start))
       then (let ((nelts (min (- target-end target-start)
			      (- source-end source-start))))
	      (do ((target-index (+ target-start nelts -1) (1- target-index))
		   (source-index (+ source-start nelts -1) (1- source-index)))
		  ((= target-index (1- target-start)) target-sequence)
		(declare (type array-index target-index source-index))
		
		(setf (aref target-sequence target-index)
		  (aref source-sequence source-index))))
       else (do ((target-index target-start (1+ target-index))
		 (source-index source-start (1+ source-index)))
		((or (= target-index target-end) (= source-index source-end))
		 target-sequence)
	      (declare (type array-index target-index source-index))

	      (setf (aref target-sequence target-index)
		(aref source-sequence source-index))))))

#+(and clx-overlapping-arrays (not (or lispm excl)))
(defun buffer-replace (buf1 buf2 start1 end1 &optional (start2 0))
  (declare (type vector buf1 buf2)
	   (type array-index start1 end1 start2))
  (replace buf1 buf2 :start1 start1 :end1 end1 :start2 start2))

#-(or lispm excl clx-overlapping-arrays)
(defun buffer-replace (buf1 buf2 start1 end1 &optional (start2 0))
  (declare (type buffer-bytes buf1 buf2)
	   (type array-index start1 end1 start2))
  (replace buf1 buf2 :start1 start1 :end1 end1 :start2 start2))

#+ti
(defun with-location-bindings (sys:&quote bindings &rest body)
  (do ((bindings bindings (cdr bindings)))
      ((null bindings)
       (sys:eval-body-as-progn body))
    (sys:bind (sys:*eval `(sys:locf ,(caar bindings)))
	      (sys:*eval (cadar bindings)))))

#+ti
(compiler:defoptimizer with-location-bindings with-l-b-compiler nil (form)
  (let ((bindings (cadr form))
	(body (cddr form)))
    `(let ()
       ,@(loop for (accessor value) in bindings
	       collect `(si:bind (si:locf ,accessor) ,value))
       ,@body)))

#+(and lispm (not ti))
(defmacro with-location-bindings (bindings &body body)
  `(sys:letf* ,bindings ,@body))

#+lispm
(defmacro with-gcontext-bindings ((gc saved-state indexes ts-index temp-mask temp-gc)
				  &body body)
  ;; don't use svref on LHS because Symbolics didn't define locf for it
  (let* ((local-state (gensym))
	 (bindings `(((aref ,local-state ,ts-index) 0))))	; will become zero anyway
    (dolist (index indexes)
      (push `((aref ,local-state ,index) (svref ,saved-state ,index))
	    bindings))
    `(let ((,local-state (gcontext-local-state ,gc)))
       (declare (type gcontext-state ,local-state))
       (unwind-protect
	   (with-location-bindings ,bindings
	     ,@body)
	 (setf (svref ,local-state ,ts-index) 0)
	 (when ,temp-gc
	   (restore-gcontext-temp-state ,gc ,temp-mask ,temp-gc))
	 (deallocate-gcontext-state ,saved-state)))))

#-lispm
(defmacro with-gcontext-bindings ((gc saved-state indexes ts-index temp-mask temp-gc)
				  &body body)
  (let ((local-state (gensym))
	(resets nil))
    (dolist (index indexes)
      (push `(setf (svref ,local-state ,index) (svref ,saved-state ,index))
	    resets))
    `(unwind-protect
	 (progn
	   ,@body)
       (let ((,local-state (gcontext-local-state ,gc)))
	 (declare (type gcontext-state ,local-state))
	 ,@resets
	 (setf (svref ,local-state ,ts-index) 0))
       (when ,temp-gc
	 (restore-gcontext-temp-state ,gc ,temp-mask ,temp-gc))
       (deallocate-gcontext-state ,saved-state))))

;;; -----------------------------------------------------------------------------
;;; How error detection should CLX do?
;;; Several levels are possible:
;;;
;;; 1. Do the equivalent of check-type on every argument.
;;; 
;;; 2. Simply report TYPE-ERROR.  This eliminates overhead of all the format
;;;    strings generated by check-type.
;;; 
;;; 3. Do error checking only on arguments that are likely to have errors
;;;    (like keyword names)
;;; 
;;; 4. Do error checking only where not doing so may dammage the envirnment
;;;    on a non-tagged machine (i.e. when storing into a structure that has
;;;    been passed in)
;;; 
;;; 5. No extra error detection code.  On lispm's, ASET may barf trying to
;;;    store a non-integer into a number array. 
;;; 
;;; How extensive should the error checking be?  For example, if the server
;;; expects a CARD16, is is sufficient for CLX to check for integer, or
;;; should it also check for non-negative and less than 65536?
;;;-----------------------------------------------------------------------------
 
;; The *TYPE-CHECK?* constant controls how much error checking is done.
;; Possible values are:
;;    NIL      - Don't do any error checking
;;    t        - Do the equivalent of checktype on every argument
;;    :minimal - Do error checking only where errors are likely

;;; This controls macro expansion, and isn't changable at run-time You will
;;; probably want to set this to nil if you want good performance at
;;; production time.
(defconstant *type-check?* t)

;; TYPE? is used to allow the code to do error checking at a different level from
;; the declarations.  It also does some optimizations for systems that don't have
;; good compiler support for TYPEP.  The definitions for CARD32, CARD16, INT16, etc.
;; include range checks.  You can modify TYPE? to do less extensive checking
;; for these types if you desire.

(defmacro type? (object type)
  (if (not (constantp type))
      `(typep ,object ,type)
    (progn
      (setq type (eval type))
      #+explorer
      (if *type-check?*
	  `(locally (declare (optimize safety)) (typep ,object ',type))
	`(typep ,object ',type))
      #-explorer
      (let ((predicate (assoc type
			      '((drawable drawable-p) (window window-p) (pixmap pixmap-p)
				(cursor cursor-p) (font font-p)
				(gcontext gcontext-p) (colormap colormap-p)
				(null null) (integer integerp)))))
	(if predicate
	    `(,(second predicate) ,object)
	  (if *type-check?*
	      `(locally (declare (optimize safety)) (typep ,object ',type))
	    `(typep ,object ',type)))))))

;; X-TYPE-ERROR is the function called for type errors.
;; If you want lots of checking, but are concerned about code size,
;; this can be made into a macro that ignores some parameters.

(defun x-type-error (object type &optional error-string)
  (x-error 'type-error :object object :type type :type-string error-string))

;;-------------------------------------------------------------------------------
;; Error handlers
;;    Hack up KMP error signaling using zetalisp until the real thing comes along
;;-------------------------------------------------------------------------------

(defun default-error-handler (display error-key &rest key-vals)
  ; The default display-error-handler.
  ; It signals the conditions listed in the DISPLAY file
  display
  (apply 'x-error error-key :display display :error-key error-key key-vals))

#+lispm
(defun x-error (condition &rest keyargs)
  (apply #'sys:signal condition keyargs))

#+lispm
(defun x-cerror (proceed-format-string condition &rest keyargs)
  (sys:signal (apply #'zl:make-condition condition keyargs)
	      :proceed-types proceed-format-string))

#-lispm
(defun x-error (condition &rest keyargs)
  (error "X-Error: ~a"
	 (princ-to-string (apply #'make-condition condition keyargs))))

#-lispm
(defun x-cerror (proceed-format-string condition &rest keyargs)
  (cerror proceed-format-string "X-Error: ~a"
	 (princ-to-string (apply #'make-condition condition keyargs))))

;; version 15 of Pitman error handling defines the syntax for define-condition to be:
;; DEFINE-CONDITION name (parent-type) [({slot}*) {option}*]
;; Where option is one of: (:documentation doc-string) (:conc-name symbol-or-string)
;; or (:report exp)

#+lispm
(defmacro define-condition (name parents &body options)
  (let ((slots (pop options))
	(documentation nil)
	(conc-name (concatenate 'string (string name) "-"))	       
	(reporter nil))
    (dolist (item options)
      (ecase (first item)
	(:documentation (setq documentation (second item)))
	(:conc-name (setq conc-name (string (second item))))
	(:report (setq reporter (second item)))))
    `(within-definition (,name define-condition)
       (zl:defflavor ,name ,slots ,parents
	 :initable-instance-variables
	 #-genera
	 (:accessor-prefix ,conc-name)
	 #+genera
	 (:conc-name ,conc-name)
	 #-genera
	 (:outside-accessible-instance-variables ,@slots)
	 #+genera
	 (:readable-instance-variables ,@slots))
       ,(when reporter ;; when no reporter, parent's is inherited
	  `(zl:defmethod #-genera (,name :report)
	                 #+genera (:report ,name) (stream)
	      ,(if (stringp reporter)
		   `(write-string ,reporter stream)
		 `(,reporter global:self stream))
	      global:self))
       ,(when documentation
	  `(setf (documentation name 'type) ,documentation))
       ',name)))

#+lispm
(zl:defflavor x-error () (global:error))

#-lispm
(defstruct x-error
  report-function)

#-lispm
(defun reporter-for-condition (name)
  (xintern "." name '-reporter.))

#-lispm
(defmacro define-condition (name parents &body options)
  ;; Define a structure that when printed displays an error message
  (let ((slots (pop options))
	(documentation nil)
	(conc-name (concatenate 'string (string name) "-"))	       
	(reporter nil)
	(condition (gensym))
	(stream (gensym))
	(report-function (reporter-for-condition name)))
    (dolist (item options)
      (ecase (first item)
	(:documentation (setq documentation (second item)))
	(:conc-name (setq conc-name (string (second item))))
	(:report (setq reporter (second item)))))
    (unless reporter (setq report-function (reporter-for-condition (car parents))))
    `(within-definition (,name define-condition)
       (defstruct (,name (:conc-name ,(intern conc-name))
		         (:print-function condition-print)
			 (:include ,(car parents) (report-function ',report-function)))
	 ,@slots)
       ,(when documentation
	  `(setf (documentation name 'type) ,documentation))
       ,(when reporter
	  `(defun ,report-function (,condition ,stream)
	     ,(if (stringp reporter)
		  `(write-string ,reporter ,stream)
		`(,reporter ,condition ,stream))
	     ,condition))
       ',name)))

#-lispm
(defun condition-print (condition stream depth)
  (declare (type x-error condition)
	   (type stream stream)
	   (ignore depth))
  (if *print-escape*
      (format stream "#<~a>" (type-of condition))
    (funcall (x-error-report-function condition) condition stream))
  condition)
  
#-lispm
(defun make-condition (type &rest slot-initializations)
  (let ((make-function (intern (concatenate 'string (string 'make-) (string type))
			       (symbol-package type))))
    (apply make-function slot-initializations)))

#-(or explorer genera)
(defun host-address (host &optional (family :internet))
  ;; Return a list whose car is the family keyword (:internet :DECnet :Chaos)
  ;; and cdr is a list of network address bytes.
  (declare (type (or stringable list) host)
	   (type (or null (member :internet :decnet :chaos) card8) family))
  (declare-values list)
  host family
  (error "HOST-ADDRESS not implemented yet."))

#+explorer
(defun host-address (host &optional (family :internet))
  ;; Return a list whose car is the family keyword (:internet :DECnet :Chaos)
  ;; and cdr is a list of network address bytes.
  (declare (type (or stringable list) host)
	   (type (or null (member :internet :decnet :chaos) card8) family))
  (declare-values list)
  (ecase family
    (:internet
     (let ((addr (ip:get-ip-address host)))
       (unless addr (error "~s isn't an internet host name" host))
       (list :internet
	     (ldb (byte 8 24) addr)
	     (ldb (byte 8 16) addr)
	     (ldb (byte 8 8) addr)
	     (ldb (byte 8 0) addr))))
    (:chaos
     (let ((addr (first (chaos:chaos-addresses host))))
       (unless addr (error "~s isn't a chaos host name" host))
       (list :chaos
	     (ldb (byte 8 0) addr)
	     (ldb (byte 8 8) addr))))))

#+genera
(defun host-address (host &optional (family :internet))
  ;; Return a list whose car is the family keyword (:internet :DECnet :Chaos)
  ;; and cdr is a list of network address bytes.
  (declare (type (or stringable list) host)
	   (type (or null (member :internet :decnet :chaos) card8) family))
  (declare-values list)
  (let ((net-type (if (eq family :DECnet)
		      :DNA
		      family)))
    (dolist (addr
	      (sys:send (net:parse-host host) :network-addresses)
	      (error "~s isn't a valid ~(~A~) host name" host family))
      (let ((network (car addr))
	    (address (cadr addr)))
	(when (sys:send network :network-typep net-type)
	  (return (ecase family
		    (:internet
		      (multiple-value-bind (a b c d) (tcp:explode-internet-address address)
			(list :internet a b c d)))
		    ((:chaos :DECnet)
		     (list family (ldb (byte 8 0) address) (ldb (byte 8 8) address))))))))))

#+explorer ;; This isn't required, but it helps make sense of the results from access-hosts
(defun get-host (host-object)
  ;; host-object is a list whose car is the family keyword (:internet :DECnet :Chaos)
  ;; and cdr is a list of network address bytes.
  (declare (type list host-object))
  (declare-values string family)
  (let* ((family (first host-object))
	 (address (ecase family
		    (:internet
		     (dpb (second host-object)
			  (byte 8 24)
			  (dpb (third host-object)
			       (byte 8 16)
			       (dpb (fourth host-object)
				    (byte 8 8)
				    (fifth host-object)))))
		    (:chaos
		     (dpb (third host-object) (byte 8 8) (second host-object))))))
    (when (eq family :internet) (setq family :ip))
    (let ((host (si:get-host-from-address address family)))
      (values (and host (funcall host :name)) family))))

;;; This isn't required, but it helps make sense of the results from access-hosts
#+genera
(defun get-host (host-object)
  ;; host-object is a list whose car is the family keyword (:internet :DECnet :Chaos)
  ;; and cdr is a list of network address bytes.
  (declare (type list host-object))
  (declare-values string family)
  (let ((family (first host-object)))
    (values (sys:send (net:get-host-from-address 
			(ecase family
			  (:internet
			    (apply #'tcp:build-internet-address (rest host-object)))
			  ((:chaos :DECnet)
			   (dpb (third host-object) (byte 8 8) (second host-object))))
			(net:local-network-of-type (if (eq family :DECnet)
						       :DNA
						       family)))
		      :name)
	    family)))

;;; Printing routines.

#-lispm
(defun display-print (display stream depth)
  depth ;; not used
  (format stream "#<DISPLAY ~a ~d>"
	  (display-host display)
	  (display-display display)))

#+lispm
(defun display-print (display stream depth)
  depth ;; not used
  (si:printing-random-object (display stream :typep)
    (princ (display-host display) stream)
    (princ " " stream)
    (princ (display-display display) stream)))
