;;; -*- Mode: LISP; Syntax: Common-lisp; Package: XLIB; Base: 10; Lowercase: Yes -*-

;;; This file contains definitions for the BUFFER object for Common-Lisp X
;;; windows version 11

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

;; A few notes:
;;
;;  1. The BUFFER implements a two-way buffered byte / half-word
;;     / word stream.  Hooks are left for implementing this with a
;;     shared memory buffer, or with effenciency hooks to the network
;;     code.
;;
;;  2. The BUFFER object uses overlapping displaced arrays for
;;     inserting and removing bytes half-words and words.
;;
;;  3. The BYTE component of these arrays is written to a STREAM
;;     associated with the BUFFER.  The stream has its own buffer.
;;     This may be made more efficient by using the Zetalisp
;;     :Send-Output-Buffer operation.
;;
;;  4. The BUFFER object is INCLUDED in the DISPLAY object.
;;     This was done to reduce access time when sending requests,
;;     while maintaing some code modularity.
;;     Several buffer functions are duplicated (with-buffer,
;;     buffer-force-output, close-buffer) to keep the naming
;;     conventions consistent.
;;
;;  5. A nother layer of software is built on top of this for generating
;;     both client and server interface routines, given a specification
;;     of the protocol. (see the INTERFACE file)
;;
;;  6. Care is taken to leave the buffer pointer (buffer-bbuf) set to
;;     a point after a complete request.  This is to ensure that a partial
;;     request won't be left after aborts (e.g. control-abort on a lispm).

(in-package 'xlib :use '(lisp))

(defparameter *requestsize* 160) ;; Max request size (excluding variable length requests)

