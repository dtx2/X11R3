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

;;;-------------------------------------------------------------------------
;;; CLX can maintain a mapping from X server ID's to local data types.  If
;;; one takes the view that CLX objects will be instance variables of
;;; objects at the next higher level, then PROCESS-EVENT will typically map
;;; from resource-id to higher-level object.  In that case, the lower-level
;;; CLX mapping will almost never be used (except in rare cases like
;;; query-tree), and only serve to consume space (which is difficult to
;;; GC), in which case always-consing versions of the make-<mumble>s will
;;; be better.  Even when maps are maintained, it isn't clear they are
;;; useful for much beyond xatoms and windows (since almost nothing else
;;; ever comes back in events).
;;;--------------------------------------------------------------------------
(defconstant *clx-cached-types*
	     '( drawable
		window
		pixmap
;		gcontext
		cursor
		colormap
		font
		xatom))

(defmacro resource-id-map-test ()
  #+excl '#'equal
  #-excl (if #.(> #x1fffffff most-positive-fixnum) '#'eql '#'eq))

;;; If you use overlapping-arrays, you must define this to match the
;;; real byte order (which probably means uncommenting it).  Otherwise,
;;; you can choose the byte order to match the byte order of the servers
;;; you talk to most frequently.

#+(or explorer genera)
(eval-when (eval compile load)
  (pushnew :clx-little-endian *features*))

;;; Steele's Common-Lisp states:  "It is an error if the array specified
;;; as the :displaced-to argument  does not have the same :element-type
;;; as the array being created" If this is the case on your lisp, then
;;; leave the overlapping-arrays feature turned off.  Lisp machines
;;; (Symbolics TI and LMI) don't have this restriction, and allow arrays
;;; with different element types to overlap.  CLX will take advantage of
;;; this to do fast array packing/unpacking when the overlapping-arrays
;;; feature is enabled.

#+(and clx-little-endian lispm)
(eval-when (eval compile load)
  (pushnew :clx-overlapping-arrays *features*))

#+(and clx-overlapping-arrays genera)
(progn
(deftype overlap16 () '(unsigned-byte 16))
(deftype overlap32 () '(signed-byte 32))
)

#+(and clx-overlapping-arrays (or explorer lambda cadr))
(progn
(deftype overlap16 () '(unsigned-byte 16))
(deftype overlap32 () '(unsigned-byte 32))
)

(deftype buffer-bytes () `(simple-array (unsigned-byte 8) (*)))

#+clx-overlapping-arrays
(progn
(deftype buffer-words () `(vector overlap16))
(deftype buffer-longs () `(vector overlap32))
)

;;; This defines a type which is a subtype of the integers.
;;; This type is used to describe all variables that can be array indices.
;;; It is here because it is used below.
;;; This is inclusive because start/end can be 1 past the end.
(deftype array-index () `(integer 0 ,array-dimension-limit))

;; this is the best place to define these?

(defun make-index-typed (form)
  (if (integerp form)
      form
      `(the array-index ,form)))

(defmacro index+ (&rest numbers)
  `(the array-index (+ ,@(mapcar #'make-index-typed numbers))))
(defmacro index-logand (&rest numbers)
  `(the array-index (logand ,@(mapcar #'make-index-typed numbers))))
(defmacro index-logior (&rest numbers)
  `(the array-index (logior ,@(mapcar #'make-index-typed numbers))))
(defmacro index- (&rest numbers)
  `(the array-index (- ,@(mapcar #'make-index-typed numbers))))
(defmacro index* (&rest numbers)
  `(the array-index (* ,@(mapcar #'make-index-typed numbers))))

(defmacro index1+ (number)
  `(the array-index (1+ (the array-index ,number))))
(defmacro index1- (number)
  `(the array-index (1- (the array-index ,number))))

;;; CLtL Page 96 -Slyme loses
(defmacro index-incf (place &optional (delta 1))
  #+genera `(setf ,place (index+ ,place ,delta))
  #-genera `(incf (the array-index ,place) (the array-index ,delta)))
(defmacro index-decf (place &optional (delta 1))
  #+genera `(setf ,place (index- ,place ,delta))
  #-genera `(decf (the array-index ,place) (the array-index ,delta)))

(defmacro index-min (&rest numbers)
  `(the array-index (min ,@(mapcar #'make-index-typed numbers))))
(defmacro index-max (&rest numbers)
  `(the array-index (max ,@(mapcar #'make-index-typed numbers))))

(defmacro index-floor (number &optional divisor)
  `(the array-index
	(values (floor (the array-index ,number)
		       ,@(when divisor `((the array-index ,divisor)))))))
(defmacro index-ceiling (number &optional divisor)
  `(the array-index
	(values (ceiling (the array-index ,number)
			 ,@(when divisor `((the array-index ,divisor)))))))
(defmacro index-truncate (number &optional divisor)
  `(the array-index
	(values (truncate (the array-index ,number)
			  ,@(when divisor `((the array-index ,divisor)))))))
(defmacro index-mod (number divisor)
  `(the array-index
	(mod (the array-index ,number) (the array-index ,divisor))))

(defmacro index-ash (number count)
  `(the array-index
	(ash (the array-index ,number) (the fixnum ,count))))

(defmacro index-plusp (number)
  `(plusp (the array-index ,number)))
(defmacro index-zerop (number)
  `(zerop (the array-index ,number)))

(defmacro index> (&rest numbers)
  `(> ,@(mapcar #'make-index-typed numbers)))
(defmacro index= (&rest numbers)
  `(= ,@(mapcar #'make-index-typed numbers)))
(defmacro index< (&rest numbers)
  `(< ,@(mapcar #'make-index-typed numbers)))
(defmacro index>= (&rest numbers)
  `(>= ,@(mapcar #'make-index-typed numbers)))
(defmacro index<= (&rest numbers)
  `(<= ,@(mapcar #'make-index-typed numbers)))

#-lispm
(proclaim '(declaration arglist values))

#+lispm
(defmacro declare-arglist (&rest args)
  `(declare (sys:arglist ,@args)))

#-lispm
(defmacro declare-arglist (&rest args)
  `(declare (arglist ,@args)))

#+lispm
(defmacro declare-values (&rest vals)
  `(declare (sys:values ,@vals)))

#-lispm
(defmacro declare-values (&rest vals)
  `(declare (values ,@vals)))

#+genera
(defmacro declare-array (type &rest vars)
  `(declare (type ,type ,@vars)
	    (sys:array-register ,@vars)))

#-genera
(defmacro declare-array (type &rest vars)
  `(declare (type ,type ,@vars)))

#+lispm
(defmacro declare-funarg (type &rest vars)
  `(declare (type ,type ,@vars)
	    (sys:downward-funarg ,@vars)))

#-lispm
(defmacro declare-funarg (type &rest vars)
  `(declare (type ,type ,@vars)))

#+genera
(defmacro with-vector ((var type) &body body)
  `(let ((,var ,var))
     (declare-array ,type ,var)
     ,@body))

#-genera
(defmacro with-vector ((var type) &body body)
  (declare (ignore var type))
  `(progn ,@body))

#+genera
(defmacro within-definition ((name type) &body body)
  `(sys:local-declare
     ((sys:function-parent ,name ,type))
     (sys:record-source-file-name ',name ',type)
     ,@body))

#+explorer
(defmacro within-definition ((name type) &body body)
  `(zl:local-declare
     ((sys:function-parent ,name ,type))
     (sys:record-source-file-name ',name ',type)
     ,@body))

#-(or genera explorer)
(defmacro within-definition ((name type) &body body)
  (declare (ignore name type))
  `(progn ,@body))

(defconstant *replysize* 32.)

;; used in defstruct initializations to avoid compiler warnings
(defvar *empty-bytes* (make-sequence 'buffer-bytes 0))
(proclaim '(type buffer-bytes *empty-bytes*))
#+clx-overlapping-arrays
(progn
(defvar *empty-words* (make-sequence 'buffer-words 0))
(proclaim '(type buffer-words *empty-words*))
)
#+clx-overlapping-arrays
(progn
(defvar *empty-longs* (make-sequence 'buffer-longs 0))
(proclaim '(type buffer-longs *empty-longs*))
)

;; We need this here so we can define BUFFER below.
;;
(defstruct (reply-buffer (:conc-name reply-) (:constructor make-reply-buffer-internal))
  (size 0 :type array-index)			;Buffer size
  ;; Byte (8 bit) input buffer
  (ibuf8 *empty-bytes* :type buffer-bytes)
  ;; Word (16bit) input buffer
  #+clx-overlapping-arrays
  (ibuf16 *empty-words* :type buffer-words)
  ;; Long (32bit) input buffer
  #+clx-overlapping-arrays
  (ibuf32 *empty-longs* :type buffer-longs)
  )

(defconstant *buffer-text16-size* 256)
(deftype buffer-text16 () `(simple-array (unsigned-byte 16) (,*buffer-text16-size*)))

;; We need this here so we can define DISPLAY for CLX.
;;
;; This structure is :INCLUDEd in the DISPLAY structure.
;; Overlapping (displaced) arrays are provided for byte
;; half-word and word access on both input and output.
;;
(defstruct (buffer (:constructor nil)
		   (:copier nil))
  ;; Lock for multi-processing systems
  (lock (make-process-lock))
#-excl
  (output-stream nil :type (or null stream))
#+excl
  (output-stream -1 :type fixnum)
  ;; Buffer size
  (size 0 :type array-index)
  ;; Buffer size minus request size
  (limit 0 :type array-index)
  (request-number 0 :type integer)
  ;; Byte position of start of last request
  ;; used for appending requests and error recovery
  (last-request nil :type (or null array-index))
  ;; Byte position of start of last flushed request
  (last-flushed-request nil :type (or null array-index))
  ;; Current byte offset
  (boffset 0 :type array-index)
  ;; Byte (8 bit) output buffer
  (obuf8 *empty-bytes* :type buffer-bytes)
  ;; Word (16bit) output buffer
  #+clx-overlapping-arrays
  (obuf16 *empty-words* :type buffer-words)
  ;; Long (32bit) output buffer
  #+clx-overlapping-arrays
  (obuf32 *empty-longs* :type buffer-longs)
  ;; Holding buffer for 16-bit text
  (tbuf16 (make-sequence 'buffer-text16 *buffer-text16-size* :initial-element 0))
  ;; Probably EQ to Output-Stream
#-excl
  (input-stream nil :type (or null stream))
#+excl
  (input-stream -1 :type fixnum)
  ;; Buffer for replies
  (reply-buffer nil :type (or null reply-buffer))
  ;; T when the host connection has gotten errors
  (dead nil :type (or null (not null)))
  
  ;; Change these functions when using shared memory buffers to the server
  ;; Function to call when writing the buffer
  (write-function 'buffer-write-default)
  ;; Function to call when flushing the buffer
  (force-output-function 'buffer-force-output-default)
  ;; Function to call when closing a connection
  (close-function 'buffer-close-default)
  ;; Function to call when reading the buffer
  (input-function 'buffer-read-default)
  )

;; These are here because.

(defparameter *xlib-package* (find-package (string 'xlib)))

(defun xintern (&rest parts)
  (intern (apply #'concatenate 'string (mapcar #'string parts)) *xlib-package*))

(defparameter *keyword-package* (find-package (string 'keyword)))

(defun kintern (name)
  (intern (string name) *keyword-package*))
