;;; -*- Mode: LISP; Syntax: Common-lisp; Package: XLIB; Base: 10; Lowercase: Yes -*-

;;; This file contains macro definitions for the BUFFER object for Common-Lisp
;;; X windows version 11

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

;;; The read- macros are in buffer.lisp, because event-case depends on (most of) them.

(defmacro write-card8 (byte-index item)
  `(setf (aref-card8 buffer-bbuf (index+ buffer-boffset ,byte-index))
	 (the card8 ,item)))

(defmacro write-int8 (byte-index item)
  `(setf (aref-int8 buffer-bbuf (index+ buffer-boffset ,byte-index))
	 (the int8 ,item)))

(defmacro write-card16 (byte-index item)
  #+clx-overlapping-arrays
  `(setf (aref-card16 buffer-wbuf (index+ buffer-woffset (index-ash ,byte-index -1)))
	 (the card16 ,item))
  #-clx-overlapping-arrays
  `(setf (aref-card16 buffer-bbuf (index+ buffer-boffset ,byte-index))
	 (the card16 ,item)))

(defmacro write-int16 (byte-index item)
  #+clx-overlapping-arrays
  `(setf (aref-int16 buffer-wbuf (index+ buffer-woffset (index-ash ,byte-index -1)))
	 (the int16 ,item))
  #-clx-overlapping-arrays
  `(setf (aref-int16 buffer-bbuf (index+ buffer-boffset ,byte-index))
	 (the int16 ,item)))

(defmacro write-card32 (byte-index item)
  #+clx-overlapping-arrays
  `(setf (aref-card32 buffer-lbuf (index+ buffer-loffset (index-ash ,byte-index -2)))
	 (the card32 ,item))
  #-clx-overlapping-arrays
  `(setf (aref-card32 buffer-bbuf (index+ buffer-boffset ,byte-index))
	 (the card32 ,item)))

(defmacro write-int32 (byte-index item)
  #+clx-overlapping-arrays
  `(setf (aref-int32 buffer-lbuf (index+ buffer-loffset (index-ash ,byte-index -2)))
	 (the int32 ,item))
  #-clx-overlapping-arrays
  `(setf (aref-int32 buffer-bbuf (index+ buffer-boffset ,byte-index))
	 (the int32 ,item)))

(defmacro write-card29 (byte-index item)
  #+clx-overlapping-arrays
  `(setf (aref-card29 buffer-lbuf (index+ buffer-loffset (index-ash ,byte-index -2)))
	 (the card29 ,item))
  #-clx-overlapping-arrays
  `(setf (aref-card29 buffer-bbuf (index+ buffer-boffset ,byte-index))
	 (the card29 ,item)))