(eval-when (eval compile load)

;;; This is here instead of in bufmac so that with-display can be
;;; compiled without macros and bufmac being loaded.

(defmacro with-buffer ((buffer) &body body)
  ;; This macro is for use in a multi-process environment.  It provides
  ;; exclusive access to the local buffer object for request generation and
  ;; reply processing.
  (declare (special *within-with-buffer*))
  (if (and (boundp '*within-with-buffer*) *within-with-buffer*)
      `(progn ,buffer ,@body) ;; Speedup hack for lexically nested with-buffer's
    `(compiler-let ((*within-with-buffer* t))
       (let ()
	 (declare-bufmac)
	 (holding-lock ((buffer-lock ,buffer) "Display-Lock") ,@body)))))

;;; The following are here instead of in bufmac so that event-case can
;;; be compiled without macros and bufmac being loaded.

(defmacro read-card8 (byte-index)
  `(aref-card8 buffer-bbuf (index+ buffer-boffset ,byte-index)))

(defmacro read-int8 (byte-index)
  `(aref-int8 buffer-bbuf (index+ buffer-boffset ,byte-index)))

(defmacro read-card16 (byte-index)
  #+clx-overlapping-arrays
  `(aref-card16 buffer-wbuf (index+ buffer-woffset (index-ash ,byte-index -1)))
  #-clx-overlapping-arrays
  `(aref-card16 buffer-bbuf (index+ buffer-boffset ,byte-index)))

(defmacro read-int16 (byte-index)
  #+clx-overlapping-arrays
  `(aref-int16 buffer-wbuf (index+ buffer-woffset (index-ash ,byte-index -1)))
  #-clx-overlapping-arrays
  `(aref-int16 buffer-bbuf (index+ buffer-boffset ,byte-index)))

(defmacro read-card32 (byte-index)
  #+clx-overlapping-arrays
  `(aref-card32 buffer-lbuf (index+ buffer-loffset (index-ash ,byte-index -2)))
  #-clx-overlapping-arrays
  `(aref-card32 buffer-bbuf (index+ buffer-boffset ,byte-index)))

(defmacro read-int32 (byte-index)
  #+clx-overlapping-arrays
  `(aref-int32 buffer-lbuf (index+ buffer-loffset (index-ash ,byte-index -2)))
  #-clx-overlapping-arrays
  `(aref-int32 buffer-bbuf (index+ buffer-boffset ,byte-index)))

(defmacro read-card29 (byte-index)
  #+clx-overlapping-arrays
  `(aref-card29 buffer-lbuf (index+ buffer-loffset (index-ash ,byte-index -2)))
  #-clx-overlapping-arrays
  `(aref-card29 buffer-bbuf (index+ buffer-boffset ,byte-index)))

(defmacro event-code (reply-buffer)
  ;; The reply-buffer structure is used for events.
  ;; The size slot is used for the event code.
  `(reply-size ,reply-buffer))

(defmacro reading-event ((event &rest options) &body body)
  (declare-arglist (buffer &key sizes) &body body)
  ;; BODY may contain calls to (READ32 &optional index) etc.
  ;; These calls will read from the input buffer at byte
  ;; offset INDEX.  If INDEX is not supplied, then the next
  ;; word, half-word or byte is returned.
  (let ((reply-buffer (gensym)))
    `(let ((,reply-buffer ,event))
       (with-buffer-input (,reply-buffer ,@options) ,@body))))

(defmacro with-buffer-input ((buffer &key (sizes '(8 16 32)) index) &body body)
  (unless (listp sizes) (setq sizes (list sizes)))
  ;; 160 is a special hack for client-message-events
  (when (set-difference sizes '(0 8 16 32 160 256))
    (error "Illegal sizes in ~a" sizes))
  `(let ()
     (declare-bufmac)
     (let* ((buffer-boffset (the array-index ,(or index 0)))
	    ,@(when (or #-clx-overlapping-arrays t (member 8 sizes))
		`((buffer-bbuf (reply-ibuf8 ,buffer))))
	    #+clx-overlapping-arrays
	    ,@(when (or (member 16 sizes) (member 160 sizes))
		`((buffer-woffset (index-ash buffer-boffset -1))
		  (buffer-wbuf (reply-ibuf16 ,buffer))))
	    #+clx-overlapping-arrays
	    ,@(when (member 32 sizes)
		`((buffer-loffset (index-ash buffer-boffset -2))
		  (buffer-lbuf (reply-ibuf32 ,buffer)))))
       ,@(when (or #-clx-overlapping-arrays t (member '8 sizes))
	   '((declare-array buffer-bytes buffer-bbuf)))
       #+clx-overlapping-arrays
       ,@(when (member '16 sizes)
	   '((declare-array buffer-words buffer-wbuf)))
       #+clx-overlapping-arrays
       ,@(when (member '32 sizes)
	   '((declare-array buffer-longs buffer-lbuf)))
       buffer-boffset
       ,@(when (or #-clx-overlapping-arrays t (member 8 sizes)) '(buffer-bbuf))
       #+clx-overlapping-arrays
       ,@(when (member 16 sizes) '(buffer-woffset buffer-wbuf))
       #+clx-overlapping-arrays
       ,@(when (member 32 sizes) '(buffer-loffset buffer-lbuf))
       ,@body)))

)

(defun make-buffer (input-size output-size constructor &rest options)
  ;; Input-size is the reply-buffer size in bytes,
  ;; Output-Size is the output-buffer size in bytes.
  (let ((byte-output (make-array output-size :element-type 'card8
				 :initial-element 0)))
    (apply constructor
	   :limit (index- output-size *requestsize*)
	   :size output-size
	   :obuf8 byte-output
	   #+clx-overlapping-arrays
	   :obuf16
	   #+clx-overlapping-arrays
	   (make-array (index-ash output-size -1)
		       :element-type 'overlap16
		       :displaced-to byte-output)
	   #+clx-overlapping-arrays
	   :obuf32
	   #+clx-overlapping-arrays
	   (make-array (index-ash output-size -2)
		       :element-type 'overlap32
		       :displaced-to byte-output)
	   :reply-buffer (make-reply-buffer input-size)
	   options)))

(defun make-reply-buffer (size)
  ;; Size is the buffer size in bytes
  (let ((byte-input (make-array size :element-type 'card8
				:initial-element 0)))
    (make-reply-buffer-internal
      :size size
      :ibuf8 byte-input
      #+clx-overlapping-arrays
      :ibuf16
      #+clx-overlapping-arrays
      (make-array (index-ash size -1)
		  :element-type 'overlap16
		  :displaced-to byte-input)
      #+clx-overlapping-arrays
      :ibuf32
      #+clx-overlapping-arrays
      (make-array (index-ash size -2)
		  :element-type 'overlap32
		  :displaced-to byte-input))))
;;
;; Buffer stream operations
;;

(defun buffer-write (vector buffer start end)
  ;; Write out VECTOR from START to END into BUFFER
  ;; Internal function, MUST BE CALLED FROM WITHIN WITH-BUFFER
  (declare (type buffer buffer)
	   (type array-index start end))
  (when (buffer-dead buffer)
    (x-error 'closed-display :display buffer))
  (wrap-buf-output buffer
    (funcall (buffer-write-function buffer) vector buffer start end))
  nil)

(defun buffer-flush (buffer)
  ;; Write the buffer contents to the server stream - doesn't force-output the stream
  ;; Internal function, MUST BE CALLED FROM WITHIN WITH-BUFFER
  (declare (type buffer buffer))
  (let ((boffset (buffer-boffset buffer)))
    (declare (type array-index boffset))
    (when (index-plusp boffset)
      (buffer-write (buffer-obuf8 buffer) buffer 0 boffset)
      (setf (buffer-boffset buffer) 0)
      (setf (buffer-last-request buffer) nil)))
  nil)

(defun buffer-force-output (buffer)
  ;; Output is normally buffered, this forces any buffered output to the server.
  (declare (type buffer buffer))
  (when (buffer-dead buffer)
    (x-error 'closed-display :display buffer))
  (buffer-flush buffer)
  (wrap-buf-output buffer
    (funcall (buffer-force-output-function buffer) buffer))
  nil)

(defun close-buffer (buffer)
  ;; Close the host connection in BUFFER
  (declare (type buffer buffer))
  (unless (null (buffer-output-stream buffer))
    (wrap-buf-output buffer
      (funcall (buffer-close-function buffer) buffer))
    (setf (buffer-dead buffer) t)
    ;; Zap pointers to the streams, to ensure they're GC'd
    (setf (buffer-output-stream buffer) nil)
    (setf (buffer-input-stream buffer) nil)
    )
  nil)

(defun buffer-input  (buffer vector start end &optional timeout)
  ;; Read into VECTOR from the buffer stream
  ;; Timeout, when non-nil, is in seconds
  ;; Returns non-nil if EOF encountered
  ;; Returns :TIMEOUT when timeout exceeded
  (declare (type buffer buffer)
	   (type vector vector)
	   (type array-index start end)
	   (type (or null number) timeout))
  (declare-values eof-p)
  (when (buffer-dead buffer)
    (x-error 'closed-display :display buffer))
  (unless (= start end)
    (funcall (buffer-input-function buffer) buffer vector start end timeout)))

;;; Reading sequences of strings

;;; a list of pascal-strings with card8 lengths, no padding in between
;;; can't use read-sequence-char
(defun read-sequence-string (buffer length nitems result-type)
  (declare (type buffer buffer)
	   (type array-index length nitems))
  (let ((result (make-sequence result-type nitems)))
    (reading-buffer-reply (buffer :sizes 8)
      (do* ((string-index 0)
	    (string-left-to-read 0)
	    (string "")
	    (sequence-index 0)
	    (size (reply-size (buffer-reply-buffer buffer)))
	    (len length (index- len chunk))
	    (chunk (index-min size len) (index-min size len)))
	   ((index-zerop len) result)
	(declare (type array-index string-index string-left-to-read sequence-index
		       size len chunk)
		 (type simple-string string))
	(buffer-input buffer buffer-bbuf 0 (lround chunk))
	(do ((buffer-index 0 (index+ buffer-index 1))
	     (card8 0))
	    ((index>= buffer-index chunk))
	  (declare (type array-index buffer-index)
		   (type card8 card8))
	  (setq card8 (read-card8 buffer-index))
	  (if (index-zerop string-left-to-read)
	      (when (index< sequence-index nitems)
		(setq string-left-to-read card8)
		(setq string (make-string string-left-to-read))
		(setq string-index 0)
		(setf (elt result sequence-index) string)
		(setq sequence-index (index+ sequence-index 1)))
	    (progn
	      (setf (aref string string-index) (the string-char (card8->char card8)))
	      (setq string-index (index+ string-index 1))
	      (setq string-left-to-read (index- string-left-to-read 1)))))))))

;;; Reading sequences of chars

(defun read-sequence-char (buffer result-type nitems &optional transform data (start 0))
  (declare (type buffer buffer)
	   (type t result-type) ;; CL type
	   (type array-index nitems start)
	   (type (or null sequence) data))
  (declare-funarg (or null (function (character) t)) transform)
  (if transform 
      (read-sequence-card8
	buffer result-type nitems
	#'(lambda (v)
	    (declare (type card8 v))
	    (funcall transform (card8->char v)))
	data
	start)
    (read-sequence-card8 buffer result-type nitems #'card8->char data start)))

;;; Reading sequences of card8's

(defun read-list-card8 (buffer nitems data start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type list data))
  (reading-buffer-chunks card8
    (do* ((j chunk (index- j 1))
	  (lst (nthcdr i data) (cdr lst))
	  (index 0 (index+ index 1)))
	 ((index-zerop j))
      (declare (type array-index j index)
	       (type cons lst))
      (setf (car lst) (read-card8 index)))))

(defun read-list-card8-with-transform (buffer nitems data transform start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type list data))
  (declare-funarg (function (card8) t) transform)
  (reading-buffer-chunks card8
    (do* ((j chunk (index- j 1))
	  (lst (nthcdr i data) (cdr lst))
	  (index 0 (index+ index 1)))
	 ((index-zerop j))
      (declare (type array-index j index)
	       (type cons lst))
      (setf (car lst) (funcall transform (read-card8 index))))))

#-lispm
(defun read-simple-array-card8 (buffer nitems data start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type (simple-array card8 (*)) data))
  (with-vector (data (simple-array card8 (*)))
    (reading-buffer-chunks card8
      (buffer-replace data buffer-bbuf i end))))

#-lispm
(defun read-simple-array-card8-with-transform (buffer nitems data transform start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type (simple-array card8 (*)) data))
  (declare-funarg (function (card8) card8) transform)
  (with-vector (data (simple-array card8 (*)))
    (reading-buffer-chunks card8
      (do* ((j i (index+ j 1))
	    (index 0 (index+ index 1)))
	   ((index>= j end))
	(declare (type array-index j index))
	(setf (aref data j) (the card8 (funcall transform (read-card8 index))))))))

(defun read-vector-card8 (buffer nitems data start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type vector data))
  (with-vector (data vector)
    (reading-buffer-chunks card8
      (buffer-replace data buffer-bbuf i end))))

(defun read-vector-card8-with-transform (buffer nitems data transform start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type vector data))
  (declare-funarg (function (card8) t) transform)
  (with-vector (data vector)
    (reading-buffer-chunks card8
      (do* ((j i (index+ j 1))
	    (index 0 (index+ index 1)))
	   ((index>= j end))
	(declare (type array-index j index))
	(setf (aref data j) (funcall transform (read-card8 index)))))))

(defun read-sequence-card8 (buffer result-type nitems &optional transform data (start 0))
  (declare (type buffer buffer)
	   (type t result-type) ;; CL type
	   (type array-index nitems start)
	   (type (or null sequence) data))
  (declare-funarg (or null (function (card8) t)) transform)
  (let ((result (or data (make-sequence result-type nitems))))
    (typecase result
      (list
	(if transform 
	    (read-list-card8-with-transform buffer nitems result transform start)
	  (read-list-card8 buffer nitems result start)))
      #-lispm
      ((simple-array card8 (*))
       (if transform 
	   (read-simple-array-card8-with-transform buffer nitems result transform start)
	 (read-simple-array-card8 buffer nitems result start)))
      (t
	(if transform 
	    (read-vector-card8-with-transform buffer nitems result transform start)
	  (read-vector-card8 buffer nitems result start))))
    result))

;;; For now, perhaps performance it isn't worth doing better?

(defun read-sequence-int8 (buffer result-type nitems &optional transform data (start 0))
  (declare (type buffer buffer)
	   (type t result-type) ;; CL type
	   (type array-index nitems start)
	   (type (or null sequence) data))
  (declare-funarg (or null (function (int8) t)) transform)
  (if transform 
      (read-sequence-card8
	buffer result-type nitems
	#'(lambda (v)
	    (declare (type card8 v))
	    (funcall transform (card8->int8 v)))
	data
	start)
    (read-sequence-card8 buffer result-type nitems #'card8->int8 data start)))

;;; Reading sequences of card16's

(defun read-list-card16 (buffer nitems data start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type list data))
  (reading-buffer-chunks card16
    (do* ((j chunk (index- j 1))
	  (lst (nthcdr i data) (cdr lst))
	  (index 0 (index+ index 2)))
	 ((index-zerop j))
      (declare (type array-index j index)
	       (type cons lst))
      (setf (car lst) (read-card16 index)))))

(defun read-list-card16-with-transform (buffer nitems data transform start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type list data))
  (declare-funarg (function (card16) t) transform)
  (reading-buffer-chunks card16
    (do* ((j chunk (index- j 1))
	  (lst (nthcdr i data) (cdr lst))
	  (index 0 (index+ index 2)))
	 ((index-zerop j))
      (declare (type array-index j index)
	       (type cons lst))
      (setf (car lst) (funcall transform (read-card16 index))))))

#-lispm
(defun read-simple-array-card16 (buffer nitems data start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type (simple-array card16 (*)) data))
  (with-vector (data (simple-array card16 (*)))
    (reading-buffer-chunks card16
      (do* ((j i (index+ j 1))
	    (index 0 (index+ index 2)))
	   ((index>= j end))
	(declare (type array-index j index))
	(setf (aref data j) (the card16 (read-card16 index))))
      ;; overlapping case
      (buffer-replace data buffer-wbuf i end))))

#-lispm
(defun read-simple-array-card16-with-transform (buffer nitems data transform start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type (simple-array card16 (*)) data))
  (declare-funarg (function (card16) card16) transform)
  (with-vector (data (simple-array card16 (*)))
    (reading-buffer-chunks card16
      (do* ((j i (index+ j 1))
	    (index 0 (index+ index 2)))
	   ((index>= j end))
	(declare (type array-index j index))
	(setf (aref data j) (the card16 (funcall transform (read-card16 index))))))))

(defun read-vector-card16 (buffer nitems data start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type vector data))
  (with-vector (data vector)
    (reading-buffer-chunks card16
      (do* ((j i (index+ j 1))
	    (index 0 (index+ index 2)))
	   ((index>= j end))
	(declare (type array-index j index))
	(setf (aref data j) (read-card16 index)))
      ;; overlapping case
      (buffer-replace data buffer-wbuf i end))))

(defun read-vector-card16-with-transform (buffer nitems data transform start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type vector data))
  (declare-funarg (function (card16) t) transform)
  (with-vector (data vector)
    (reading-buffer-chunks card16
      (do* ((j i (index+ j 1))
	    (index 0 (index+ index 2)))
	   ((index>= j end))
	(declare (type array-index j index))
	(setf (aref data j) (funcall transform (read-card16 index)))))))

(defun read-sequence-card16 (buffer result-type nitems &optional transform data (start 0))
  (declare (type buffer buffer)
	   (type t result-type) ;; CL type
	   (type array-index nitems start)
	   (type (or null sequence) data))
  (declare-funarg (or null (function (card16) t)) transform)
  (let ((result (or data (make-sequence result-type nitems))))
    (typecase result
      (list
	(if transform 
	    (read-list-card16-with-transform buffer nitems result transform start)
	  (read-list-card16 buffer nitems result start)))
      #-lispm
      ((simple-array card16 (*))
       (if transform 
	   (read-simple-array-card16-with-transform buffer nitems result transform start)
	 (read-simple-array-card16 buffer nitems result start)))
      (t
	(if transform 
	    (read-vector-card16-with-transform buffer nitems result transform start)
	  (read-vector-card16 buffer nitems result start))))
    result))
  
;;; For now, perhaps performance it isn't worth doing better?

(defun read-sequence-int16 (buffer result-type nitems &optional transform data (start 0))
  (declare (type buffer buffer)
	   (type t result-type) ;; CL type
	   (type array-index nitems start)
	   (type (or null sequence) data))
  (declare-funarg (or null (function (int16) t)) transform)
  (if transform 
      (read-sequence-card16
	buffer result-type nitems
	#'(lambda (v)
	    (declare (type card16 v))
	    (funcall transform (card16->int16 v)))
	data
	start)
    (read-sequence-card16 buffer result-type nitems #'card16->int16 data start)))

;;; Reading sequences of card32's

(defun read-list-card32 (buffer nitems data start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type list data))
  (reading-buffer-chunks card32
    (do* ((j chunk (index- j 1))
	  (lst (nthcdr i data) (cdr lst))
	  (index 0 (index+ index 4)))
	 ((index-zerop j))
      (declare (type array-index j index)
	       (type cons lst))
      (setf (car lst) (read-card32 index)))))

(defun read-list-card32-with-transform (buffer nitems data transform start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type list data))
  (declare-funarg (function (card32) t) transform)
  (reading-buffer-chunks card32
    (do* ((j chunk (index- j 1))
	  (lst (nthcdr i data) (cdr lst))
	  (index 0 (index+ index 4)))
	 ((index-zerop j))
      (declare (type array-index j index)
	       (type cons lst))
      (setf (car lst) (funcall transform (read-card32 index))))))

#-lispm
(defun read-simple-array-card32 (buffer nitems data start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type (simple-array card32 (*)) data))
  (with-vector (data (simple-array card32 (*)))
    (reading-buffer-chunks card32
      (do* ((j i (index+ j 1))
	    (index 0 (index+ index 4)))
	   ((index>= j end))
	(declare (type array-index j index))
	(setf (aref data j) (the card32 (read-card32 index))))
      ;; overlapping case
      (buffer-replace data buffer-lbuf i end))))

#-lispm
(defun read-simple-array-card32-with-transform (buffer nitems data transform start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type (simple-array card32 (*)) data))
  (declare-funarg (function (card32) card32) transform)
  (with-vector (data (simple-array card32 (*)))
    (reading-buffer-chunks card32
      (do* ((j i (index+ j 1))
	    (index 0 (index+ index 4)))
	   ((index>= j end))
	(declare (type array-index j index))
	(setf (aref data j) (the card32 (funcall transform (read-card32 index))))))))

(defun read-vector-card32 (buffer nitems data start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type vector data))
  (with-vector (data vector)
    (reading-buffer-chunks card32
      (do* ((j i (index+ j 1))
	    (index 0 (index+ index 4)))
	   ((index>= j end))
	(declare (type array-index j index))
	(setf (aref data j) (read-card32 index)))
      ;; overlapping case
      (buffer-replace data buffer-lbuf i end))))

(defun read-vector-card32-with-transform (buffer nitems data transform start)
  (declare (type buffer buffer)
	   (type array-index nitems start)
	   (type vector data))
  (declare-funarg (function (card32) t) transform)
  (with-vector (data vector)
    (reading-buffer-chunks card32
      (do* ((j i (index+ j 1))
	    (index 0 (index+ index 4)))
	   ((index>= j end))
	(declare (type array-index j index))
	(setf (aref data j) (funcall transform (read-card32 index)))))))

(defun read-sequence-card32 (buffer result-type nitems &optional transform data (start 0))
  (declare (type buffer buffer)
	   (type t result-type) ;; CL type
	   (type array-index nitems start)
	   (type (or null sequence) data))
  (declare-funarg (or null (function (card32) t)) transform)
  (let ((result (or data (make-sequence result-type nitems))))
    (typecase result
      (list
	(if transform 
	    (read-list-card32-with-transform buffer nitems result transform start)
	  (read-list-card32 buffer nitems result start)))
      #-lispm
      ((simple-array card32 (*))
       (if transform 
	   (read-simple-array-card32-with-transform buffer nitems result transform start)
	 (read-simple-array-card32 buffer nitems result start)))
      (t
	(if transform 
	    (read-vector-card32-with-transform buffer nitems result transform start)
	  (read-vector-card32 buffer nitems result start))))
    result))

;;; For now, perhaps performance it isn't worth doing better?

(defun read-sequence-int32 (buffer result-type nitems &optional transform data (start 0))
  (declare (type buffer buffer)
	   (type t result-type) ;; CL type
	   (type array-index nitems start)
	   (type (or null sequence) data))
  (declare-funarg (or null (function (int32) t)) transform)
  (if transform 
      (read-sequence-card32
	buffer result-type nitems
	#'(lambda (v)
	    (declare (type card32 v))
	    (funcall transform (card32->int32 v)))
	data
	start)
    (read-sequence-card32 buffer result-type nitems #'card32->int32 data start)))

;;; Writing sequences of chars

(defun write-sequence-char
       (buffer boffset data &optional (start 0) (end (length data)) transform)
  (declare (type buffer buffer)
	   (type sequence data)
	   (type array-index boffset start end))
  (declare-funarg (or null (function (t) character)) transform)
  (if transform 
      (write-sequence-card8
	buffer boffset data start end
	#'(lambda (x) (char->card8 (the character (funcall transform x)))))
    (write-sequence-card8 buffer boffset data start end #'char->card8)))

;;; Writing sequences of card8's

(defun write-list-card8 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type list data)
	   (type array-index boffset start end))
  (writing-buffer-chunks card8
			 ((lst (nthcdr start data)))
			 ((type list lst))
    (dotimes (j chunk)
      (declare (type array-index j))
      (write-card8 j (pop lst))))
  nil)

(defun write-list-card8-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type list data)
	   (type array-index boffset start end))
  (declare-funarg (function (t) card8) transform)
  (writing-buffer-chunks card8
			 ((lst (nthcdr start data)))
			 ((type list lst))
    (dotimes (j chunk)
      (declare (type array-index j))
      (write-card8 j (funcall transform (pop lst)))))
  nil)

;;; Should really write directly from data, instead of into the buffer first
#-lispm
(defun write-simple-array-card8 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type (simple-array card8 (*)) data)
	   (type array-index boffset start end))
  (with-vector (data (simple-array card8 (*)))
    (writing-buffer-chunks card8
			   ((index start (index+ index chunk)))
			   ((type array-index index))
      (buffer-replace buffer-bbuf data
		      buffer-boffset
		      (index+ buffer-boffset chunk)
		      index)))
  nil)

#-lispm
(defun write-simple-array-card8-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type (simple-array card8 (*)) data)
	   (type array-index boffset start end))
  (declare-funarg (function (card8) card8) transform)
  (with-vector (data (simple-array card8 (*)))
    (writing-buffer-chunks card8
			   ((index start))
			   ((type array-index index))
      (dotimes (j chunk)
	(declare (type array-index j))
	(write-card8 j (funcall transform (aref data index)))
	(setq index (index+ index 1)))))
  nil)

(defun write-vector-card8 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type vector data)
	   (type array-index boffset start end))
  (with-vector (data vector)
    (writing-buffer-chunks card8
			   ((index start (index+ index chunk)))
			   ((type array-index index))
      (buffer-replace buffer-bbuf data
		      buffer-boffset
		      (index+ buffer-boffset chunk)
		      index)))
  nil)

(defun write-vector-card8-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type vector data)
	   (type array-index boffset start end))
  (declare-funarg (function (t) card8) transform)
  (with-vector (data vector)
    (writing-buffer-chunks card8
			   ((index start))
			   ((type array-index index))
      (dotimes (j chunk)
	(declare (type array-index j))
	(write-card8 j (funcall transform (aref data index)))
	(setq index (index+ index 1)))))
  nil)

