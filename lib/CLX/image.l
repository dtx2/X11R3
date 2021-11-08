;;; -*- Mode:Lisp; Package:XLIB; Syntax:COMMON-LISP; Base:10; Lowercase:T -*-

;;; CLX Image functions

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

;;;
;;; Change history:
;;;
;;;  Date	Author	Description
;;; -------------------------------------------------------------------------------------
;;; 10/14/87	LGO	Created
;;; 11/03/87	LGO	Re-written to split out image-x image-xy and image-z types
;;;  5/18/88    DLC     Moved BYTE-REVERSE to before it was used for inline expansion

;;; TO DO:
;;; 1. Write lispm versions of the conversion functions that use BITBLT
;;; 2. Export VISUAL-INFO?  What support does XLIB give for visuals?
;;; 3. Does bit-lsb-first-p apply to z-format 4 bit-per-pixel?
;;; 4. What does byte-lsb-first-p mean for z-format 24 bit-per-pixel?
;;; 5. Why does read-bitmap-file need :bit-lsb-first-p t to create-image?
;;; 6. This hasn't been tested with depths 4 16 24 or 32 (I don't have
;;;    access to a server that supports these visuals - LGO)

(in-package 'xlib :use '(lisp))

(export '(bitmap
	  pixarray 
	  image
	  image-width
	  image-height
	  image-depth
	  image-plist
	  image-name
	  image-x-hot
	  image-y-hot
	  image-red-mask
	  image-blue-mask
	  image-green-mask
	  image-x
	  image-xy
	  image-z
	  image-x-p
	  image-xy-p
	  image-z-p
	  image-xy-bitmap-list
	  image-z-bits-per-pixel
	  image-z-pixarray
	  create-image
	  get-image
	  put-image
	  copy-image
	  read-bitmap-file
	  write-bitmap-file
	  bitmap-image
	  image-pixmap))

(deftype bitmap () '(array bit (* *)))

(deftype pixarray () '(or (array pixel (* *))
			  (array card16 (* *))
			  (array card8 (* *))
			  (array (unsigned-byte 4) (* *))
			  (array bit (* *))))

(defstruct (image (:constructor nil) (:copier nil))
  ;; Public structure
  (width 0 :type card16 :read-only t)
  (height 0 :type card16 :read-only t)
  (depth 1 :type card8 :read-only t)
  (plist nil :type list))

;; Image-Plist accessors:
(defun image-name (image) (getf (image-plist image) :name))
(defun image-x-hot (image) (getf (image-plist image) :x_hot))
(defun image-y-hot (image) (getf (image-plist image) :y_hot))
(defun image-red-mask (image) (getf (image-plist image) :red-mask))
(defun image-blue-mask (image) (getf (image-plist image) :blue-mask))
(defun image-green-mask (image) (getf (image-plist image) :green-mask))

(defsetf image-name (image) (name) `(set-image-property ,image :name ,name))
(defsetf image-x-hot (image) (x) `(set-image-property ,image :x_hot ,x))
(defsetf image-y-hot (image) (y) `(set-image-property ,image :y_hot ,y))
(defsetf image-red-mask (image) (mask) `(set-image-property ,image :red-mask ,mask))
(defsetf image-blue-mask (image) (mask) `(set-image-property ,image :blue-mask ,mask))
(defsetf image-green-mask (image) (mask) `(set-image-property ,image :green-mask ,mask))

(defun set-image-property (image name value) (setf (getf (image-plist image) name) value))

