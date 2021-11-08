;;; -*- Mode: common-lisp; Package: xlib; Base: 10; Lowercase: Yes -*-
;;;
;;; Copyright (c) 1988 Franz Inc, Berkeley, Ca.
;;;
;;; Permission is granted to any individual or institution to use, copy,
;;; modify, and distribute this software, provided that this complete
;;; copyright and permission notice is maintained, intact, in all copies and
;;; supporting documentation.
;;;
;;; Franz Incorporated provides this software "as is" without express or
;;; implied warranty.
;;;

(in-package :xlib :use '(:foreign-functions :lisp :excl))

(require :foreign)
;; (require :mdproc)
;; (require :process)

#+allegro
(require :defsystem "defsys")

(eval-when (load)
  (require :clxexcldep "excldep")
  (provide :clx))


;;
;; The following is a suggestion.  If you comment out this form be
;; prepared for possible deadlock, since no interrupts will be recognized
;; while reading from the X socket if the scheduler is not running.
;;
(setq compiler::generate-interrupt-checks-switch
  (compile nil '(lambda (safety size speed)
		  (declare (ignore size))
		  (or (< speed 3) (> safety 0)))))


;;
;; Now comes the stuff that really belongs here.
;;

#+allegro
(excl:defsystem clx
;;              (:default-pathname "/usr/tech/jdi/lisp/CLX/")
                ()
  |depdefs|
  (|clx| :load-before-compile (|depdefs|)
	 :recompile-on (|depdefs|))
  (|dependent| :load-before-compile (|depdefs| |clx|)
	       :recompile-on (|clx|))
  (|macros| :load-before-compile (|depdefs| |clx| |dependent|)
	    :recompile-on (|dependent|))
  (|bufmac| :load-before-compile (|depdefs| |clx| |dependent| |macros|)
	    :recompile-on (|macros|))
  (|buffer| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					    |bufmac|)
	    :recompile-on (|bufmac|))
  (|display| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					     |bufmac| |buffer|)
	     :recompile-on (|buffer|))
  (|gcontext| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					      |bufmac| |buffer| |display|)
	      :recompile-on (|display|))
  (|requests| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					      |bufmac| |buffer| |display|)
	      :recompile-on (|display|))
  (|input| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					      |bufmac| |buffer| |display|)
	   :recompile-on (|display|))
  (|fonts| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					   |bufmac| |buffer| |display|)
	   :recompile-on (|display|))
  (|graphics| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					      |bufmac| |buffer| |display|
					      |fonts|)
	      :recompile-on (|fonts|))
  (|text| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					      |bufmac| |buffer| |display|
					      |gcontext| |fonts|)
	  :recompile-on (|gcontext| |fonts|)
	  :load-after (|translate|))
  ;; The above line gets around a compiler macro expansion bug.
  (|attributes| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					   |bufmac| |buffer| |display|)
		:recompile-on (|display|))
  (|translate| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					   |bufmac| |buffer| |display| |text|)
	     :recompile-on (|display|))
  (|keysyms| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					     |bufmac| |buffer| |display|
					     |translate|)
	     :recompile-on (|translate|))
  (|manager| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					   |bufmac| |buffer| |display|)
	     :recompile-on (|display|))
  (|image| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					   |bufmac| |buffer| |display|)
	   :recompile-on (|display|))
  
  ;; Don't know if l-b-c list is correct.  XXX
  (|resource| :load-before-compile (|depdefs| |clx| |dependent| |macros|
					   |bufmac| |buffer| |display|)
	   :recompile-on (|display|))
  )