(defun write-sequence-card8
       (buffer boffset data &optional (start 0) (end (length data)) transform)
  (declare (type buffer buffer)
	   (type sequence data)
	   (type array-index boffset start end))
  (declare-funarg (or null (function (t) card8)) transform)
  (typecase data
    (list
      (if transform
	  (write-list-card8-with-transform buffer boffset data start end transform)
	  (write-list-card8 buffer boffset data start end)))
    #-lispm
    ((simple-array card8 (*))
     (if transform
	 (write-simple-array-card8-with-transform buffer boffset data start end transform)
	 (write-simple-array-card8 buffer boffset data start end)))
    (t
      (if transform
	  (write-vector-card8-with-transform buffer boffset data start end transform)
	  (write-vector-card8 buffer boffset data start end)))))

;;; For now, perhaps performance it isn't worth doing better?

(defun write-sequence-int8
       (buffer boffset data &optional (start 0) (end (length data)) transform)
  (declare (type buffer buffer)
	   (type sequence data)
	   (type array-index boffset start end))
  (declare-funarg (or null (function (t) int8)) transform)
  (if transform 
      (write-sequence-card8
	buffer boffset start end
	#'(lambda (x) (int8->card8 (the int8 (funcall transform x))))
	data)
      (write-sequence-card8 buffer boffset start end #'int8->card8)))

;;; Writing sequences of card16's

(defun write-list-card16 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type list data)
	   (type array-index boffset start end))
  (writing-buffer-chunks card16
			 ((lst (nthcdr start data)))
			 ((type list lst))
    ;; Depends upon the chunks being an even multiple of card16's big
    (do ((j 0 (index+ j 2)))
	((index>= j chunk))
      (declare (type array-index j))
      (write-card16 j (pop lst))))
  nil)

(defun write-list-card16-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type list data)
	   (type array-index boffset start end))
  (declare-funarg (function (t) card16) transform)
  (writing-buffer-chunks card16
			 ((lst (nthcdr start data)))
			 ((type list lst))
    ;; Depends upon the chunks being an even multiple of card16's big
    (do ((j 0 (index+ j 2)))
	((index>= j chunk))
      (declare (type array-index j))
      (write-card16 j (funcall transform (pop lst)))))
  nil)