(defvar *empty-data-x* (make-sequence '(array card8 (*)) 0))
(proclaim '(type (array card8 (*)) *empty-data-x*))
(defvar *empty-data-z* (make-array '(0 0) :element-type 'bit))
(proclaim '(type pixarray *empty-data-z*))

(defstruct (image-x (:include image))
  ;; Use this format for shoveling image data
  ;; Private structure. Accessors for these NOT exported.
  (format :z-pixmap :type (member :bitmap :xy-pixmap :z-pixmap))
  (bytes-per-line 0 :type card16)
  (bits-per-pixel 1 :type (member 1 4 8 16 24 32))
  (bit-lsb-first-p nil :type boolean)		; Bit order
  (byte-lsb-first-p nil :type boolean)		; Byte order
  (data *empty-data-x* :type (array card8 (*)))); row-major

(defstruct (image-xy (:include image))
  ;; Public structure
  ;; Use this format for image processing
  (bitmap-list nil :type list)) ;; list of bitmaps

(defstruct (image-z (:include image))
  ;; Public structure
  ;; Use this format for image processing
  (bits-per-pixel 1 :type (member 1 4 8 16 24 32))
  (pixarray *empty-data-z* :type pixarray))

(defun create-image (&key width height
		     depth data plist name x-hot y-hot
		     red-mask blue-mask green-mask
		     bits-per-pixel format bytes-per-line
		     byte-lsb-first-p bit-lsb-first-p )
  ;; Returns an image-x image-xy or image-z structure, depending on the
  ;; type of the :DATA parameter.
  (declare
    (type (or null card16) width height)	; Required
    (type (or null card8) depth)		; Defualts to 1
    (type (or (array card8 (*))			;Returns image-x
	      cons ; (list bitmap)		;Returns image-xy
	      pixarray) data)			;Returns image-z
    (type list plist)
    (type (or null stringable) name)
    (type (or null card16) x-hot y-hot)
    (type (or null pixel) red-mask blue-mask green-mask)
    (type (or null (member 1 4 8 16 24 32)) bits-per-pixel)
    
    ;; The following parameters are ignored for image-xy and image-z:
    (type (or null (member :bitmap :xy-pixmap :z-pixmap))
	  format)				; defaults to :z-pixmap
    (type (or null card16) bytes-per-line)
    (type boolean byte-lsb-first-p bit-lsb-first-p))
  (declare-values image)
  (let (image)
    ;; If image is a list of one element, use image-z
    ;; (when (and (consp data) (not (cdr data))) (setq data (car data)))
    (etypecase data
      (vector					; image-x
       (unless depth (setq depth 1))
       (unless width (required-arg width))
       (unless height (required-arg height))
       (unless bytes-per-line
	 (setq bytes-per-line (floor (length data) (* (or bits-per-pixel depth) height))))
       (setq image (make-image-x :width width :height height
				 :depth depth :plist plist
				 :bits-per-pixel (or bits-per-pixel depth)
				 :format (or format (if (= depth 1) :xy-pixmap :z-pixmap))
				 :bytes-per-line bytes-per-line
				 :byte-lsb-first-p byte-lsb-first-p
				 :bit-lsb-first-p bit-lsb-first-p
				 :data data)))
      (cons					; image-xy
       (unless width (setq width (array-dimension (car data) 1)))
       (unless height (setq height (array-dimension (car data) 0)))
       (setq image (make-image-xy :width width :height height :plist plist
				  :depth (or depth (length image))
				  :bitmap-list data)))
      
      (pixarray					; image-z
       (unless width (setq width (array-dimension data 1)))
       (unless height (setq height (array-dimension data 0)))
       (unless depth (setq depth (pixarray-depth data)))
       (setq bits-per-pixel (ash 1 (integer-length (1- (or bits-per-pixel depth))))) ;; round up to power of 2
       (setq image (make-image-z :width width :height height
				:depth depth :plist plist
				:bits-per-pixel bits-per-pixel
				:pixarray data))))
    (when name (setf (image-name image) name))
    (when x-hot (setf (image-x-hot image) x-hot))
    (when y-hot (setf (image-y-hot image) y-hot))
    (when red-mask (setf (image-red-mask image) red-mask))
    (when blue-mask (setf (image-blue-mask image) blue-mask))
    (when green-mask (setf (image-green-mask image) green-mask))
    image))

(defun scanline-byte-round (scanline-length scanline-pad)
  (ecase scanline-pad
    (8 scanline-length)
    (16 (wround scanline-length))
    (32 (lround scanline-length))))

(defun pixarray-depth (pixarray)
  (or (second (assoc (array-element-type pixarray)
		     '((bit 1)
		       ((mod 4) 2)
		       ((mod 16) 4)
		       ((mod 256) 8)
		       ((mod #x10000) 16)
		       ((mod #x100000000) 32))
		     :test #'subtypep))
      (x-type-error pixarray 'pixarray)))


;;;-----------------------------------------------------------------------------
;;; GET-IMAGE

;; Should this be exported?
(defun visual-info (display visual-id)
  (dolist (screen (display-roots display))
    (dolist (vis (screen-depths screen))
      (dolist (visual-info (cdr vis))
	(when (= visual-id (visual-info-id visual-info))
	  (return-from visual-info visual-info)))))
  (error "Visual info not found for id #x~x in display ~s" visual-id display))

(defun get-image (drawable &key 
		  (x (required-arg x))
		  (y (required-arg y))
		  (width (required-arg width))
		  (height (required-arg height))
		  plane-mask format result-type)
  ;; Get an image from the server.
  ;; Result-Type defaults from Format, image-z for :z-pixmap, image-xy
  ;; for :xy-pixmap and image-x when unspecified.
  ;; Format defaults from result-type: :sy-pixmap for imagexy, :z-pixmap
  ;; for image-z, or when unspecified.
  ;; Plane-mask defaults to #xFFFFFFFF.
  ;; Returns an image-x image-xy or image-z structure, depending on the
  ;; result-type parameter.
  (declare (type drawable drawable)
	   (type int16 x y) ;; required
	   (type card16 width height) ;; required
	   (type (or null pixel) plane-mask)
	   (type (or null (member :xy-pixmap :z-pixmap)) format)
	   (type (or null (member image-x image-xy image-z)) result-type))
  (declare-values image visual-id)
  (unless result-type
    (setq result-type (case format
			(:xy-pixmap 'image-xy)
			(:z-pixmap 'image-z)
			((nil) 'image-x))))
  (unless format
    (setq format (case result-type
		   (image-xy :xy-pixmap)
		   ((image-z image-x) :z-pixmap))))
  (unless (ecase result-type
	    (image-xy (eq format :xy-pixmap))
	    (image-z (eq format :z-pixmap))
	    (image-x t))
    (error "Result-type ~s is incompatable with format ~s"
	   result-type format))
  (multiple-value-bind (data depth visual-id)
      (get-raw-image drawable :x x :y y :width width :height height
		     :plane-mask (or plane-mask #xffffffff) :format format)
    (let* ((display (drawable-display drawable))
	   (bitmap-format (display-bitmap-format display))
	   (scanline-pad (bitmap-format-pad bitmap-format))
	   (bit-lsb-first-p (bitmap-format-lsb-first-p bitmap-format))
	   (byte-lsb-first-p (display-image-lsb-first-p display))
	   (bits-per-pixel depth)
	   (bytes-per-line 0))
      (if (= depth 1)
	  (setq format :xy-pixmap)
	(let ((pixmap-format (find depth (display-pixmap-formats display)
				   :key #'pixmap-format-depth)))
	  (unless pixmap-format ;; Should never happen
	    (error "Display doesn't support pixmaps of depth ~d" depth))
	  (setq bits-per-pixel (pixmap-format-bits-per-pixel pixmap-format))
	  (when (eq format :z-pixmap)
	    (setq scanline-pad (pixmap-format-scanline-pad pixmap-format)))))
      (if (eq format :z-pixmap)
	  (let ((scanline-length (ceiling (* width depth) 8)))
	    (setq bytes-per-line (scanline-byte-round scanline-length scanline-pad)))
	(let ((scanline-length (ceiling width 8)))
	  (setq bytes-per-line (scanline-byte-round scanline-length scanline-pad))))
      ;; Convert image to the format needed for pixarray transformation
      (unless (eq result-type 'image-x)
	(when byte-lsb-first-p
	  (byte-swap-vector data 0 (length data) scanline-pad))
	(when (and bit-lsb-first-p (= depth 1))
	  (bit-reverse-vector data 0 (length data))))
      (let ((image
	      (ecase result-type
		(image-x
		 (create-image :width width :height height :format format
			       :depth depth :data data
			       :bits-per-pixel bits-per-pixel
			       :bytes-per-line bytes-per-line
			       :byte-lsb-first-p byte-lsb-first-p
			       :bit-lsb-first-p bit-lsb-first-p
			       ))
		(image-xy
		 (do ((plane 0 (1+ plane))
		      (bytes-per-plane (* bytes-per-line height))
		      (start 0 (+ start bytes-per-plane))
		      (end (length data))
		      (result nil))
		     ((or (>= plane depth)
			  (>= start end))
		      (create-image :width width :height height
				    :depth plane :data (nreverse result)
				    ))
		   (push (z-format-pixarray data start bytes-per-line 1 1
					    0 0 width height)
			 result)))
		(image-z
		 (let ((pixarray (z-format-pixarray data 0 bytes-per-line depth bits-per-pixel
						    0 0 width height)))
		   (create-image :width width :height height
				 :depth depth :data pixarray
				 ))))))
	(when (plusp visual-id)
	  (let ((visual-info (visual-info display visual-id)))
	    (setf (image-red-mask image) (visual-info-red-mask visual-info))
	    (setf (image-green-mask image) (visual-info-green-mask visual-info))
	    (setf (image-blue-mask image) (visual-info-blue-mask visual-info))))
	(values image visual-id)))))


;;;-----------------------------------------------------------------------------
;;; Pixel-Array conversions

#+comment ;; Used to generate the table in byte-reverse
(defun genbyte ()
  (let ((result (make-array 256)))
    (dotimes (i 256)
      (let ((b 0))
	(setq b (dpb (ldb (byte 1 0) i) (byte 1 7) b))
	(setq b (dpb (ldb (byte 1 1) i) (byte 1 6) b))
	(setq b (dpb (ldb (byte 1 2) i) (byte 1 5) b))
	(setq b (dpb (ldb (byte 1 3) i) (byte 1 4) b))
	(setq b (dpb (ldb (byte 1 4) i) (byte 1 3) b))
	(setq b (dpb (ldb (byte 1 5) i) (byte 1 2) b))
	(setq b (dpb (ldb (byte 1 6) i) (byte 1 1) b))
	(setq b (dpb (ldb (byte 1 7) i) (byte 1 0) b))
	(setf (aref result i) b)))
    result))

(proclaim '(inline byte-reverse))
(defun byte-reverse (byte)
  (aref '#.(coerce
	    '#(0 128 64 192 32 160 96 224 16 144 80 208 48 176 112 240
	       8 136 72 200 40 168 104 232 24 152 88 216 56 184 120 248
	       4 132 68 196 36 164 100 228 20 148 84 212 52 180 116 244
	       12 140 76 204 44 172 108 236 28 156 92 220 60 188 124 252
	       2 130 66 194 34 162 98 226 18 146 82 210 50 178 114 242
	       10 138 74 202 42 170 106 234 26 154 90 218 58 186 122 250
	       6 134 70 198 38 166 102 230 22 150 86 214 54 182 118 246
	       14 142 78 206 46 174 110 238 30 158 94 222 62 190 126 254
	       1 129 65 193 33 161 97 225 17 145 81 209 49 177 113 241
	       9 137 73 201 41 169 105 233 25 153 89 217 57 185 121 249
	       5 133 69 197 37 165 101 229 21 149 85 213 53 181 117 245
	       13 141 77 205 45 173 109 237 29 157 93 221 61 189 125 253
	       3 131 67 195 35 163 99 227 19 147 83 211 51 179 115 243
	       11 139 75 203 43 171 107 235 27 155 91 219 59 187 123 251
	       7 135 71 199 39 167 103 231 23 151 87 215 55 183 119 247
	       15 143 79 207 47 175 111 239 31 159 95 223 63 191 127 255)
	     '(vector card8))
	byte))


(defun z-format-pixarray (data index bytes-per-line depth bits-per-pixel
				src-x src-y width height)
  (declare (type (simple-array card8 (*)) data)
	   (type array-index index)
	   (type card16 bytes-per-line)
	   (type card8 depth)
	   (type (member 1 4 8 16 24 32) bits-per-pixel)
	   (type card16 src-x src-y width height))
  (let* ((row src-y)
	 (copy-pixarray-row (cdr (assoc bits-per-pixel
					'((1 . z-format-row-1)
					  (4 . z-format-row-4)
					  (8 . z-format-row-8)
					  (16 . z-format-row-16)
					  (24 . z-format-row-24)
					  (32 . z-format-row-32)))))
	 (pixarray (make-array (list height width) :element-type `(unsigned-byte ,depth))))
    (declare (type array-index row)
	     (type pixarray pixarray))
    (dotimes (i height)
      ;; Copy scanline
      (funcall copy-pixarray-row data index pixarray row src-x width)
      (index-incf row)
      (index-incf index bytes-per-line))
    pixarray))

(defun z-format-row-1 (data index pixarray row src-x width)
  ;; Copy 1 bit-per-pixel pixels from data to pixarray
  (declare (type (simple-array card8 (*)) data)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (data (simple-array card8 (*)))
    (let* ((col 0)
	   (index (+ index (ash src-x -3))) ;; starting index
	   (start-pixels (logand src-x 7))
	   (middle-bytes (ash (- width start-pixels) -3))
	   (end-pixels (- width start-pixels (ash middle-bytes 3))))
      (declare (type array-index col index start-pixels middle-bytes end-pixels))
      ;; copy partial byte at start
      (when (plusp start-pixels)
	(let ((byte (byte-reverse (aref data index))))
	  (declare (type card8 byte))
	  (index-incf index)
	  (dotimes (i start-pixels)
	    (setf (aref pixarray row i) (logand byte 1))
	    (setq byte (ash byte -1)))
	  (index-incf col start-pixels)))
      ;; Copy whole bytes in middle
      (dotimes (b middle-bytes)
	(let ((byte (byte-reverse (aref data index))))
	  (declare (type card8 byte))
	  (index-incf index)
	  (setf (aref pixarray row col) (logand byte 1))
	  (setf (aref pixarray row (incf col)) (logand (setq byte (ash byte -1)) 1))
	  (setf (aref pixarray row (incf col)) (logand (setq byte (ash byte -1)) 1))
	  (setf (aref pixarray row (incf col)) (logand (setq byte (ash byte -1)) 1))
	  (setf (aref pixarray row (incf col)) (logand (setq byte (ash byte -1)) 1))
	  (setf (aref pixarray row (incf col)) (logand (setq byte (ash byte -1)) 1))
	  (setf (aref pixarray row (incf col)) (logand (setq byte (ash byte -1)) 1))
	  (setf (aref pixarray row (incf col)) (logand (setq byte (ash byte -1)) 1))
	  (incf col)))
      ;; Copy partial byte at end
      (when (plusp end-pixels)
	(let ((byte (byte-reverse (aref data index))))
	  (declare (type card8 byte))
	  (index-incf index)
	  (dotimes (i end-pixels)
	    (setf (aref pixarray row (+ i col)) (logand byte 1))
	    (setq byte (ash byte -1))))))))

(defun z-format-row-4 (data index pixarray row src-x width)
  ;; Copy 1 bit-per-pixel pixels from data to pixarray
  (declare (type (simple-array card8 (*)) data)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (data (simple-array card8 (*)))
    (let* ((col 0)
	   (index (+ index (ash src-x -1))) ;; starting index
	   (start-pixels (logand src-x 1))
	   (middle-bytes (ash (- width start-pixels) -1))
	   (end-pixels (- width start-pixels (ash middle-bytes 1))))
      (declare (type array-index col index start-pixels middle-bytes end-pixels))
      ;; copy partial byte at start
      (when (plusp start-pixels)
	(let ((byte (aref data index)))
	  (declare (type card8 byte))
	  (index-incf index)
	  (setf (aref pixarray row col) (logand byte 15))
	  (index-incf col)))
      ;; Copy whole bytes in middle
      (dotimes (b middle-bytes)
	(let ((byte (aref data index)))
	  (declare (type card8 byte))
	  (index-incf index)
	  (setf (aref pixarray row col) (ash byte -4))
	  (setf (aref pixarray row (incf col)) (logand byte 15))
	  (incf col)))
      ;; Copy partial byte at end
      (when (plusp end-pixels)
	(let ((byte (aref data index)))
	  (declare (type card8 byte))
	  (index-incf index)
	  (setf (aref pixarray row col) (ash byte -4)))))))

(defun z-format-row-8 (data index pixarray row src-x width)
  ;; Copy 1 bit-per-pixel pixels from data to pixarray
  (declare (type (simple-array card8 (*)) data)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (data (simple-array card8 (*)))
    (let* ((index (+ index src-x))) ;; starting index
      (declare (type array-index index))
      (dotimes (col width)
	(setf (aref pixarray row col) (aref data index))
	(index-incf index)))))

(defun z-format-row-16 (data index pixarray row src-x width)
  ;; Copy 1 bit-per-pixel pixels from data to pixarray
  (declare (type (simple-array card8 (*)) data)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (data (simple-array card8 (*)))
    (let* ((index (+ index src-x))) ;; starting index
      (declare (type array-index index))
      (dotimes (col width)
	(setf (aref pixarray row col)
	      (dpb (aref data index) (byte 8 8) (aref data (index-incf index))))
	(index-incf index)))))

(defun z-format-row-24 (data index pixarray row src-x width)
  ;; Copy 1 bit-per-pixel pixels from data to pixarray
  (declare (type (simple-array card8 (*)) data)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (data (simple-array card8 (*)))
    (let* ((index (+ index src-x))) ;; starting index
      (declare (type array-index index))
      (dotimes (col width)
	(setf (aref pixarray row col)
	      (dpb (aref data index) (byte 8 16)
		   (dpb (aref data (index-incf index)) (byte 8 8)
			(aref data (index-incf index)))))
	(index-incf index)))))

(defun z-format-row-32 (data index pixarray row src-x width)
  ;; Copy 1 bit-per-pixel pixels from data to pixarray
  (declare (type (simple-array card8 (*)) data)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (data (simple-array card8 (*)))
    (let* ((index (+ index src-x))) ;; starting index
      (declare (type array-index index))
      (dotimes (col width)
	(setf (aref pixarray row col)
	      (dpb (aref data index) (byte 8 24)
		   (dpb (aref data (index-incf index)) (byte 8 16)
			(dpb (aref data (index-incf index)) (byte 8 8)
			     (aref data (index-incf index))))))
	(index-incf index)))))

#+comment ;; not used
(defun xy-format-image-pixarray (data index width height depth bytes-per-line)
  ;; Extract a pixarray from an xy-pixmap data vector
  (declare (type card16 height depth bytes-per-line))
  (let* ((end-byte (floor width 8))
	 (right-pad (rem width 8))
	 (width8 end-byte)
	 (plane-length (* height bytes-per-line))
	 (pixarray (make-array (list height width) :element-type `(unsigned-byte ,depth)))
	 (plane-start 0)
	 (start index)
	 (nbyte 0)
	 (col 0))
    (declare (type card16 end-byte right-pad width8)
	     (type array-index start plane-length plane-start nbyte col))
    (with-vector (data (simple-array card8 (*)))
      (do ((shift 0 (1+ shift))
	   (mask 1 (ash mask 1)))
	  ((>= shift depth))
	(declare (type card16 shift mask))
	(dotimes (row height pixarray)
	  (setq col 0)
	  ;; Copy full bytes in center
	  (dotimes (i width8)
	    (do ((sbit 0 (1+ sbit))
		 (byte (ash (byte-reverse (aref data nbyte)) shift)
		       (ash byte -1))) ;; Reverse to make shifting easier
		((>= sbit 8))
	      (declare (type card16 sbit byte))
	      (setf (aref pixarray row col)
		    (logior (aref pixarray row col) (logand mask byte)))
	      (incf col))
	    (incf nbyte))
	  ;; Copy partial byte at end
	  (when (plusp right-pad)
	    (do ((sbit 0 (1+ sbit))
		 (byte (ash (byte-reverse (aref data nbyte)) shift)
		       (ash byte -1))) ;; Reverse to make shifting easier
		((>= sbit right-pad))
	      (declare (type card16 sbit byte))
	      (setf (aref pixarray row col)
		    (logior (aref pixarray row col) (logand mask byte)))
	      (incf col)))
	  (incf start bytes-per-line)
	  (setq nbyte start))
	(incf plane-start plane-length)
	(setq start plane-start))
      pixarray)))

;; Before an image can be byte-swapped, each scanline must be padded
;; out to a multiple of the scanline pad.
(defun convert-image-scanline-pad (image scanline-pad)
  (declare (type image-x image)
	   (type (member 8 16 32) scanline-pad))
  (let* ((width (image-width image))
	 (height (image-height image))
	 (data (image-x-data image))
	 (depth (image-depth image))
	 (scanline-length (ceiling (* width depth) 8))
	 (image-length (scanline-byte-round scanline-length scanline-pad))
	 (pad (- image-length scanline-length))
	 (sbyte 0)
	 (dbyte 0))
    (unless (zerop pad)
      (with-vector (data (simple-array card8 (*)))
	(let* ((length (* image-length height))
	       (result (if (<= length (length data)) data
			 (make-array length :element-type '(unsigned-byte 8)))))
	  (with-vector (result (simple-array card8 (*)))
	    (setf (image-x-data image) result)
	    (dotimes (i height)
	      (dotimes (j scanline-length)
		(setf (aref result dbyte) (aref data sbyte))
		(incf dbyte)
		(incf sbyte))
	      (incf dbyte pad))))))))

(defun byte-swap-image (image scanline-pad)
  ;; Swap bytes
  (declare (type image-x image))
  (let ((data (image-x-data image)))
    (setf (image-x-byte-lsb-first-p image) (not (image-x-byte-lsb-first-p image)))
    (byte-swap-vector data 0 (length data) scanline-pad)))

(defun byte-swap-vector (data start nbytes scanline-pad)
  (let ((sbyte start)
	(dbyte start))
    (with-vector (data (simple-array card8 (*)))
      (ecase scanline-pad
	(8 nil) ;; Nothing to swap
	(16					; Swap 2 bytes
	 (dotimes (j (floor nbytes 2))
	   (let ((temp (aref (aref data sbyte))))
	     (setf (aref data dbyte) (aref data (incf sbyte)))
	     (setf (aref data (incf dbyte)) temp))
	   (incf dbyte)
	   (incf sbyte)))
	(24					; Swap 3 bytes
	 (dotimes (j (floor nbytes 3))
	   (let ((temp1 (aref (aref data sbyte)))
		 (temp2 (aref data (incf sbyte))))
	     (setf (aref data dbyte) (aref data (incf sbyte)))
	     (setf (aref data (incf dbyte)) temp2)
	     (setf (aref data (incf dbyte)) temp1))
	   (incf dbyte)
	   (incf sbyte)))
	(32					; Swap word
	 (dotimes (j (floor nbytes 4))
	   (let ((temp1 (aref data sbyte))
		 (temp2 (aref data (incf sbyte)))
		 (temp3 (aref data (incf sbyte))))
	     (setf (aref data dbyte) (aref data (incf sbyte)))
	     (setf (aref data (incf dbyte)) temp3)
	     (setf (aref data (incf dbyte)) temp2)
	     (setf (aref data (incf dbyte)) temp1))
	   (incf dbyte)
	   (incf sbyte)))))
    sbyte))

(defun bit-reverse-image (image)
  (let ((data (image-x-data image)))
    (bit-reverse-vector data 0 (length data)))
  (setf (image-x-bit-lsb-first-p image)
	(not (image-x-bit-lsb-first-p image))))

(defun bit-reverse-vector (data start nbytes)
  (declare (type (simple-array card8 (*)) data)
	   (type array-index start nbytes))
  (let* ((index start))
    (declare (type array-index index))
    (with-vector (data (simple-array card8 (*)))
      (dotimes (i nbytes)
	(setf (aref data index) (byte-reverse (aref data index)))
	(index-incf index)))))



;;;-----------------------------------------------------------------------------
;;; PUT-IMAGE

;;; Note:	The only difference between a format of :bitmap and :xy-pixmap
;;;		of depth 1 is that when sending a :bitmap format the foreground 
;;;		and background in the gcontext are used.

(defun put-image (drawable gcontext image &rest options &key
		  (src-x 0) (src-y 0)		;Position within image
		  (x (required-arg x))		;Position within drawable
		  (y (required-arg y))
		  width height
		  bitmap-p)
  ;; Copy an image into a drawable.
  ;; WIDTH and HEIGHT default from IMAGE.
  ;; When BITMAP-P, force format to be :bitmap when depth=1.
  ;; This causes gcontext to supply foreground & background pixels.
  (declare (type drawable drawable)
	   (type gcontext gcontext)
	   (type image image)
	   (type int16 x y) ;; required
	   (type (or null card16) width height)
	   (type boolean bitmap-p))
  (let* ((image-width (image-width image))
	 (image-height (image-height image))
	 (width (min (or width image-width) (- image-width src-x)))
	 (height (min (or height image-height) (- image-height src-y)))
	 (depth (image-depth image))
	 (display (drawable-display drawable))
	 (bitmap-format (display-bitmap-format display))
	 (scanline-pad (bitmap-format-pad bitmap-format))
	 pixmap-format)
    (declare (type card16 image-width image-height width height depth scanline-pad))
      (if bitmap-p
	  (unless (= depth 1) (error "Bitmaps must have depth 1"))
	(progn
	  (setq pixmap-format (find depth (display-pixmap-formats display)
				    :key #'pixmap-format-depth))
	  (unless pixmap-format
	    (error "Display doesn't support pixmaps of depth ~d" depth))))
    (etypecase image
      (image-x
       (when (eq (image-x-format image) :z-pixmap)
	 (setq scanline-pad (pixmap-format-scanline-pad pixmap-format)))
       (put-image-x drawable gcontext image src-x src-y x y width height bitmap-p scanline-pad))
      (image-xy (put-image-xy drawable gcontext image src-x src-y x y width height bitmap-p scanline-pad))
      (image-z
       (when pixmap-format
	 (setq scanline-pad (pixmap-format-scanline-pad pixmap-format)))
       (put-image-z drawable gcontext image src-x src-y x y width height bitmap-p scanline-pad)))))
;;
;; PUT X-IMAGE
;;
(defun put-image-x (drawable gcontext image src-x src-y x y width height bitmap-p scanline-pad)
  ;; Send an X-Format image to the server
  ;; When BITMAP-P, force format to be :bitmap when depth=1
  ;; This causes gcontext to supply foreground & background pixels.
  (declare (type drawable drawable)
	   (type gcontext gcontext)
	   (type image-x image)
	   (type card16 src-x src-y)
	   (type int16 x y)
	   (type card16 width height)
	   (type boolean bitmap-p))
  (let* ((display (drawable-display drawable))
	 (format (image-x-format image))
	 (depth (image-depth image))
	 (bitmap-format (display-bitmap-format display))
	 (bit-lsb-first-p (bitmap-format-lsb-first-p bitmap-format))
	 (byte-lsb-first-p (display-image-lsb-first-p display)))
    (when bitmap-p
      (setq format :bitmap))
    ;; Convert image to the format needed by the display
    (when (not (eq (and byte-lsb-first-p t) (and (image-x-byte-lsb-first-p image) t)))
      (unless (zerop (rem (image-x-bytes-per-line image) (floor scanline-pad 8)))
	(convert-image-scanline-pad image scanline-pad))
      (byte-swap-image image scanline-pad))
    (when (and (= depth 1)
	       (not (eq (and bit-lsb-first-p t) (and (image-x-bit-lsb-first-p image) t))))
      (bit-reverse-image image))
    (put-image-x-internal drawable gcontext image src-x src-y x y width height format scanline-pad)))

(defun put-image-x-internal (drawable gcontext image src-x src-y x y width height format scanline-pad)
  ;; Send an X-Format image to the server after all image conversion has been done.
  (declare (type drawable drawable)
	   (type gcontext gcontext)
	   (type image-x image)
	   (type int16 x y) ;; required
	   (type card16 src-x src-y)
	   (type card16 width height)
	   (type (member :bitmap :xy-pixmap :z-pixmap) format)
	   (type (member 8 16 32) scanline-pad))
  ;; Geometry calculations      
  (let* ((display (drawable-display drawable))
	 (depth (image-depth image)))
    (declare (type display display)
	     (type card8 depth))
    ;; Send image to the display
    (with-buffer-request (display *x-putimage* :gc-force gcontext)
      ((data (member :bitmap :xy-pixmap :z-pixmap)) format)
      (drawable drawable)
      (gcontext gcontext)
      (card16 width height)
      (int16 x y)
      (card8 0 depth)				;left-pad, depth
      (pad16 nil)
      (progn ;; Need seperate copy functions for XY and Z formats, because
	     ;; the sub-image extraction has to work differently.
	(if (eq format :z-pixmap)
	    (buffer-put-image display buffer-boffset image src-x src-y width height
			      scanline-pad (image-x-bits-per-pixel image) 1)
	  (buffer-put-image display buffer-boffset image src-x src-y width height
			    scanline-pad 1 depth))))))

(defun buffer-put-image (buffer boffset image src-x src-y width height scanline-pad bits-per-pixel nplanes)
  ;; copy an X-Format Z-pixmap image into the buffer
  (declare (type buffer buffer)
	   (type array-index boffset)
	   (type image-x image)
	   (type card16 src-x src-y width height)
	   (type (member 8 16 32) scanline-pad)
	   (type (member 1 4 8 16 24 32) bits-per-pixel)
	   (type card8 nplanes))
  (let* ((start-bit (* src-x bits-per-pixel))
	 (start-byte (floor start-bit 8))
	 (end-byte (ceiling (+ start-bit (* width bits-per-pixel)) 8))
	 (scanline-length (scanline-byte-round (- end-byte start-byte) scanline-pad))
	 (length (* height scanline-length nplanes))
	 (size (buffer-size buffer))
	 (data (image-x-data image))
	 (image-x-bytes-per-line (image-x-bytes-per-line image))
	 (start (* src-y image-x-bytes-per-line))
	 (plane-length (+ (* (image-height image) image-x-bytes-per-line) start))
	 (plane-start start))
    (declare (type array-index start-bit start-byte end-byte scanline-length length
		   size image-x-bytes-per-line start)
	     (type (simple-array card8 (*)) data))
    (with-buffer-output (buffer :index boffset :sizes (8 16))
      (card16-put 2 (ceiling (+ 24 length) 4)) ;; Set length
      (incf boffset 24)
      ;; Note: The server doesn't handle left-pad for z-format, and neither do we.
      (dotimes (i nplanes)
	(setq start plane-start)
	(do ((nrows 0)
	     (rows height (- rows nrows)))
	    ((not (plusp rows)))
	  (declare (type fixnum nrows rows))
	  (setq nrows (floor (- size boffset) scanline-length))
	  (when (zerop nrows)
	    ;; Flush buffer when necessary
	    (setf (buffer-boffset buffer) boffset)
	    (buffer-flush buffer)
	    (setq boffset (buffer-boffset buffer))
	    (setq nrows (floor (- size boffset) scanline-length)))	  
	  ;; Copy scanlines
	  (dotimes (r (min rows nrows))
	    (buffer-replace buffer-bbuf data boffset (incf boffset scanline-length)
			    (+ start start-byte))
	    (incf start image-x-bytes-per-line)))
	(incf plane-start plane-length))
      (setf (buffer-boffset buffer) (lround boffset)))))
;;
;; PUT Z-IMAGE
;;
(defun put-image-z (drawable gcontext image src-x src-y x y width height bitmap-p scanline-pad)
  ;; Send a Z-Format image to the server
  ;; When BITMAP-P, force format to be :bitmap when depth=1
  ;; This causes gcontext to supply foreground & background pixels.
  (declare (type drawable drawable)
	   (type gcontext gcontext)
	   (type image-z image)
	   (type card16 src-x src-y)
	   (type int16 x y)
	   (type card16 width height)
	   (type boolean bitmap-p))
  ;; Geometry calculations      
  (let* ((display (drawable-display drawable))
	 (depth (image-z-bits-per-pixel image))
	 (image-width (image-width image))
	 (image-height (image-height image))
	 (width (min (or width image-width) (- image-width src-x)))
	 (height (min (or height image-height) (- image-height src-y)))
	 (format :z-pixmap)
	 (pixarray (image-z-pixarray image))
	 (bits-per-pixel (image-z-bits-per-pixel image)))
    (declare (type display display)
	     (type card8 depth)
	     (type card16 image-width image-height width height)
	     (type pixarray pixarray)
	     (type (member 1 4 8 16 24 32) bits-per-pixel))
    (when bitmap-p
      (setq format :bitmap)
      (unless (= depth 1) (error "Bitmaps must have depth 1")))
    ;; Send image to the display
    (with-buffer-request (display *x-putimage* :gc-force gcontext)
      ((data (member :bitmap :xy-pixmap :z-pixmap)) format)
      (drawable drawable)
      (gcontext gcontext)
      (card16 width height)
      (int16 x y)
      (card8 0 depth)				;left-pad, depth
      (pad16 nil)
      (progn
	(buffer-put-pixarray display buffer-boffset pixarray bits-per-pixel src-x src-y width height scanline-pad)))))

(defun buffer-put-pixarray (display boffset pixarray bits-per-pixel src-x src-y width height scanline-pad)
  (declare (type display display)
	   (type array-index boffset)
	   (type pixarray pixarray)
	   (type (member 1 4 8 16 24 32) bits-per-pixel)
	   (type card16 src-x src-y width height))
  (let* ((row src-y)
	 (copy-pixarray-row (cdr (assoc bits-per-pixel
					'((1 . copy-pixarray-row-1)
					  (4 . copy-pixarray-row-4)
					  (8 . copy-pixarray-row-8)
					  (16 . copy-pixarray-row-16)
					  (24 . copy-pixarray-row-24)
					  (32 . copy-pixarray-row-32)))))
	 (bytes-per-line (ceiling (* width bits-per-pixel) 8))
	 (scanline-length (scanline-byte-round bytes-per-line scanline-pad))
	 (length (* height scanline-length))
	 (size (buffer-size display))
	 (bitmap-format (display-bitmap-format display))
	 (bit-lsb-first-p (bitmap-format-lsb-first-p bitmap-format))
	 (byte-lsb-first-p (display-image-lsb-first-p display)))
    (declare (type array-index row bytes-per-line scanline-length length size))
    (with-buffer-output (display :index boffset :sizes (8 16))
      (card16-put 2 (ceiling (index+ 24 length) 4)) ;; Set length
      (index-incf boffset 24)
      (dotimes (i height)
	;; Flush buffer when necessary
	(when (>= (+ boffset scanline-length) size)
	  (setf (buffer-boffset display) boffset)
	  (buffer-flush display)
	  (setq boffset (buffer-boffset display)))
	;; Copy scanline
	(funcall copy-pixarray-row buffer-bbuf boffset pixarray row src-x width)
	;; Swap bytes and bits when needed
	(ecase bits-per-pixel
	  ((1)
	   (when bit-lsb-first-p
	     (bit-reverse-vector buffer-bbuf boffset scanline-length)))
	  ((4)
	   (when byte-lsb-first-p
	     (error "Need to swap the nibbles.")))
	  ((8))
	  ((16 24 32)
	   (when byte-lsb-first-p
	     (byte-swap-vector buffer-bbuf boffset scanline-length bits-per-pixel))))
	(index-incf row)
	(index-incf boffset scanline-length))
      (setf (buffer-boffset display) (lround boffset)))))

(defun copy-pixarray-row-1 (buffer index pixarray row src-x width)
  ;; Copy a row of 1 bit-per-pixel pixels from from PIXARRAY to BUFFER
  (declare (type (simple-array card8 (*)) buffer)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (buffer (simple-array card8 (*)))
    (let ((x (index- src-x 1))
	  (index index)
	  (end-bits (logand width 7))) ;; (REM width 8)
      (declare (type array-index x index end-bits))
      (dotimes (i (ash width -3)) ;; (FLOOR width 8)
	(setf (aref buffer index)
	      (logior (ash (aref pixarray row (index-incf x)) 7)
		      (ash (aref pixarray row (index-incf x)) 6)
		      (ash (aref pixarray row (index-incf x)) 5)
		      (ash (aref pixarray row (index-incf x)) 4)
		      (ash (aref pixarray row (index-incf x)) 3)
		      (ash (aref pixarray row (index-incf x)) 2)
		      (ash (aref pixarray row (index-incf x)) 1)
		      (aref pixarray row (index-incf x))))
	(index-incf index))
      (when (plusp end-bits)
	(let ((byte 0))
	  (declare (type card8 byte))
	  (dotimes (i end-bits)
	    (setq byte (logior (ash byte 1) (aref pixarray row (index-incf x)))))
	  (setf (aref buffer index) (ash byte (- 8 end-bits))))))))

(defun copy-pixarray-row-4 (buffer index pixarray row src-x width)
  ;; Copy a row of 4 bit-per-pixel pixels from from PIXARRAY to BUFFER
  (declare (type (simple-array card8 (*)) buffer)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (buffer (simple-array card8 (*)))
    (let ((x (index- src-x 1))
	  (index index))
      (declare (type array-index x index))
      (dotimes (i (ash width -1)) ;; (FLOOR width 2)
	(setf (aref buffer index)
	      (logior (ash (aref pixarray row (index-incf x)) 4)
		      (aref pixarray row (index-incf x))))
	(index-incf index))
      (when (oddp width)
	(setf (aref buffer index) (ash (aref pixarray row (index-incf x)) 4))))))

(defun copy-pixarray-row-8 (buffer index pixarray row src-x width)
  ;; Copy a row of 8 bit-per-pixel pixels from from PIXARRAY to BUFFER
  (declare (type (simple-array card8 (*)) buffer)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (buffer (simple-array card8 (*)))
    (let ((x (index- src-x 1))
	  (index index))
      (declare (type array-index x index))
      (dotimes (i width)
	(setf (aref buffer index)
	      (aref pixarray row (index-incf x)))
	(index-incf index)))))

(defun copy-pixarray-row-16 (buffer index pixarray row src-x width)
  ;; Copy a row of 16 bit-per-pixel pixels from from PIXARRAY to BUFFER
  (declare (type (simple-array card8 (*)) buffer)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (buffer (simple-array card8 (*)))
    (let ((x (index- src-x 1))
	  (index index))
      (declare (type array-index x index))
      (dotimes (i width)
	(let ((pixel (aref pixarray row (index-incf x))))
	  (setf (aref buffer index) (ldb (byte 8 8) pixel))
	  (index-incf index)
	  (setf (aref buffer index) pixel)
	  (index-incf index))))))

(defun copy-pixarray-row-24 (buffer index pixarray row src-x width)
  ;; Copy a row of 16 bit-per-pixel pixels from from PIXARRAY to BUFFER
  (declare (type (simple-array card8 (*)) buffer)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (buffer (simple-array card8 (*)))
    (let ((x (index- src-x 1))
	  (index index))
      (declare (type array-index x index))
      (dotimes (i width)
	(let ((pixel (aref pixarray row (index-incf x))))
	  (setf (aref buffer index) (ldb (byte 8 16) pixel))
	  (index-incf index)
	  (setf (aref buffer index) (ldb (byte 8 8) pixel))
	  (index-incf index)
	  (setf (aref buffer index) pixel)
	  (index-incf index))))))

(defun copy-pixarray-row-32 (buffer index pixarray row src-x width)
  ;; Copy a row of 16 bit-per-pixel pixels from from PIXARRAY to BUFFER
  (declare (type (simple-array card8 (*)) buffer)
	   (type array-index index row src-x width)
	   (type pixarray pixarray))
  (with-vector (buffer (simple-array card8 (*)))
    (let ((x (index- src-x 1))
	  (index index))
      (declare (type array-index x index))
      (dotimes (i width)
	(let ((pixel (aref pixarray row (index-incf x))))
	  (setf (aref buffer index) (ldb (byte 8 24) pixel))
	  (index-incf index)
	  (setf (aref buffer index) (ldb (byte 8 16) pixel))
	  (index-incf index)
	  (setf (aref buffer index) (ldb (byte 8 8) pixel))
	  (index-incf index)
	  (setf (aref buffer index) pixel)
	  (index-incf index))))))
;;
;; PUT XY-IMAGE
;;
(defun put-image-xy (drawable gcontext image src-x src-y x y width height bitmap-p scanline-pad)
  ;; Send an XY-Format image to the server
  ;; When BITMAP-P, force format to be :bitmap when depth=1
  ;; This causes gcontext to supply foreground & background pixels.
  (declare (type drawable drawable)
	   (type gcontext gcontext)
	   (type image-xy image)
	   (type card16 src-x src-y)
	   (type int16 x y)
	   (type card16 width height)
	   (type boolean bitmap-p))
  ;; Geometry calculations      
  (let* ((display (drawable-display drawable))
	 (depth (image-depth image))
	 (image-width (image-width image))
	 (image-height (image-height image))
	 (width (min (or width image-width) (- image-width src-x)))
	 (height (min (or height image-height) (- image-height src-y)))
	 (format :xy-pixmap))
    (declare (type display display)
	     (type card8 depth)
	     (type card16 image-width image-height width height))
    (when bitmap-p
      (setq format :bitmap)
      (unless (= depth 1) (error "Bitmaps must have depth 1")))
    ;; Send image to the display
    (with-buffer-request (display *x-putimage* :gc-force gcontext)
      ((data (member :bitmap :xy-pixmap :z-pixmap)) format)
      (drawable drawable)
      (gcontext gcontext)
      (card16 width height)
      (int16 x y)
      (card8 0 depth)				;left-pad, depth
      (pad16 nil)
      (progn
	(buffer-put-xy-pixarray display buffer-boffset (image-xy-bitmap-list image)
				1 src-x src-y width height scanline-pad)))))

(defun buffer-put-xy-pixarray (display boffset bitmaps bits-per-pixel src-x src-y width height scanline-pad)
  (declare (type display display)
	   (type array-index boffset)
	   (type list bitmaps)
	   (type (member 1 4 8 16 24 32) bits-per-pixel)
	   (type card16 src-x src-y width height))
  (let* ((row src-y)
	 (bytes-per-line (ceiling (* width bits-per-pixel) 8))
	 (bitmap-format (display-bitmap-format display))
	 (scanline-length (scanline-byte-round bytes-per-line scanline-pad))
	 (length (* height scanline-length (length bitmaps)))
	 (size (buffer-size display))
	 (bit-lsb-first-p (bitmap-format-lsb-first-p bitmap-format))
	 (byte-lsb-first-p (display-image-lsb-first-p display)))
    (declare (type array-index row bytes-per-line scanline-length length size))
    (with-buffer-output (display :index boffset :sizes (8 16))
      (card16-put 2 (ceiling (index+ 24 length) 4)) ;; Set length
      (index-incf boffset 24)
      (dolist (pixarray bitmaps)
	(setq row src-y)
	(dotimes (i height)
	  ;; Flush buffer when necessary
	  (when (>= (+ boffset scanline-length) size)
	    (setf (buffer-boffset display) boffset)
	    (buffer-flush display)
	    (setq boffset (buffer-boffset display)))
	  ;; Copy scanline
	  (copy-pixarray-row-1 buffer-bbuf boffset pixarray row src-x width)
	  ;; Swap bytes and bits when needed
	  (when byte-lsb-first-p
	    (byte-swap-vector buffer-bbuf boffset scanline-length scanline-pad))
	  (when (and (= bits-per-pixel 1) bit-lsb-first-p)
	    (bit-reverse-vector buffer-bbuf boffset scanline-length))
	  (index-incf row)
	  (index-incf boffset scanline-length)))
      (setf (buffer-boffset display) (lround boffset)))))


;;;-----------------------------------------------------------------------------

(defun copy-image (image &key (x 0) (y 0) width height result-type)
  ;; Copy with optional sub-imaging and format conversion.
  ;; result-type defaults to (type-of image)
  (declare (type image image)
	   (type card16 x y)
	   (type (or null card16) width height) ;; Default from image
	   (type (or null (member image-x image-xy image-z)) result-type))
  (declare-values image)
  (let* ((image-width (image-width image))
	 (image-height (image-height image))
	 (width (or width image-width))
	 (height (or height image-height)))
    (setq width (min width (max (- image-width x) 0)))
    (setq height (min height (max (- image-height x) 0)))
    (etypecase image
      (image-x
       (ecase result-type
	 ((nil image-x) (image-x->image-x image x y width height))
	 (image-xy (image-x->image-xy image x y width height))
	 (image-z  (image-x->image-z  image x y width height))))
      (image-xy
       (ecase result-type
	 (image-x (image-xy->image-x image x y width height))
	 ((nil image-xy)
	  (let ((copy (copy-image-xy image)))
	    (setf (image-xy-bitmap-list copy)
		  (mapcar #'copy-pixarray (image-xy-bitmap-list image)))
	    copy))
	 (image-z  (image-xy->image-z  image x y width height))))
      (image-z 
       (ecase result-type
	 (image-x (image-z->image-x image x y width height))
	 (image-xy  (image-z->image-xy image x y width height))
	 ((nil image-z)
	  (let ((copy (copy-image-z image)))
	    (setf (image-z-pixarray copy) (copy-pixarray (image-z-pixarray image)))
	    copy)))))))

(defun copy-pixarray (array)
  (if #.(fboundp 'copy) ;; Some lisps may not have copy...
      (copy array)
    (let ((copy (make-array (array-dimensions array)
			    :element-type (array-element-type array))))
      (dotimes (i (array-dimension array 0))
	(dotimes (j (array-dimension array 1))
	  (setf (aref copy i j) (aref array i j))))
      copy)))

(defun image-x->image-x (image src-x src-y width height)
  (let ((vector
	  (ecase (image-x-format image)
	    (:z-pixmap
	     (copy-image-to-vector image src-x src-y width height (image-x-bits-per-pixel image) 1))
	    (:xy-pixmap
	     (copy-image-to-vector image src-x src-y width height 1 (image-depth image))))))
    (create-image :width width :height height :plist (image-plist image)
		  :depth (image-x-depth image)
		  :data vector)))

(defun copy-image-to-vector (image src-x src-y width height bits-per-pixel nplanes)
  ;; copy an X-Format Z-pixmap image into the buffer
  (declare (type image-x image)
	   (type card16 src-x src-y width height)
	   (type (member 1 4 8 16 24 32) bits-per-pixel)
	   (type card8 nplanes))
  (let* ((start-bit (* src-x bits-per-pixel))
	 (start-byte (floor start-bit 8))
	 (end-byte (ceiling (+ start-bit (* width bits-per-pixel)) 8))
	 (scanline-length (- end-byte start-byte))
	 (length (* height scanline-length nplanes))
	 (vector (make-array length :element-type 'card8))
	 (data (image-x-data image))
	 (image-x-bytes-per-line (image-x-bytes-per-line image))
	 (start (* src-y image-x-bytes-per-line))
	 (plane-length (+ (* (image-height image) image-x-bytes-per-line) start))
	 (plane-start start)
	 (boffset 0))
    (declare (type array-index start-bit start-byte end-byte scanline-length length
		   image-x-bytes-per-line start boffset)
	     (type (simple-array card8 (*)) data))
    ;; Note: The server doesn't handle left-pad for z-format, and neither do we.
    (dotimes (i nplanes)
      (setq start plane-start)
      ;; Copy scanlines
      (dotimes (row height)
	(buffer-replace vector data boffset (incf boffset scanline-length)
			(+ start start-byte))
	(incf start image-x-bytes-per-line))
      (incf plane-start plane-length))
    vector))

(defun image-x->image-z  (image x y width height)
  (declare (type image-x image)
	   (type card16 x y width height))
  (declare-values image-z)
  (let ((pixarray (if (and (eq (image-x-format image) :xy-pixmap)
			   (> (image-depth image) 1))
		      (error "Conversion from :XY-PIXMAP to image-z not supported")
		      (z-format-pixarray (image-x-data image) 0 (image-x-bytes-per-line image)
					 (image-depth image) (image-x-bits-per-pixel image)
					 x y width height))))
    (create-image :width width :height height :plist (image-plist image)
		  :depth (image-x-depth image)
		  :data pixarray)))

(defun image-x->image-xy (image x y width height)
  (declare (type image-x image)
	   (type card16 x y width height))
  (declare-values image-xy)
  (if (eq (image-x-format image) :z-pixmap)
      (error "Conversion from :Z-PIXMAP to IMAGE-XY not supported")
    (do* ((depth (image-depth image))
	  (plane 0 (1+ plane))
	  (bytes-per-line (image-x-bytes-per-line image))
	  (bytes-per-plane (* bytes-per-line (image-height image)))
	  (start 0 (+ start bytes-per-plane))
	  (data (image-x-data image))
	  (result nil))
	((>= plane depth)
	 (create-image :width width :height height
		       :depth depth :data (nreverse result)
		       ))
      (push (z-format-pixarray data start bytes-per-line 1 1
			       x y width height)
	    result))))

(defun image-xy->image-x (image x y width height)
  (let* ((depth (image-depth image))
	 (bytes-per-line (ceiling width 8))
	 (bitmaps (image-xy-bitmap-list image))
	 (length (* height bytes-per-line depth))
	 (vector (make-array length :element-type 'card8)))
    (dolist (bitmap bitmaps)
      (copy-pixarray-to-vector vector bitmap 1 bytes-per-line
			       x y width height))
    (create-image :width width :height height :depth depth :plist (image-plist image)
		  :format :xy-pixmap :bytes-per-line bytes-per-line
		  :data vector)))

(defun image-z->image-x (image x y width height)
  (let* ((bits-per-pixel (image-z-bits-per-pixel image))
	 (bytes-per-line (ceiling (* width bits-per-pixel) 8))
	 (length (* height bytes-per-line))
	 (vector (make-array length :element-type 'card8)))
    (copy-pixarray-to-vector vector (image-z-pixarray image) bits-per-pixel bytes-per-line
			     x y width height)
    (create-image :width width :height height :depth (image-depth image)
		  :plist (image-plist image) :bits-per-pixel bits-per-pixel
		  :format :z-pixmap :bytes-per-line bytes-per-line
		  :data vector)))

(defun copy-pixarray-to-vector (vector pixarray bits-per-pixel bytes-per-line src-x src-y width height)
  (declare (type pixarray pixarray)
	   (type (member 1 4 8 16 24 32) bits-per-pixel)
	   (type card16 src-x src-y width height))
  (let* ((row src-y)
	 (copy-pixarray-row (cdr (assoc bits-per-pixel
					'((1 . copy-pixarray-row-1)
					  (4 . copy-pixarray-row-4)
					  (8 . copy-pixarray-row-8)
					  (16 . copy-pixarray-row-16)
					  (24 . copy-pixarray-row-24)
					  (32 . copy-pixarray-row-32)))))
	 (boffset 0))
    (declare (type array-index row boffset))
    (dotimes (i height)
      ;; Copy scanline
      (funcall copy-pixarray-row vector boffset pixarray row src-x width)
      (index-incf row)
      (index-incf boffset bytes-per-line))))

(defun image-xy->image-z (image x y width height)
  image x y width height ;; unused
  (error "Conversion of image-xy to image-z not supported"))

(defun image-z->image-xy (image x y width height)
  image x y width height ;; unused
  (error "Conversion of image-z to image-xy not supported"))


;;;-----------------------------------------------------------------------------
;;; Image I/O functions


(defun read-bitmap-file (pathname)
  ;; Creates an image from a C include file in standard X11 format
  (declare (type (or pathname string stream) pathname))
  (declare-values image)
  (with-open-file (fstream pathname :direction :input)
    (let ((line "")
	  (name nil)
	  (properties nil)
	  (start nil))
      (declare (type string line)
	       (type stringable name)
	       (type list properties)
	       (type (or null array-index) start))
      (with-vector (line string)
	;; Get properties
	(loop
	  (setq line (read-line fstream))
	  (unless (eql (aref line 0) #\#)
	    (return))
	  (unless start (setq start (position #\_ line :from-end t)))
	  (let ((*package* (find-package 'keyword))
		(value 0)
		property)
	    (setq name (read-from-string line t nil :start 7 :end start))
	    (multiple-value-setq (property value)
	      (read-from-string line t nil :start (1+ start)))
	    (setf (getf properties property) (read-from-string line t nil :start value))))
	(when name (setf (getf properties :name) name))
	;; Calculate sizes
	(let* ((width (getf properties :width))
	       (height (getf properties :height))
	       (depth (getf properties :depth 1)))
	  (declare (type (or null card16) width height))
	  (unless (and width height)
	    (error "Not a BITMAP file"))
	  (let* ((byte-width (ceiling (* width depth) 8))
		 (line-width (* 4 (ceiling (* width depth) 32)))
		 (data (make-array (* line-width height)
				   :element-type '(unsigned-byte 8)))
		 (number-string (make-string 2))
		 (line-base 0)
		 (byte 0))
	    (declare (type card16 byte-width line-width line-base byte)
		     (type string number-string)
		     (type (simple-array card8 (*)) data))
	    (with-vector (data (simple-array card8 (*)))
	      ;; Read data
	      (dotimes (i height)
		(dotimes (j byte-width)
		  (loop (when (eql (read-char fstream) #\x) (return)))
		  (setf (aref number-string 0) (read-char fstream))
		  (setf (aref number-string 1) (read-char fstream))
		  (setf (aref data (+ line-base byte))
			(parse-integer number-string :radix 16. :junk-allowed t))
		  (incf byte))
		(setq byte 0
		      line-base (+ line-base line-width)))
	      (create-image :width width :height height :depth depth
			    :plist properties :data data
			    :bit-lsb-first-p t ;; WHY? (sounds like a bug)
			    ))))))))

(defun write-bitmap-file (pathname image &optional name)
  ;; Writes an image to a C include file in standard X11 format
  ;; NAME argument used for variable prefixes.  Defaults to "image"
  (declare (type (or pathname string stream) pathname)
	   (type image image)
	   (type (or null stringable) name))
  (unless (typep image 'image-x)
    (setq image (copy-image image :result-type 'image-x)))
  (when (and (= (image-depth image) 1)
	     (not (image-x-bit-lsb-first-p image)))
    (bit-reverse-image image))
  (let* ((plist (image-plist image))
	 (name (or name (image-name image) 'image))
	 (width (image-width image))
	 (height (image-height image))
	 (depth (image-depth image))
	 (data (image-x-data image))
	 (byte-width (ceiling (* width depth) 8))
	 (line-width (* 4 (ceiling (* width depth) 32)))
	 (line 0)
	 (byte-number 0)
	 (count 0))
    (declare (type list plist)
	     (type stringable name)
	     (type card16 width height)
	     (type card16 byte-width line-width line byte-number count)
	     (type (simple-array card8 (*)) data))
    (with-vector (data (simple-array card8 (*)))
      (setq name (string-downcase (string name)))
      (with-open-file (fstream pathname :direction :output)
	(format fstream "#define ~a_width ~d~%" name width)
	(format fstream "#define ~a_height ~d~%" name height)
	(unless (= depth 1)
	  (format fstream "#define ~a_depth ~d~%" name depth))
	(do ((prop plist (cddr prop)))
	    ((endp prop))
	  (when (and (not (member (car prop) '(:width :height)))
		     (numberp (cadr prop)))
	    (format fstream "#define ~a_~a ~d~%"
		    name (string-downcase (string (car prop))) (cadr prop))))
	(format fstream "static char ~a_bits[] = {" name)
	(dotimes (i height)
	  (dotimes (j byte-width)
	    (when (zerop (mod count 12)) (format fstream "~%  "))
	    (write-string " 0x" fstream)
	    ;; Faster than (format fstream "0x~2,'0x," byte)
	    (let ((byte (aref data (+ line byte-number)))
		  (translate "0123456789abcdef"))
	      (write-char (aref translate (ldb (byte 4 4) byte)) fstream)
	      (write-char (aref translate (ldb (byte 4 0) byte)) fstream)
	      (incf byte-number)
	      (incf count)
	      (unless (and (= (1+ i) height)
			   (= (1+ j) byte-width))
		(write-char #\, fstream))))
	  (setq byte-number 0
		line (+ line line-width)))
	(format fstream "};~%" fstream)))))


(defun bitmap-image (&optional plist &rest patterns)
  ;; Create an image containg pattern
  ;; PATTERNS are bit-vector constants (e.g. #*10101)
  ;; If the first parameter is a list, its used as the image property-list.
  (declare (type (or list bit-vector) plist)
	   (type list patterns)) ;; list of bitvector
  (declare-values image)
  (unless (listp plist)
    (push plist patterns)
    (setq plist nil))
  (let* ((width (length (first patterns)))
	 (height (length patterns))
	 (bitarray (make-array (list height width) :element-type 'bit))
	 (row 0))
    (dolist (pattern patterns)
      (dotimes (col width)
	(setf (aref bitarray row col) (aref pattern col)))
      (incf row))
    (create-image :width width :height height :plist plist :data bitarray)))

(defun image-pixmap (drawable image &key gcontext width height depth)
  ;; Create a pixmap containing IMAGE. Size defaults from the image.
  ;; DEPTH is the pixmap depth.
  ;; GCONTEXT is used for putting the image into the pixmap.
  ;; If none is supplied, then one is created, used then freed.
  (declare (type drawable drawable)
	   (type image image)
	   (type (or null gcontext) gcontext)
	   (type (or null card16) width height)
	   (type (or null card8) depth))
  (declare-values pixmap)
  (let* ((image-width (image-width image))
	 (image-height (image-height image))
	 (image-depth (image-depth image))
	 (width (or width image-width))
	 (height (or height image-height))
	 (depth (or depth image-depth))
	 (pixmap (create-pixmap :drawable drawable
			       :width width
			       :height height
			       :depth depth))
	 (gc (or gcontext (create-gcontext
			    :drawable pixmap
			    :foreground 1
			    :background 0))))
    (unless (= depth image-depth)
      (if (= image-depth 1)
	  (unless gcontext (xlib::required-arg gcontext))
	(error "Pixmap depth ~d incompatable with image depth ~d"
	       depth image-depth)))	       
    (put-image pixmap gc image :x 0 :y 0
	       :bitmap-p (and (= image-depth 1)
			      gcontext))
    ;; Tile when image-width is less than the pixmap width, or
    ;; the image-height is less than the pixmap height.
    ;; ??? Would it be better to create a temporary pixmap and 
    ;; ??? let the server do the tileing?
    (do ((x image-width (+ x image-width)))
	((>= x width))
      (copy-area pixmap gc 0 0 image-width image-height pixmap x 0)
      (incf image-width image-width))
    (do ((y image-height (+ y image-height)))
	((>= y height))
      (copy-area pixmap gc 0 0 image-width image-height pixmap 0 y)
      (incf image-height image-height))
    (unless gcontext (free-gcontext gc))
    pixmap))

