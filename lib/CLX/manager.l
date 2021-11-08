;;; -*- Mode:Lisp; Package:XLIB; Syntax:COMMON-LISP; Base:10; Lowercase:T -*-

;;; Window Manager Property functions

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

(export '(wm-name		;These are all setf'able accessor functions
	  wm-icon-name
	  wm-client-machine
	  wm-command
	  wm-hints
	  wm-normal-hints
	  wm-zoom-hints
	  icon-sizes

	  wm-size-hints
	  wm-size-hints-p
	  make-wm-size-hints
	  wm-size-hints-user-specified-position-p
	  wm-size-hints-user-specified-size-p
	  wm-size-hints-x
	  wm-size-hints-y
	  wm-size-hints-width
	  wm-size-hints-height
	  wm-size-hints-min-width
	  wm-size-hints-min-height
	  wm-size-hints-max-width
	  wm-size-hints-max-height
	  wm-size-hints-width-inc
	  wm-size-hints-height-inc
	  wm-size-hints-min-aspect
	  wm-size-hints-max-aspect

	  wm-hints
	  wm-hints-p
	  make-wm-hints
	  wm-hints-input
	  wm-hints-initial-state
	  wm-hints-icon-pixmap
	  wm-hints-icon-window
	  wm-hints-icon-x
	  wm-hints-icon-y
	  wm-hints-icon-mask
	  wm-hints-window-group
	  wm-hints-flags

	  transient-for

	  set-standard-properties

	  get-wm-class
	  set-wm-class
	  get-standard-colormap
	  set-standard-colormap

	  cut-buffer   ;; Setf'able
	  rotate-cut-buffers
	  ))

(defun wm-name (window)
  (declare (type window window))
  (declare-values string)
  (get-property window :wm_name :type :string :result-type 'string :transform #'card8->char))

(defsetf wm-name (window) (name)
  (declare (type window window))
  (declare-values string)
  `(set-string-property ,window :wm_name ,name))

(defun set-string-property (window property string)
  (declare (type window window)
	   (type keyword property)
	   (type stringable string))
  (change-property window property (string string) :string 8 :transform #'char->card8)
  string)

(defun wm-icon-name (window)
  (declare (type window window))
  (declare-values string)
  (get-property window :wm_icon_name :type :string
		:result-type 'string :transform #'card8->char))

(defsetf wm-icon-name (window) (name)
  `(set-string-property ,window :wm_icon_name ,name))

(defun wm-client-machine (window)
  (declare (type window window))
  (declare-values string)
  (get-property window :wm_client_machine :type :string
		:result-type 'string :transform #'card8->char))