#-lispm
(defun write-simple-array-card16 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type (simple-array card16 (*)) data)
	   (type array-index boffset start end))
  (with-vector (data (simple-array card16 (*)))
    (writing-buffer-chunks card16
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of card16's big
      (do ((j 0 (index+ j 2)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-card16 j (aref data index))
	(setq index (index+ index 1)))
      ;; overlapping case
      (let ((length (floor chunk 2)))
	(buffer-replace buffer-wbuf data
			buffer-woffset
			(index+ buffer-woffset length)
			index)
	(setq index (index+ index length)))))
  nil)

#-lispm
(defun write-simple-array-card16-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type (simple-array card16 (*)) data)
	   (type array-index boffset start end))
  (declare-funarg (function (card16) card16) transform)
  (with-vector (data (simple-array card16 (*)))
    (writing-buffer-chunks card16
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of card16's big
      (do ((j 0 (index+ j 2)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-card16 j (funcall transform (aref data index)))
	(setq index (index+ index 1)))))
  nil)

(defun write-vector-card16 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type vector data)
	   (type array-index boffset start end))
  (with-vector (data vector)
    (writing-buffer-chunks card16
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of card16's big
      (do ((j 0 (index+ j 2)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-card16 j (aref data index))
	(setq index (index+ index 1)))
      ;; overlapping case
      (let ((length (floor chunk 2)))
	(buffer-replace buffer-wbuf data
			buffer-woffset
			(index+ buffer-woffset length)
			index)
	(setq index (index+ index length)))))
  nil)

