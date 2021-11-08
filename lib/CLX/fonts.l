;;; -*- Mode: LISP; Syntax: Common-lisp; Package: XLIB; Base: 10; Lowercase: Yes -*-

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
	  open-font
	  discard-font-info
	  close-font
	  list-font-names
	  list-fonts
	  font-path))

;; The char-info stuff is here instead of CLX because of uses of int16->card16.

; To allow efficient storage representations, the type char-info is not
; required to be a structure.

;; For each of left-bearing, right-bearing, width, ascent, descent, attributes:

;(defun char-<metric> (font index)
;  ;; Note: I have tentatively chosen to return nil for an out-of-bounds index
;  ;; (or an in-bounds index on a pseudo font), although returning zero or
;  ;; signalling might be better.
;  (declare (type font font)
;	   (type integer index)
;	   (values (or null integer))))

;(defun max-char-<metric> (font)
;  ;; Note: I have tentatively chosen separate accessors over allowing :min and
;  ;; :max as an index above.
;  (declare (type font font)
;	   (values integer)))

;(defun min-char-<metric> (font)
;  (declare (type font font)
;	   (values integer)))

;; Note: char16-<metric> accessors could be defined to accept two-byte indexes.

(deftype char-info-vec () '(simple-array int16 (6)))

(eval-when (eval compile) ;; I'd rather use macrolet, but Symbolics doesn't like it...

(defmacro def-char-info-accessors (useless-name &body fields)
  `(within-definition (,useless-name def-char-info-accessors)
     ,@(do ((field fields (cdr field))
	    (n 0 (1+ n))
	    (name) (type)
	    (result nil))
	   ((endp field) result)
	 (setq name (xintern 'char- (caar field)))
	 (setq type (cadar field))
	 (flet ((from (form)
		  (if (eq type 'int16)
		      form
		      `(,(xintern 'int16-> type) ,form))))
	   (push
	     `(defun ,name (font index)
		(declare (type font font)
			 (type array-index index))
		(declare-values ,type)
		(let ((char-info-vector (font-char-infos font))
		      (char-entry (index+ (index* 6 (index+ (font-min-char font) index))
					  ,n)))
		  (declare (type char-info-vec char-info-vector)
			   (type array-index char-entry))
		  (the ,type
		       ,(from
			  `(the int16
				(if (index< char-entry (length char-info-vector))
				    ;; Variable width font
				    (aref char-info-vector char-entry)
				    ;; Fixed width font
				    (aref (the char-info-vec (font-max-bounds font))
					  ,n)))))))
	     result)
	   (push `(proclaim '(inline ,name)) result)
	   (setq name (xintern 'min-char- (caar field)))
	   (push
	     `(defun ,name (font)
		(declare (type font font))
		(declare-values ,type)
		(the ,type ,(from `(the int16
					(aref (the char-info-vec (font-min-bounds font))
					      ,n)))))
	     result)
	   (push `(proclaim '(inline ,name)) result)
	   (setq name (xintern 'max-char- (caar field)))
	   (push
	     `(defun ,name (font)
		(declare (type font font))
		(declare-values ,type)
		(the ,type ,(from `(the int16
					(aref (the char-info-vec (font-max-bounds font))
					      ,n)))))
	     result)
	   (push `(proclaim '(inline ,name)) result)))
	  
     (defun make-char-info
	    (&key ,@(mapcar
		      #'(lambda (field)
			  `(,(car field) (required-arg ,(car field))))
		      fields))
       (declare ,@(mapcar #'(lambda (field) `(type ,@(reverse field))) fields))
       (let ((result (make-array ,(length fields) :element-type 'int16)))
	 (declare-array char-info-vec result)
	 ,@(do* ((field fields (cdr field))
		 (var (caar field) (caar field))
		 (type (cadar field) (cadar field))
		 (n 0 (1+ n))
		 (result nil))
		((endp field) (nreverse result))
	     (push `(setf (aref result ,n)
			  ,(if (eq type 'int16)
			       var
			       `(,(xintern type '->int16) ,var)))
		   result))
	 result))))
) ;; End eval-when

(def-char-info-accessors ignore
  (left-bearing int16)
  (right-bearing int16)
  (width int16)
  (ascent int16)
  (descent int16)
  (attributes card16))
    
(defun open-font (display name)
  ;; Font objects may be cached and reference counted locally within the display
  ;; object.  This function might not execute a with-display if the font is cached.
  ;; The protocol QueryFont request happens on-demand under the covers.
  (declare (type display display)
	   (type stringable name))
  (declare-values font)
  (let* ((name-string (string-downcase (string name)))
	 (font (car (member name-string (display-font-cache display)
			    :key 'font-name
			    :test 'equal)))
	 font-id)
    (unless font
      (setq font (make-font :display display :name name-string))
      (setq font-id (allocate-resource-id display font 'font))
      (setf (font-id-internal font) font-id)
      (with-buffer-request (display *x-openfont*)
	(resource-id font-id)
	(card16 (length name-string))
	(pad16 nil)
	(string name-string))
      (push font (display-font-cache display)))
    (incf (font-reference-count font))
    font))

(defun open-font-internal (font)
  ;; Called "under the covers" to open a font object
  (declare (type font font))
  (declare-values resource-id)
  (let* ((name-string (font-name font))
	 (display (font-display font))
	 (id (allocate-resource-id display font 'font)))
    (setf (font-id-internal font) id)
    (with-buffer-request (display *x-openfont*)
      (resource-id id)
      (card16 (length name-string))
      (pad16 nil)
      (string name-string))
    (push font (display-font-cache display))
    (incf (font-reference-count font))
    id))

(defun discard-font-info (font)
  ;; Discards any state that can be re-obtained with QueryFont.  This is
  ;; simply a performance hint for memory-limited systems.
  (declare (type font font))
  (setf (font-font-info-internal font) nil
	(font-char-infos-internal font) nil))

(defun query-font (font)
  ;; Internal function called by font and char info accessors
  (declare (type font font))
  (declare-values font-info)
  (let ((display (font-display font))
	font-id
	font-info
	props)
    (with-display (display)
      (setq font-id (font-id font)) ;; May issue an open-font request
      (with-buffer-request (display *x-queryfont* :no-after)
	(resource-id font-id))
      (wait-for-reply display nil)
      (reading-buffer-reply (display)
	(buffer-input display buffer-bbuf *replysize* 60)
	(let* ((min-byte2 (card16-get 40))
	       (max-byte2 (card16-get 42))
	       (min-byte1 (card8-get 49))
	       (max-byte1 (card8-get 50))
	       (min-char  min-byte2)
	       (max-char  (+ (ash max-byte1 8) max-byte2))
	       (nfont-props (card16-get 46))
	       (nchar-infos (* (card32-get 56) 6))
	       (char-info (make-array nchar-infos :element-type 'int16)))
	  (setq font-info
		(make-font-info
		  :direction (member8-get 48 :left-to-right :right-to-left)
		  :min-char min-char
		  :max-char max-char
		  :min-byte1 min-byte1
		  :max-byte1 max-byte1
		  :min-byte2 min-byte2
		  :max-byte2 max-byte2
		  :all-chars-exist-p (boolean-get 51)
		  :default-char (card16-get 44)
		  :ascent (int16-get 52)
		  :descent (int16-get 54)
		  :min-bounds (char-info-get 8)
		  :max-bounds (char-info-get 24)))
	  (setq props (sequence-get :length (* 2 nfont-props) :format int32
				    :result-type 'list))
	  (sequence-get :length nchar-infos :format int16 :data char-info)
	  (setf (font-char-infos-internal font) char-info)
	  (setf (font-font-info-internal font) font-info)
	  )))
    (display-invoke-after-function display)
    ;; Replace atom id's with keywords in the plist
    (do ((p props (cddr p)))
	((endp p))
      (setf (car p) (lookup-xatom display (car p))))
    (setf (font-info-properties font-info) props)
    font-info))

(defun close-font (font)
  ;; This might not generate a protocol request if the font is reference
  ;; counted locally.
  (declare (type font font))
  (when (and (not (plusp (decf (font-reference-count font))))
	     (font-id-internal font))
    (let ((display (font-display font))
	  (id (font-id-internal font)))
      (declare (type display display))
      ;; Remove font from cache
      (setf (display-font-cache display) (delete font (display-font-cache display)))
      ;; Close the font
      (with-buffer-request (display *x-closefont*)
	(resource-id id)))))

(defun list-font-names (display pattern &key (max-fonts 65535) (result-type 'list))
  (declare (type display display)
	   (type string pattern)
	   (type card16 max-fonts)
	   (type t result-type)) ;; CL type
  (declare-values (sequence string))
  (let ((string (string pattern))
	result)
    (with-display (display)
      (with-buffer-request (display *x-listfonts* :no-after)
	(card16 max-fonts (length string))
	(string string))
      (reading-buffer-reply (display :sizes 16)
	(let ((length (- (wait-for-reply display nil) *replysize*))
	      (nfonts (card16-get 8)))
	  (setq result (read-sequence-string display length nfonts result-type)))))
    (display-invoke-after-function display)
    result))

(defun list-fonts (display pattern &key (max-fonts 65535) (result-type 'list))
  ;; Note: Was called list-fonts-with-info.
  ;; Returns "pseudo" fonts that contain basic font metrics and properties, but
  ;; no per-character metrics and no resource-ids.  These pseudo fonts will be
  ;; converted (internally) to real fonts dynamically as needed, by issuing an
  ;; OpenFont request.  However, the OpenFont might fail, in which case the
  ;; invalid-font error can arise.
  (declare (type display display)
	   (type string pattern)
	   (type card16 max-fonts)
	   (type t result-type)) ;; CL type
  (declare-values (sequence font))
  (let ((string (string pattern))
	(result nil))
    (with-display (display)
      (with-buffer-request (display *x-listfontswithinfo* :no-after)
	(card16 max-fonts (length string))
	(string string))
      (loop
	;; The font info's come in seperate reply packets
	(wait-for-reply display nil)
	(reading-buffer-reply (display)
	  (buffer-input display buffer-bbuf *replysize* 60)
	  (when (zerop (card8-get 1))
	    (return))
	  (let* ((name-len (card8-get 1))
		 (min-byte2 (card16-get 40))
		 (max-byte2 (card16-get 42))
		 (min-byte1 (card8-get 49))
		 (max-byte1 (card8-get 50))
		 (min-char  min-byte2)
		 (max-char  (+ (ash max-byte1 8) max-byte2))
		 (nfont-props (card16-get 46))
		 (font
		   (make-font
		     :display display
		     :name nil
		     :font-info-internal
		     (make-font-info
		       :direction (member8-get 48 :left-to-right :right-to-left)
		       :min-char min-char
		       :max-char max-char
		       :min-byte1 min-byte1
		       :max-byte1 max-byte1
		       :min-byte2 min-byte2
		       :max-byte2 max-byte2
		       :all-chars-exist-p (boolean-get 51)
		       :default-char (card16-get 44)
		       :ascent (int16-get 52)
		       :descent (int16-get 54)
		       :min-bounds (char-info-get 8)
		       :max-bounds (char-info-get 24)
		       :properties (sequence-get :length (* 2 nfont-props)
						 :format int32
						 :result-type 'list)))))
	    (setf (font-name font) (string-get name-len))
	    (push font result)))))
    (display-invoke-after-function display)
    ;; Replace atom id's with keywords in the plist
    (dolist (font result)
      (do ((p (font-properties font) (cddr p)))
	  ((endp p))
	(setf (car p) (lookup-xatom display (car p)))))
    (coerce (nreverse result) result-type)))

#+comment ;beta protocol
(defun list-fonts (display pattern &key (max-fonts 65535) (result-type 'list))
  ;; Note: Was called list-fonts-with-info.
  ;; Returns "pseudo" fonts that contain basic font metrics and properties, but
  ;; no per-character metrics and no resource-ids.  These pseudo fonts will be
  ;; converted (internally) to real fonts dynamically as needed, by issuing an
  ;; OpenFont request.  However, the OpenFont might fail, in which case the
  ;; invalid-font error can arise.
  (declare (type display display)
	   (type string pattern)
	   (type card16 max-fonts)
	   (type t result-type)) ;; CL type
  (declare-values (sequence font))
  (let ((string (string pattern))
	result)
    (with-display (display)
      (with-buffer-request (display *x-listfontswithinfo* :no-after)
	(card16 max-fonts (length string))
	(string string))
      (reading-buffer-reply (display :sizes (8 16))
	(let ((length (- (wait-for-reply display nil) *replysize*))
	      (nfonts (card16-get 8)))
	  (setq result (read-sequence-string display length nfonts result-type))
	  ;; The font info's come in seperate reply packets
	  (dotimes (i nfonts)
	    (buffer-input display buffer-bbuf 8 60)
	    (let* ((min-byte2 (card16-get 40))
		   (max-byte2 (card16-get 42))
		   (min-byte1 (card8-get 49))
		   (max-byte1 (card8-get 50))
		   (min-char  min-byte2)
		   (max-char  (+ (ash max-byte1 8) max-byte2))
		   (nfont-props (card16-get 46))
		   (font
		     (make-font
		       :display display
		       :name (elt result i)
		       :font-info-internal
		       (make-font-info
			 :direction (member8-get 48 :left-to-right :right-to-left)
			 :min-char min-char
			 :max-char max-char
			 :min-byte1 min-byte1
			 :max-byte1 max-byte1
			 :min-byte2 min-byte2
			 :max-byte2 max-byte2
			 :all-chars-exist-p (boolean-get 51)
			 :default-char (card16-get 44)
			 :ascent (int16-get 52)
			 :descent (int16-get 54)
			 :min-bounds (char-info-get 8)
			 :max-bounds (char-info-get 24)
			 :properties (sequence-get :length (* 2 nfont-props)
						   :format int32
						   :result-type 'list)))))
	      (setf (elt result i) font))))))
      (display-invoke-after-function display)
      ;; Replace atom id's with keywords in the plist
      (dotimes (i (length result))
	(do ((p (font-properties (elt result i)) (cddr p)))
	    ((endp p))
	  (setf (car p) (lookup-xatom display (car p)))))
      result))

(defun font-path (display &key (result-type 'list))
  (declare (type display display)
	   (type t result-type)) ;; CL type
  (declare-values (sequence (or string pathname)))
  (let (result)
    (with-display (display)
      (with-buffer-request (display *x-getfontpath* :no-after))
      (reading-buffer-reply (display :sizes 16)
	(let ((length (- (wait-for-reply display nil) *replysize*))
	      (nfonts (card16-get 8)))
	  (setq result (read-sequence-string display length nfonts result-type)))))
    (display-invoke-after-function display)
    result))

(defun set-font-path (display paths)
  (declare (type display display)
	   (type sequence paths)) ;; (sequence (or string pathname))
  (let ((path-length (length paths))
	(request-length 8))
    ;; Find the request length
    (dotimes (i path-length)
      (let* ((string (string (elt paths i)))
	     (len (length string)))
	(incf request-length (1+ len))))
    (with-buffer-request (display *x-setfontpath* :length request-length)
      (length (ceiling request-length 4))
      (card16 path-length)
      (pad16 nil)
      (progn
	(incf buffer-boffset 8)
	(dotimes (i path-length)
	  (let* ((string (string (elt paths i)))
		 (len (length string)))
	    (card8-put 0 len)
	    (string-put 1 string :appending t :header-length 1)
	    (incf buffer-boffset (1+ len))))
	(setf (buffer-boffset display) (lround buffer-boffset))))))

(defsetf font-path set-font-path)