(defsetf wm-client-machine (window) (name)
  `(set-string-property ,window :wm_client_machine ,name))

(defun get-wm-class (window)
  (declare (type window window))
  (declare-values (or null name-string) (or null class-string))
  (let ((value (get-property window :wm_class :type :string
			     :result-type 'string :transform #'card8->char)))
    (declare (type (or null string) value))
    (when value
      (let* ((name-len (position #.(int-char 0) (the string value)))
	     (name (subseq (the string value) 0 name-len))
	     (class (subseq (the string value) (1+ name-len) (1- (length value)))))
	(values (and (plusp (length name)) name)
		(and (plusp (length class)) class))))))

(defun set-wm-class (window resource-name resource-class)
  (declare (type window window)
	   (type (or null stringable) resource-name resource-class))
  (set-string-property window :wm_class
		       (concatenate 'string
				    (string (or resource-name ""))
				    #.(make-string 1 :initial-element (int-char 0))
				    (string (or resource-class ""))
				    #.(make-string 1 :initial-element (int-char 0))))
  (values))

(defun wm-command (window)
  ;; Returns a list whose car is the command and 
  ;; whose cdr is the list of arguments
  (declare (type window window))
  (declare-values list)
  (do* ((command-string (get-property window :wm_command :type :string
				      :result-type 'string :transform #'card8->char))
	(command nil)
	(start 0 (1+ end))
	(end 0)
	(len (length command-string)))
       ((>= start len) (nreverse command))
    (setq end (position #.(int-char 0) command-string :start start))
    (push (subseq command-string start end) command)))

(defsetf wm-command set-wm-command)
(defun set-wm-command (window command)
  ;; Uses PRIN1 to a string-stream with the following bindings:
  ;; (*print-length* nil) (*print-level* nil) (*print-radix* nil)
  ;; (*print-base* 10.) (*print-array* t) (*package* (find-package 'lisp))
  ;; each element of command is seperated with NULL characters.
  ;; This enables (mapcar #'read-from-string (wm-command window))
  ;; to recover a lisp command.
  (declare (type window window)
	   (type list command))
  (set-string-property window :wm_command
		       (with-output-to-string (stream)
			 (let ((*print-length* nil)
			       (*print-level* nil)
			       (*print-radix* nil)
			       (*print-base* 10.)
			       (*print-array* t)
			       (*package* (find-package 'lisp))
			       #+ti (ticl:*print-structure* t))
			   (dolist (c command)
			     (prin1 c stream)
			     (write-char #.(int-char 0) stream)))))
  command)

;;-----------------------------------------------------------------------------
;; WM_HINTS

(defstruct wm-hints
  (input nil :type (or null (member :off :on)))
  (initial-state nil :type (or null (member :dont-care :normal :zoom :iconic :inactive)))
  (icon-pixmap nil :type (or null pixmap))
  (icon-window nil :type (or null window))
  (icon-x nil :type (or null card16))
  (icon-y nil :type (or null card16))
  (icon-mask nil :type (or null pixmap))
  (window-group nil :type (or null resource-id))
  (flags 0 :type card32)    ;; Extension-hook.  Exclusive-Or'ed with the FLAGS field
  ;; may be extended in the future
  )

(defun wm-hints (window)
  (declare (type window window))
  (declare-values wm-hints)
  (let ((prop (get-property window :wm_hints :type :wm_hints :result-type 'vector)))
    (when prop
      (decode-wm-hints prop (window-display window)))))

(defsetf wm-hints set-wm-hints)
(defun set-wm-hints (window wm-hints)
  (declare (type window window)
	   (type wm-hints wm-hints))
  (declare-values wm-hints)
  (change-property window :wm_hints (encode-wm-hints wm-hints) :wm_hints 32)
  wm-hints)

(defun decode-wm-hints (vector display)
  (declare (type (simple-vector 9) vector)
	   (type display display))
  (declare-values wm-hints)
  (let ((input-hint 0)
	(state-hint 1)
	(icon-pixmap-hint 2)
	(icon-window-hint 3)
	(icon-position-hint 4)
	(icon-mask-hint 5)
	(window-group-hint 6)
	)
    (let ((flags (aref vector 0))
	  (hints (make-wm-hints)))
      (declare (type card32 flags)
	       (type wm-hints hints))
      (setf (wm-hints-flags hints) flags)
      (compiler-let ((*buffer* 'display))
	(when (logbitp input-hint flags)
	  (setf (wm-hints-input hints) (decode-type (member :off :on) (aref vector 1))))
	(when (logbitp state-hint flags)
	  (setf (wm-hints-initial-state hints)
		(decode-type (member :dont-care :normal :zoom :iconic :inactive)
			     (aref vector 2))))
	(when (logbitp icon-pixmap-hint flags)
	  (setf (wm-hints-icon-pixmap hints) (decode-type pixmap (aref vector 3))))
	(when (logbitp icon-window-hint flags)
	  (setf (wm-hints-icon-window hints) (decode-type window (aref vector 4))))
	(when (logbitp icon-position-hint flags)
	  (setf (wm-hints-icon-x hints) (aref vector 5)
		(wm-hints-icon-y hints) (aref vector 6)))
	(when (logbitp icon-mask-hint flags)
	  (setf (wm-hints-icon-mask hints) (decode-type pixmap (aref vector 7))))
	(when (and (logbitp window-group-hint flags) (> (length vector) 7))
	  (setf (wm-hints-window-group hints) (aref vector 8)))
	hints))))


(defun encode-wm-hints (wm-hints)
  (declare (type wm-hints wm-hints))
  (declare-values simple-vector)
  (let ((input-hint         #b1)
	(state-hint         #b10)
	(icon-pixmap-hint   #b100)
	(icon-window-hint   #b1000)
	(icon-position-hint #b10000)
	(icon-mask-hint     #b100000)
	(window-group-hint  #b1000000)
	(mask               #b1111111)
	)
    (let ((vector (make-array 9 :initial-element 0))
	  (flags 0))
      (declare (type (simple-vector 9) vector)
	       (type card16 flags))
      (when (wm-hints-input wm-hints)
	(setf flags input-hint
	      (aref vector 1) (encode-type (member :off :on) (wm-hints-input wm-hints))))
      (when (wm-hints-initial-state wm-hints)
	(setf flags (logior flags state-hint)
	      (aref vector 2) (encode-type (member :dont-care :normal :zoom :iconic :inactive)
					   (wm-hints-initial-state wm-hints))))
      (when (wm-hints-icon-pixmap wm-hints)
	(setf flags (logior flags icon-pixmap-hint)
	      (aref vector 3) (encode-type pixmap (wm-hints-icon-pixmap wm-hints))))
      (when (wm-hints-icon-window wm-hints)
	(setf flags (logior flags icon-window-hint)
	      (aref vector 4) (encode-type window (wm-hints-icon-window wm-hints))))
      (when (and (wm-hints-icon-x wm-hints) (wm-hints-icon-y wm-hints))
	(setf flags (logior flags icon-position-hint)
	      (aref vector 5) (encode-type card16 (wm-hints-icon-x wm-hints))
	      (aref vector 6) (encode-type card16 (wm-hints-icon-y wm-hints))))
      (when (wm-hints-icon-mask wm-hints)
	(setf flags (logior flags icon-mask-hint)
	      (aref vector 7) (encode-type pixmap (wm-hints-icon-mask wm-hints))))
      (when (wm-hints-window-group wm-hints)
	(setf flags (logior flags window-group-hint)
	      (aref vector 8) (wm-hints-window-group wm-hints)))
      (setf (aref vector 0) (logior flags (logandc2 (wm-hints-flags wm-hints) mask)))
      vector)))

;;-----------------------------------------------------------------------------
;; WM_SIZE_HINTS

(defstruct wm-size-hints
  ;; Defaulted T to put the burden of remembering these on widget programmers.
  (user-specified-position-p t :type boolean) ;; True when user specified x y
  (user-specified-size-p t :type boolean)     ;; True when user specified width height
  (x nil :type (or null int16))
  (y nil :type (or null int16))
  (width nil :type (or null card16))
  (height nil :type (or null card16))
  (min-width nil :type (or null card16))
  (min-height nil :type (or null card16))
  (max-width nil :type (or null card16))
  (max-height nil :type (or null card16))
  (width-inc nil :type (or null card16))
  (height-inc nil :type (or null card16))
  (min-aspect nil :type (or null number))
  (max-aspect nil :type (or null number)))

(defun wm-normal-hints (window)
  (declare (type window window))
  (declare-values wm-size-hints)
  (decode-wm-size-hints (get-property window :wm_normal_hints :type :wm_size_hints :result-type 'vector)))

(defsetf wm-normal-hints set-wm-normal-hints)
(defun set-wm-normal-hints (window hints)
  (declare (type window window)
	   (type wm-size-hints hints))
  (declare-values wm-size-hints)
  (change-property window :wm_normal_hints (encode-wm-size-hints hints) :wm_size_hints 32)
  hints)

(defun wm-zoom-hints (window)
  (declare (type window window))
  (declare-values wm-size-hints)
  (decode-wm-size-hints (get-property window :wm_zoom_hints :type :wm_size_hints :result-type 'vector)))

(defsetf wm-zoom-hints set-wm-zoom-hints)
(defun set-wm-zoom-hints (window hints)
  (declare (type window window)
	   (type wm-size-hints hints))
  (declare-values wm-size-hints)
  (change-property window :wm_zoom_hints (encode-wm-size-hints hints) :wm_size_hints 32)
  hints)

(defun decode-wm-size-hints (vector)
  (declare (type (or null (simple-vector 15)) vector))
  (declare-values (or null wm-size-hints))
  (when vector
    (let ((usposition 0)			;User Specified position
	  (ussize 1)				;User Specified size
	  (pposition 2)				;Program specified position
	  (psize 3)				;Program specified size
	  (pminsize 4)				;Program specified minimum size
	  (pmaxsize 5)				;Program specified maximum size
	  (presizeinc 6)			;Program specified resize increments
	  (paspect 7)				;Program specfied min and max aspect ratios
	  )
      (let ((flags (aref vector 0))
	    (hints (make-wm-size-hints)))
	(declare (type card16 flags)
		 (type wm-size-hints hints))
	(when (or (logbitp usposition flags)
		  (logbitp pposition flags))
	  (setf (wm-size-hints-user-specified-position-p hints) (logbitp usposition flags)
		(wm-size-hints-x hints) (aref vector 1)
		(wm-size-hints-y hints) (aref vector 2)))
	(when (or (logbitp ussize flags)
		  (logbitp psize flags))
	  (setf (wm-size-hints-user-specified-size-p hints) (logbitp usposition flags)
		(wm-size-hints-width hints) (aref vector 3)
		(wm-size-hints-height hints) (aref vector 4)))
	(when (logbitp pminsize flags)
	  (setf (wm-size-hints-min-width hints) (aref vector 5)
		(wm-size-hints-min-height hints) (aref vector 6)))
	(when (logbitp pmaxsize flags)
	  (setf (wm-size-hints-max-width hints) (aref vector 7)
		(wm-size-hints-max-height hints) (aref vector 8)))
	(when (logbitp presizeinc flags)
	  (setf (wm-size-hints-width-inc hints) (aref vector 9)
		(wm-size-hints-height-inc hints) (aref vector 10)))
	(when (logbitp paspect flags)
	  (setf (wm-size-hints-min-aspect hints) (/ (aref vector 11) (aref vector 12))
		(wm-size-hints-max-aspect hints) (/ (aref vector 13) (aref vector 14))))
	hints))))

(defun encode-wm-size-hints (hints)
  (declare (type wm-size-hints hints))
  (declare-values simple-vector)
  (let ((usposition #b1)			;User Specified position
	(ussize     #b10)			;User Specified size
	(pposition  #b100)			;Program specified position
	(psize      #b1000)			;Program specified size
	(pminsize   #b10000)			;Program specified minimum size
	(pmaxsize   #b100000)			;Program specified maximum size
	(presizeinc #b1000000)			;Program specified resize increments
	(paspect    #b10000000)			;Program specfied min and max aspect ratios
	)
    (let ((vector (make-array 15 :initial-element 0))
	  (flags 0))
      (declare (type (simple-vector 15) vector)
	       (type card16 flags))
      (when (and (wm-size-hints-x hints) (wm-size-hints-y hints))
	(setq flags (if (wm-size-hints-user-specified-position-p hints) usposition pposition))
	(setf (aref vector 1) (wm-size-hints-x hints)
	      (aref vector 2) (wm-size-hints-y hints)))
      (when (and (wm-size-hints-width hints) (wm-size-hints-height hints))
	(setf flags (logior flags (if (wm-size-hints-user-specified-position-p hints) ussize psize))
	      (aref vector 3) (wm-size-hints-width hints)
	      (aref vector 4) (wm-size-hints-height hints)))
      
      (when (and (wm-size-hints-min-width hints) (wm-size-hints-min-height hints))
	(setf flags (logior flags pminsize)
	      (aref vector 5) (wm-size-hints-min-width hints)
	      (aref vector 6) (wm-size-hints-min-height hints)))
      (when (and (wm-size-hints-max-width hints) (wm-size-hints-max-height hints))
	(setf flags (logior flags pmaxsize)
	      (aref vector 7) (wm-size-hints-max-width hints)
	      (aref vector 8) (wm-size-hints-max-height hints)))
      (when (and (wm-size-hints-width-inc hints) (wm-size-hints-height-inc hints))
	(setf flags (logior flags presizeinc)
	      (aref vector 9) (wm-size-hints-width-inc hints)
	      (aref vector 10) (wm-size-hints-height-inc hints)))
      (let ((min-aspect (wm-size-hints-min-aspect hints))
	    (max-aspect (wm-size-hints-max-aspect hints)))
	(when (and min-aspect max-aspect)
	  (setf flags (logior flags paspect)
		min-aspect (rationalize min-aspect)
		max-aspect (rationalize max-aspect)
		(aref vector 11) (numerator min-aspect)
		(aref vector 12) (denominator min-aspect)
		(aref vector 13) (numerator max-aspect)
		(aref vector 14) (denominator max-aspect))))
      (setf (aref vector 0) flags)
      vector)))

;;-----------------------------------------------------------------------------
;; Icon_Size

;; Use the same intermediate structure as WM_SIZE_HINTS

(defun icon-sizes (window)
  (declare (type window window))
  (declare-values wm-size-hints)
  (let ((vector (get-property window :wm_icon_size :type :wm_icon_size :result-type 'vector)))
    (declare (type (or null (simple-vector 6)) vector))
    (when vector
      (make-wm-size-hints
	:min-width (aref vector 0)
	:min-height (aref vector 1)
	:max-width (aref vector 2)
	:max-height (aref vector 3)
	:width-inc (aref vector 4)
	:height-inc (aref vector 5)))))
  
(defsetf icon-sizes set-icon-sizes)
(defun set-icon-sizes (window wm-size-hints)
  (declare (type window window)
	   (type wm-size-hints wm-size-hints))
  (let ((vector (vector (wm-size-hints-min-width wm-size-hints)
			(wm-size-hints-min-height wm-size-hints)
			(wm-size-hints-max-width wm-size-hints)
			(wm-size-hints-max-height wm-size-hints)
			(wm-size-hints-width-inc wm-size-hints)
			(wm-size-hints-height-inc wm-size-hints))))
    (change-property window :wm_icon_size vector :wm_icon_size 32)
    wm-size-hints))

;;-----------------------------------------------------------------------------
;; Transient-For

(defun transient-for (window)
  (let ((prop (get-property window :wm_transient_for :type :window :result-type 'list)))
    (and prop (lookup-window (window-display window) (car prop)))))

(defsetf transient-for set-transient-for)
(defun set-transient-for (window transient)
  (declare (type window window transient))
  (change-property window :wm_transient_for (list (window-id transient)) :window 32)
  transient)

;;-----------------------------------------------------------------------------
;; Set-Standard-Properties

(defun set-standard-properties (window &rest options &key 
				name icon-name resource-name resource-class command
				client-machine hints normal-hints zoom-hints
				;; the following are used for wm-normal-hints
				user-specified-position-p
				user-specified-size-p
				x y width height min-width min-height max-width max-height
				width-inc height-inc min-aspect max-aspect
				;; the following are used for wm-hints
				input initial-state icon-pixmap icon-window
				icon-x icon-y icon-mask window-group)
  ;; Set properties for WINDOW.
  (declare (type window window)
	   (type (or null stringable) name icon-name resource-name resource-class client-machine)
	   (type (or null list) command)
	   (type (or null wm-hints) hints)
	   (type (or null wm-size-hints) normal-hints zoom-hints)
	   (type (or null boolean) user-specified-position-p user-specified-size-p)
	   (type (or null int16) x y)
	   (type (or null card16) width height min-width min-height max-width max-height width-inc height-inc)
	   (type (or null number) min-aspect max-aspect)
	   (type (or null (member :off :on)) input)
	   (type (or null (member :dont-care :normal :zoom :iconic :inactive)) initial-state)
	   (type (or null pixmap) icon-pixmap icon-mask)
	   (type (or null window) icon-window)
	   (type (or null card16) icon-x icon-y)
	   (type (or null resource-id) window-group))
  (when name (setf (wm-name window) name))
  (when icon-name (setf (wm-icon-name window) icon-name))
  (when client-machine (setf (wm-client-machine window) client-machine))
  (when (or resource-name resource-class)
    (set-wm-class window resource-name resource-class))
  (when command (setf (wm-command window) command))
  ;; WM-HINTS
  (if (dolist (arg '(:input :initial-state :icon-pixmap :icon-window
			    :icon-x :icon-y :icon-mask :window-group))
	(when (getf options arg) (return t)))
      (let ((wm-hints (if hints (copy-wm-hints hints) (make-wm-hints))))
	(when input (setf (wm-hints-input wm-hints) input))
	(when initial-state (setf (wm-hints-initial-state wm-hints) initial-state))
	(when icon-pixmap (setf (wm-hints-icon-pixmap wm-hints) icon-pixmap))
	(when icon-window (setf (wm-hints-icon-window wm-hints) icon-window))
	(when icon-x (setf (wm-hints-icon-x wm-hints) icon-x))
	(when icon-y (setf (wm-hints-icon-y wm-hints) icon-y))
	(when icon-mask (setf (wm-hints-icon-mask wm-hints) icon-mask))
	(when window-group (setf (wm-hints-input wm-hints) window-group))
	(setf (wm-hints window) wm-hints))
    (when hints (setf (wm-hints window) hints)))
  ;; WM-NORMAL-HINTS
  (if (dolist (arg '(:x :y :width :height :min-width :min-height :max-width :max-height
			:width-inc :height-inc :min-aspect :max-aspect
			:user-specified-position-p :user-specified-size-p))
	(when (getf options arg) (return t)))
      (let ((size (if normal-hints (copy-wm-size-hints normal-hints) (make-wm-size-hints))))
	(when x (setf (wm-size-hints-x size) x))
	(when y (setf (wm-size-hints-y size) y))
	(when width (setf (wm-size-hints-width size) width))
	(when height (setf (wm-size-hints-height size) height))
	(when min-width (setf (wm-size-hints-min-width size) min-width))
	(when min-height (setf (wm-size-hints-min-height size) min-height))
	(when max-width (setf (wm-size-hints-max-width size) max-width))
	(when max-height (setf (wm-size-hints-max-height size) max-height))
	(when width-inc (setf (wm-size-hints-width-inc size) width-inc))
	(when height-inc (setf (wm-size-hints-height-inc size) height-inc))
	(when min-aspect (setf (wm-size-hints-min-aspect size) min-aspect))
	(when max-aspect (setf (wm-size-hints-max-aspect size) max-aspect))
	(when user-specified-position-p (setf (wm-size-hints-user-specified-position-p size)
					      user-specified-position-p))
	(when user-specified-size-p (setf (wm-size-hints-user-specified-size-p size)
					  user-specified-size-p))
	(setf (wm-normal-hints window) size))
    (when normal-hints (setf (wm-normal-hints window) normal-hints)))
  (when zoom-hints (setf (wm-zoom-hints window) zoom-hints))
  )

;;-----------------------------------------------------------------------------
;; Colormaps

(defun get-standard-colormap (window property)
  (declare (type window window)
	   (type (member :rgb_default_map :rgb_best_map :rgb_red_map
			 :rgb_green_map :rgb_blue_map) property))
  (declare-values colormap base-pixel max-color mult-color)
  (let ((prop (get-property window property :type :rgb_color_map :result-type 'vector)))
    (declare (type (or null (simple-vector 8)) prop))
    (when prop
      (values (lookup-colormap (window-display window) (aref prop 0))
	      (aref prop 7)			;Base Pixel
	      (make-color :red   (card16->rgb-val (aref prop 1))	;Max Color
			  :green (card16->rgb-val (aref prop 3))
			  :blue  (card16->rgb-val (aref prop 5)))
	      (make-color :red   (card16->rgb-val (aref prop 2))	;Mult color
			  :green (card16->rgb-val (aref prop 4))
			  :blue  (card16->rgb-val (aref prop 6)))
	      ))))

(defun set-standard-colormap (window property colormap base-pixel max-color mult-color)
  (declare (type window window)
	   (type (member :rgb_default_map :rgb_best_map :rgb_red_map
			 :rgb_green_map :rgb_blue_map) property)
	   (type colormap colormap)
	   (type pixel base-pixel)
	   (type color max-color mult-color))
  (let ((prop (vector (encode-type colormap colormap)
		      (encode-type rgb-val (color-red max-color))
		      (encode-type rgb-val (color-red mult-color))
		      (encode-type rgb-val (color-green max-color))
		      (encode-type rgb-val (color-green mult-color))
		      (encode-type rgb-val (color-blue max-color))
		      (encode-type rgb-val (color-blue mult-color))
		      base-pixel)))
    (change-property window property prop :rgb_color_map 32)))

;;-----------------------------------------------------------------------------
;; Cut-Buffers

(defun cut-buffer (display &key (buffer 0) (type :string) (result-type 'string)
		   (transform #'card8->char) (start 0) end)
  ;; Return the contents of cut-buffer BUFFER
  (declare (type display display)
	   (type (integer 0 7) buffer)
	   (type xatom type)
	   (type array-index start)
	   (type (or null array-index) end)
	   (type t result-type)			;a sequence type
	   (type (or null (function (integer) t)) transform))
  (declare-values sequence type format bytes-after)
  (let* ((root (screen-root (first (display-roots display))))
	 (property (aref '#(:cut_buffer0 :cut_buffer1 :cut_buffer2 :cut_buffer3
			    :cut_buffer4 :cut_buffer5 :cut_buffer6 :cut_buffer7)
			 buffer)))
    (get-property root property :type type :result-type result-type
		  :start start :end end :transform transform)))

;; Implement the following:
;; (defsetf cut-buffer (display &key (buffer 0) (type :string) (format 8)
;;			        (transform #'char->card8) (start 0) end) (data)
;; In order to avoid having to pass positional parameters to set-cut-buffer,
;; We've got to do the following.  WHAT A PAIN...
(define-setf-method cut-buffer (display &rest option-list)
  (do* ((options (copy-list option-list))
	(option options (cddr option))
	(store (gensym))
	(dtemp (gensym))
	(temps (list dtemp))
	(values (list display)))
       ((endp option)
	(values (nreverse temps)
		(nreverse values)
		(list store)
		`(set-cut-buffer ,store ,dtemp ,@options)
		`(cut-buffer ,@options)))
    (unless (member (car option) '(:buffer :type :format :start :end :transform))
      (error "Keyword arg ~s isn't recognized" (car option)))
    (let ((x (gensym)))
      (push x temps)
      (push (cadr option) values)
      (setf (cadr option) x))))

(defun set-cut-buffer (data display &key (buffer 0) (type :string) (format 8)
		       (start 0) end (transform #'char->card8))
  (declare (type sequence data)
	   (type display display)
	   (type (integer 0 7) buffer)
	   (type xatom type)
	   (type (member 8 16 32) format)
	   (type array-index start)
	   (type (or null array-index) end)
	   (type (or null (function (integer) t)) transform))
  (let* ((root (screen-root (first (display-roots display))))
	 (property (aref '#(:cut_buffer0 :cut_buffer1 :cut_buffer2 :cut_buffer3
			    :cut_buffer4 :cut_buffer5 :cut_buffer6 :cut_buffer7)
			 buffer)))
    (change-property root property data type format :transform transform :start start :end end)
    data))

(defun rotate-cut-buffers (display &optional (delta 1) (careful-p t))
  ;; Positive rotates left, negative rotates right (opposite of actual protocol request).
  ;; When careful-p, ensure all cut-buffer properties are defined, to prevent errors.
  (declare (type display display)
	   (type int16 delta)
	   (type boolean careful-p))
  (let* ((root (screen-root (first (display-roots display))))
	 (buffers '#(:cut_buffer0 :cut_buffer1 :cut_buffer2 :cut_buffer3
		     :cut_buffer4 :cut_buffer5 :cut_buffer6 :cut_buffer7)))
    (when careful-p
      (let ((props (list-properties root)))
	(dotimes (i 8)
	  (unless (member (aref buffers i) props)
	    (setf (cut-buffer display :buffer i) "")))))
    (rotate-properties root buffers delta)))