(defun write-vector-card16-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type vector data)
	   (type array-index boffset start end))
  (declare-funarg (function (t) card16) transform)
  (with-vector (data vector)
    (writing-buffer-chunks card16
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of card16's big
      (do ((j 0 (index+ j 2)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-card16 j (funcall transform (aref data index)))
	(setq index (index+ index 1)))))
  nil)

(defun write-sequence-card16
       (buffer boffset data &optional (start 0) (end (length data)) transform)
  (declare (type buffer buffer)
	   (type sequence data)
	   (type array-index boffset start end))
  (declare-funarg (or null (function (t) card16)) transform)
  (typecase data
    (list
      (if transform
	  (write-list-card16-with-transform buffer boffset data start end transform)
	  (write-list-card16 buffer boffset data start end)))
    #-lispm
    ((simple-array card16 (*))
     (if transform
	 (write-simple-array-card16-with-transform buffer boffset data start end transform)
	 (write-simple-array-card16 buffer boffset data start end)))
    (t
      (if transform
	  (write-vector-card16-with-transform buffer boffset data start end transform)
	  (write-vector-card16 buffer boffset data start end)))))

;;; Writing sequences of int16's

(defun write-list-int16 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type list data)
	   (type array-index boffset start end))
  (writing-buffer-chunks int16
			 ((lst (nthcdr start data)))
			 ((type list lst))
    ;; Depends upon the chunks being an even multiple of int16's big
    (do ((j 0 (index+ j 2)))
	((index>= j chunk))
      (declare (type array-index j))
      (write-int16 j (pop lst))))
  nil)