(defun compile-clx (&optional pathname-defaults)
  (let ((*default-pathname-defaults*
	  (or pathname-defaults *default-pathname-defaults*)))
    (declare (special *default-pathname-defaults*))
    (compile-file "depdefs")
    (load "depdefs")
    (compile-file "clx")
    (load "clx")
    (compile-file "dependent")
    (load "dependent")
    (compile-file "macros")
    (load "macros")
    (compile-file "bufmac")
    (load "bufmac")
    (compile-file "buffer")
    (load "buffer")
    (compile-file "display")
    (load "display")
    (compile-file "gcontext")
    (load "gcontext")
    (compile-file "requests")
    (load "requests")
    (compile-file "input")
    (load "input")
    (compile-file "fonts")
    (load "fonts")
    (compile-file "graphics")
    (load "graphics")
    (compile-file "text")
    (load "text")
    (compile-file "attributes")
    (load "attributes")
    (load "translate")
    (compile-file "translate")		; work-around bug in 2.0 and 2.2
    (load "translate")
    (compile-file "keysyms")
    (load "keysyms")
    (compile-file "manager")
    (load "manager")
    (compile-file "image")
    (load "image")
    (compile-file "resource")
    (load "resource")
    ))


(defun load-clx (&optional pathname-defaults)
  (let ((*default-pathname-defaults*
	  (or pathname-defaults *default-pathname-defaults*)))
    (declare (special *default-pathname-defaults*))
    (load "depdefs")
    (load "clx")
    (load "dependent")
    (load "macros")
    (load "bufmac")
    (load "buffer")
    (load "display")
    (load "gcontext")
    (load "requests")
    (load "input")
    (load "fonts")
    (load "graphics")
    (load "text")
    (load "attributes")
    (load "translate")
    (load "keysyms")
    (load "manager")
    (load "image")
    (load "resource")
    ))


(when (plusp (ff:get-entry-points
	      (make-array 2
			  :initial-contents
			  (list (ff:convert-to-lang "connect_to_server"
						    :language :c)
				(ff:convert-to-lang "c_howmany_bytes"
						    :language :c)))
	      (make-array 2 :element-type '(unsigned-byte 32))))

  (ff:remove-entry-point (ff:convert-to-lang "connect_to_server" :language :c))
  (ff:remove-entry-point (ff:convert-to-lang "c_howmany_bytes" :language :c))
  (ff:remove-entry-point (ff:convert-to-lang "c_read_bytes" :language :c))
  (ff:remove-entry-point (ff:convert-to-lang "c_read_bytes_interruptible"
					     :language :c))
  (ff:remove-entry-point (ff:convert-to-lang "c_write_bytes" :language :c))
  (ff:remove-entry-point (ff:convert-to-lang "c_flush_bytes" :language :c))
  (load "socket.o")
  (load "excldep.o")
  )


(ff:defforeign-list `((xlib::connect-to-server
		       :entry-point
		       ,(ff:convert-to-lang "connect_to_server" :language :c)
		       :return-type :fixnum
		       :arg-checking nil
		       :arguments (string fixnum))
		      (xlib::c-howmany-bytes
		       :entry-point
		       ,(ff:convert-to-lang "c_howmany_bytes" :language :c)
		       :return-type :fixnum
		       :arg-checking nil
		       :arguments (fixnum))
		      (xlib::c-read-bytes
		       :entry-point
		       ,(ff:convert-to-lang "c_read_bytes" :language :c)
		       :return-type :fixnum
		       :arg-checking nil
		       :arguments (fixnum (simple-array (unsigned-byte 8))
				   fixnum fixnum))
		      (xlib::c-read-bytes-interruptible
		       :entry-point
		       ,(ff:convert-to-lang "c_read_bytes_interruptible"
			 :language :c)
		       :return-type :fixnum
		       :arg-checking nil
		       :arguments (fixnum (simple-array (unsigned-byte 8))
				   fixnum fixnum))
		      (xlib::c-write-bytes
		       :entry-point
		       ,(ff:convert-to-lang "c_write_bytes" :language :c)
		       :return-type :fixnum
		       :arg-checking nil
		       :arguments (fixnum (simple-array (unsigned-byte 8))
				   fixnum fixnum))
		      (xlib::c-flush-bytes
		       :entry-point
		       ,(ff:convert-to-lang "c_flush_bytes" :language :c)
		       :return-type :fixnum
		       :arg-checking nil
		       :arguments (fixnum))
		      ))