(defmacro set-buffer-offset (value &key (sizes '(8 16 32)))
  (unless (listp sizes) (setq sizes (list sizes)))
  `(progn
     (setq buffer-boffset ,value)
     #+clx-overlapping-arrays
     ,@(when (member 16 sizes)
	 `((setq buffer-woffset (the array-index (index-ash buffer-boffset -1)))))
     #+clx-overlapping-arrays
     ,@(when (member 32 sizes)
	 `((setq buffer-loffset (the array-index (index-ash buffer-boffset -2)))))))

(defmacro with-buffer-output ((buffer &key (sizes '(8 16 32)) length index) &body body)
  (unless (listp sizes) (setq sizes (list sizes)))
  `(let ()
     (declare-bufmac)
     ,(if length
	  (unless (eq length :none)
	    `(when (index>= (index+ (buffer-boffset ,buffer) ,length) (buffer-limit ,buffer))
	       (buffer-flush ,buffer)))
	`(when (index>= (buffer-boffset ,buffer) (buffer-limit ,buffer))
	   (buffer-flush ,buffer)))
     (let* ((buffer-boffset (the array-index ,(or index `(buffer-boffset ,buffer))))
	    ,@(when (or #-clx-overlapping-arrays t (member 8 sizes))
		`((buffer-bbuf (buffer-obuf8 ,buffer))))
	    #+clx-overlapping-arrays
	    ,@(when (member 16 sizes)
		`((buffer-woffset (index-ash buffer-boffset -1))
		  (buffer-wbuf (buffer-obuf16 ,buffer))))
	    #+clx-overlapping-arrays
	    ,@(when (member 32 sizes)
		`((buffer-loffset (index-ash buffer-boffset -2))
		  (buffer-lbuf (buffer-obuf32 ,buffer)))))
       (declare (type array-index buffer-boffset))
       ,@(when (or #-clx-overlapping-arrays t (member 8 sizes))
	   '((declare-array buffer-bytes buffer-bbuf)))
       #+clx-overlapping-arrays
       ,@(when (member 16 sizes)
	   `((declare (type array-index buffer-woffset))
	     (declare-array buffer-words buffer-wbuf)))
       #+clx-overlapping-arrays
       ,@(when (member 32 sizes)
	   `((declare (type array-index buffer-loffset))
	     (declare-array buffer-longs buffer-lbuf)))
       buffer-boffset
       ,@(when (or #-clx-overlapping-arrays t (member 8 sizes)) '(buffer-bbuf))
       #+clx-overlapping-arrays
       ,@(when (member 16 sizes) '(buffer-woffset buffer-wbuf))
       #+clx-overlapping-arrays
       ,@(when (member 32 sizes) '(buffer-loffset buffer-lbuf))
       ,@body
       )))

(defmacro writing-buffer-send ((buffer &rest options) &body body)
  ;; BODY may contain calls to (WRITE32 item) (WRITE16 item) or (WRITE8 item)
  ;; These calls will place "item" in the next available word, half-word
  ;; or byte in BUFFER.  BUFFER will be flushed when full.
  (declare-arglist (buffer &key sizes index length) &body body)
  `(compiler-let ((*buffer* ',buffer))
     (with-buffer-output (,buffer ,@options) ,@body)))

(defmacro reading-buffer-reply ((buffer &rest options) &body body)
  (declare-arglist (buffer &key sizes) &body body)
  ;; BODY may contain calls to (READ32 &optional index) etc.
  ;; These calls will read from the input buffer at byte
  ;; offset INDEX.  If INDEX is not supplied, then the next
  ;; word, half-word or byte is returned.
  (let ((reply-buffer (gensym)))
    `(let ((,reply-buffer (buffer-reply-buffer ,buffer)))
       (compiler-let ((*buffer* ',buffer))
	 (with-buffer-input (,reply-buffer ,@options) ,@body)))))

;;; These next two macros are just used internally in buffer

(defmacro reading-buffer-chunks (type &body body)
  (when (> (length body) 2)
    (error "reading-buffer-chunks called with too many forms"))
  (let* ((size (* 8 (index-increment type)))
	 (form #-clx-overlapping-arrays
	       (first body)
	       #+clx-overlapping-arrays		; XXX type dependencies
	       (or (second body)
		   (first body))))
    `(reading-buffer-reply (buffer :sizes ,(reverse (adjoin size '(8))))
       (do* ((i start end)
	     (i-end (index+ start nitems))
	     (size (index-ash (reply-size (buffer-reply-buffer buffer))
			      ,(- (truncate size 16))))
	     (len nitems (index- len chunk))
	     (chunk (index-min size len) (index-min size len))
	     (end (index+ i chunk) (index+ i chunk)))
	    ((index>= i i-end) data)
	 (declare (type array-index i-end size len chunk end i))
	 (buffer-input buffer buffer-bbuf 0
		       (lround ,(if (= size 8)
				    'chunk
				    `(index-ash chunk ,(truncate size 16)))))
	 ,form))))

(defmacro writing-buffer-chunks (type args decls &body body)
  (when (> (length body) 2)
    (error "writing-buffer-chunks called with too many forms"))
  (let* ((size (* 8 (index-increment type)))
	 (form #-clx-overlapping-arrays
	       (first body)
	       #+clx-overlapping-arrays		; XXX type dependencies
	       (or (second body)
		   (first body))))
    `(writing-buffer-send (buffer :index boffset :sizes ,(reverse (adjoin size '(8))))
       ;; Loop filling the buffer
       (do* (,@args
	     ;; Number of bytes needed to output
	     (len ,(if (= size 8)
		       `(index- end start)
		       `(index-ash (index- end start) ,(truncate size 16)))
		  (index- len chunk))
	     ;; Number of bytes available in buffer
	     (chunk (index-min len (index- (buffer-size buffer) buffer-boffset))
		    (index-min len (index- (buffer-size buffer) buffer-boffset))))
	    ((not (index-plusp len)))
	 (declare ,@decls
		  (type array-index len chunk))
	 ,form
	 (index-incf buffer-boffset chunk)
	 ;; Flush the buffer
	 (when (and (index-plusp len)
		    (index> buffer-boffset (buffer-limit buffer)))
	   (setf (buffer-boffset buffer) buffer-boffset)
	   (buffer-flush buffer)
	   (setq buffer-boffset (buffer-boffset buffer))
	   #+clx-overlapping-arrays
	   ,(case size
	      (16 '(setq buffer-woffset (index-ash buffer-boffset -1)))
	      (32 '(setq buffer-loffset (index-ash buffer-boffset -2))))))
       (setf (buffer-boffset buffer) (lround buffer-boffset)))))