(defun write-list-int16-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type list data)
	   (type array-index boffset start end))
  (declare-funarg (function (t) int16) transform)
  (writing-buffer-chunks int16
			 ((lst (nthcdr start data)))
			 ((type list lst))
    ;; Depends upon the chunks being an even multiple of int16's big
    (do ((j 0 (index+ j 2)))
	((index>= j chunk))
      (declare (type array-index j))
      (write-int16 j (funcall transform (pop lst)))))
  nil)

#-lispm
(defun write-simple-array-int16 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type (simple-array int16 (*)) data)
	   (type array-index boffset start end))
  (with-vector (data (simple-array int16 (*)))
    (writing-buffer-chunks int16
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of int16's big
      (do ((j 0 (index+ j 2)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-int16 j (aref data index))
	(setq index (index+ index 1)))
      ;; overlapping case
      (let ((length (floor chunk 2)))
	(buffer-replace buffer-wbuf data
			buffer-woffset
			(index+ buffer-woffset length)
			index)
	(setq index (index+ index length)))))
  nil)

#-lispm
(defun write-simple-array-int16-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type (simple-array int16 (*)) data)
	   (type array-index boffset start end))
  (declare-funarg (function (int16) int16) transform)
  (with-vector (data (simple-array int16 (*)))
    (writing-buffer-chunks int16
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of int16's big
      (do ((j 0 (index+ j 2)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-int16 j (funcall transform (aref data index)))
	(setq index (index+ index 1)))))
  nil)

