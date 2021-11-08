;;; -*- Mode: common-lisp; Syntax: Common-lisp; Package: USER; Base: 10; Lowercase: Yes -*-

(eval-when (eval compile load)
  (let ((x '#(1)))
    (if (not (eq 0 (sys::memref x
				#.(comp::mdparam 'comp::md-svector-data0-adj)
				0 :unsigned-byte)))
	(pushnew :clx-little-endian *features*)))
  #+allegro
  (if (not (boundp 'excl::cl-major-version-number))
      (pushnew :allegro-v2.2 *features*)))


(eval-when (load)
  (provide :clxexcldep))

(in-package :excl)

#-allegro
(defun type-array-element-type-to-array (type &aux temp) 
   ;; type is a type descriptor, return a descriptor which tells
   ;; the array code what kind of array to make

   ; convert the given element type to one of the symbols which
   ; is in the car of the array-descriptors list
   ;(msg "beginning type is " type 'N)
   (if* (symbolp type)
      then (if* (franz:memq type '(t bit string-char fixnum))
	      thenret	; it is ok as it is
	      else (let ((temp (get type 'deftype-expander)))
		     (if* temp
			then
			     (return-from type-array-element-type-to-array
			       (type-array-element-type-to-array
				(funcall temp (list type))))
			else
			     (setq type (case type
					  (standard-char 'string-char)
					  ((single-float short-float) 'single-float)
					  ((double-float long-float) 'double-float)
					  (t t))))))
    elseif (consp type)
      then (setq type
		 (case (car type)
		    (mod (if* (integerp (setq temp (cadr type)))
			    then (cond ((< temp 1) t)
				       ((<= temp 2) 'bit)
				       ((<= temp 256) 'ubyte)
				       ((<= temp 65536) 'uword)
				       ((<= temp 4294967296) 'ulong)
				       (t t))
			    else t))
		    (signed-byte
		       (if* (integerp (setq temp (cadr type)))
			  then (cond ((<= temp 0) t)
				     ((<= temp 8) 'byte)
				     ((<= temp 16) 'word)
				     ((<= temp 29) 'fixnum)
				     ((<= temp 32) 'long)
				     (t  t))
			  else t))
		    (unsigned-byte
		       (if* (integerp (setq temp (cadr type)))
			  then (cond ((<= temp 0) t)
				     ((<= temp 8) 'ubyte)
				     ((<= temp 16) 'uword)
				     ((<= temp 32) 'ulong)
				     (t  t))
			  else t))
		    (t t)))
      else (setq type t))
   ; type is now one of the valid types.  We return a descriptor
   ; based on that name
   ;(msg "resulting type is " type 'N)
   (let ((res (franz:assq type array-descriptors)))
      ;(msg " resulting decriptor " res 'N)
      res))

#+(or (not allegro) allegro-v2.2)
(defun make-sequence (type length &rest rest &key initial-element)
  "Returns a sequence of the given Type and Length, with elements initialized
  to :Initial-Element."
  (declare (fixnum length)
	   (ignore initial-element))
  (case (type-specifier type)
    (list (apply #'make-list length rest))
    ((simple-string string)
     (apply #'make-string length rest))
    ((array simple-array vector simple-vector)
     (if* (listp type)
	 then (apply #'make-array length :element-type (cadr type) rest)
	 else (apply #'make-array length rest)))
    ((bit-vector simple-bit-vector)
     (apply #'make-array length :element-type 'bit rest))
    (t
     ;; Now, we can either have a user-defined type symbol, or an error.
     (if* (symbolp type)
	then (let ((temp (get type 'excl::deftype-expander)))
	       (if* temp
		  then (cond (rest (return-from make-sequence
				     (make-sequence (funcall temp (list type)) length
						   :initial-element (cadr rest))))
			      (t (return-from make-sequence (make-sequence
				   (funcall temp (list type)) length)))))))
     (error "~s is a bad type specifier for sequences." type ))))

#-allegro
(eval-when (load compile)
  (if (not (fboundp 'excl::scheduler-running-p))
      (defun excl::scheduler-running-p ()
         (boundp 'mp::*scheduler-stack-group*))))