(defun write-vector-int16 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type vector data)
	   (type array-index boffset start end))
  (with-vector (data vector)
    (writing-buffer-chunks int16
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of int16's big
      (do ((j 0 (index+ j 2)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-int16 j (aref data index))
	(setq index (index+ index 1)))
      ;; overlapping case
      (let ((length (floor chunk 2)))
	(buffer-replace buffer-wbuf data
			buffer-woffset
			(index+ buffer-woffset length)
			index)
	(setq index (index+ index length)))))
  nil)

(defun write-vector-int16-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type vector data)
	   (type array-index boffset start end))
  (declare-funarg (function (t) int16) transform)
  (with-vector (data vector)
    (writing-buffer-chunks int16
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of int16's big
      (do ((j 0 (index+ j 2)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-int16 j (funcall transform (aref data index)))
	(setq index (index+ index 1)))))
  nil)

(defun write-sequence-int16
       (buffer boffset data &optional (start 0) (end (length data)) transform)
  (declare (type buffer buffer)
	   (type sequence data)
	   (type array-index boffset start end))
  (declare-funarg (or null (function (t) int16)) transform)
  (typecase data
    (list
      (if transform
	  (write-list-int16-with-transform buffer boffset data start end transform)
	  (write-list-int16 buffer boffset data start end)))
    #-lispm
    ((simple-array int16 (*))
     (if transform
	 (write-simple-array-int16-with-transform buffer boffset data start end transform)
	 (write-simple-array-int16 buffer boffset data start end)))
    (t
      (if transform
	  (write-vector-int16-with-transform buffer boffset data start end transform)
	  (write-vector-int16 buffer boffset data start end)))))

;;; Writing sequences of card32's

(defun write-list-card32 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type list data)
	   (type array-index boffset start end))
  (writing-buffer-chunks card32
			 ((lst (nthcdr start data)))
			 ((type list lst))
    ;; Depends upon the chunks being an even multiple of card32's big
    (do ((j 0 (index+ j 4)))
	((index>= j chunk))
      (declare (type array-index j))
      (write-card32 j (pop lst))))
  nil)

(defun write-list-card32-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type list data)
	   (type array-index boffset start end))
  (declare-funarg (function (t) card32) transform)
  (writing-buffer-chunks card32
			 ((lst (nthcdr start data)))
			 ((type list lst))
    ;; Depends upon the chunks being an even multiple of card32's big
    (do ((j 0 (index+ j 4)))
	((index>= j chunk))
      (declare (type array-index j))
      (write-card32 j (funcall transform (pop lst)))))
  nil)

#-lispm
(defun write-simple-array-card32 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type (simple-array card32 (*)) data)
	   (type array-index boffset start end))
  (with-vector (data (simple-array card32 (*)))
    (writing-buffer-chunks card32
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of card32's big
      (do ((j 0 (index+ j 4)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-card32 j (aref data index))
	(setq index (index+ index 1)))
      ;; overlapping case
      (let ((length (floor chunk 4)))
	(buffer-replace buffer-lbuf data
			buffer-loffset
			(index+ buffer-loffset length)
			index)
	(setq index (index+ index length)))))
  nil)

#-lispm
(defun write-simple-array-card32-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type (simple-array card32 (*)) data)
	   (type array-index boffset start end))
  (declare-funarg (function (card32) card32) transform)
  (with-vector (data (simple-array card32 (*)))
    (writing-buffer-chunks card32
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of card32's big
      (do ((j 0 (index+ j 4)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-card32 j (funcall transform (aref data index)))
	(setq index (index+ index 1)))))
  nil)

(defun write-vector-card32 (buffer boffset data start end)
  (declare (type buffer buffer)
	   (type vector data)
	   (type array-index boffset start end))
  (with-vector (data vector)
    (writing-buffer-chunks card32
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of card32's big
      (do ((j 0 (index+ j 4)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-card32 j (aref data index))
	(setq index (index+ index 1)))
      ;; overlapping case
      (let ((length (floor chunk 4)))
	(buffer-replace buffer-lbuf data
			buffer-loffset
			(index+ buffer-loffset length)
			index)
	(setq index (index+ index length)))))
  nil)

(defun write-vector-card32-with-transform (buffer boffset data start end transform)
  (declare (type buffer buffer)
	   (type vector data)
	   (type array-index boffset start end))
  (declare-funarg (function (t) card32) transform)
  (with-vector (data vector)
    (writing-buffer-chunks card32
			   ((index start))
			   ((type array-index index))
      ;; Depends upon the chunks being an even multiple of card32's big
      (do ((j 0 (index+ j 4)))
	  ((index>= j chunk))
	(declare (type array-index j))
	(write-card32 j (funcall transform (aref data index)))
	(setq index (index+ index 1)))))
  nil)

(defun write-sequence-card32
       (buffer boffset data &optional (start 0) (end (length data)) transform)
  (declare (type buffer buffer)
	   (type sequence data)
	   (type array-index boffset start end))
  (declare-funarg (or null (function (t) card32)) transform)
  (typecase data
    (list
      (if transform
	  (write-list-card32-with-transform buffer boffset data start end transform)
	  (write-list-card32 buffer boffset data start end)))
    #-lispm
    ((simple-array card32 (*))
     (if transform
	 (write-simple-array-card32-with-transform buffer boffset data start end transform)
	 (write-simple-array-card32 buffer boffset data start end)))
    (t
      (if transform
	  (write-vector-card32-with-transform buffer boffset data start end transform)
	  (write-vector-card32 buffer boffset data start end)))))

;;; For now, perhaps performance it isn't worth doing better?

(defun write-sequence-int32
       (buffer boffset data &optional (start 0) (end (length data)) transform)
  (declare (type buffer buffer)
	   (type sequence data)
	   (type array-index boffset start end))
  (declare-funarg (or null (function (t) int32)) transform)
  (if transform 
      (write-sequence-card32
	buffer boffset start end
	#'(lambda (x) (int32->card32 (the int32 (funcall transform x))))
	data)
      (write-sequence-card32 buffer boffset start end #'int32->card32)))

(defun read-bitvector256 (buffer-bbuf boffset data)
  (declare (type buffer-bytes buffer-bbuf)
	   (type array-index boffset)
	   (type (or null (simple-bit-vector 256)) data))
  (let ((result (or data (make-array 256 :element-type 'bit :initial-element 0))))
    (declare-array (simple-bit-vector 256) result)
    (do ((i (index+ boffset 1) (index+ i 1)) ;; Skip first byte
	 (j 8 (index+ j 8)))
	((index>= j 256))
      (declare (type array-index i j))
      (do ((byte (aref-card8 buffer-bbuf i) (index-ash byte -1))
	   (k j (index+ k 1)))
	  ((zerop byte)
	   (when data ;; Clear uninitialized bits in data
	     (do ((end (index+ j 8)))
		 ((= k end))
	       (setf (aref result k) 0)
	       (index-incf k))))
	(declare (type array-index k)
		 (type card8 byte))
	(setf (aref result k) (the bit (logand byte 1)))))
    result))

(defun write-bitvector256 (buffer boffset map)
  (declare (type buffer buffer)
	   (type array-index boffset)
	   (type (simple-array bit (*)) map))
  (writing-buffer-send (buffer :index boffset :sizes 8)
    (do* ((i (index+ buffer-boffset 1) (index+ i 1))	; Skip first byte
	  (j 8 (index+ j 8)))		
	 ((index>= j 256))
      (declare (type array-index i j))
      (do ((byte 0)
	   (bit (index+ j 7) (index- bit 1)))
	  ((index< bit j)
	   (aset-card8 byte buffer-bbuf i))
	(declare (type array-index bit)
		 (type card8 byte))
	(setq byte (the card8 (logior (the card8 (ash byte 1)) (aref map bit))))))))
