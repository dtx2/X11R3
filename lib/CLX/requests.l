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

(export '(create-window
	  destroy-window
	  destroy-subwindows
	  add-to-save-set
	  remove-from-save-set
	  reparent-window
	  map-window
	  map-subwindows
	  unmap-window
	  unmap-subwindows
	  circulate-window-up
	  circulate-window-down
	  query-tree
	  intern-atom
	  find-atom
	  atom-name
	  change-property
	  delete-property
	  get-property
	  rotate-properties
	  list-properties
	  set-selection-owner
	  selection-owner
	  selection-owner
	  convert-selection
	  send-event
	  grab-pointer
	  ungrab-pointer
	  grab-button
	  ungrab-button
	  change-active-pointer-grab
	  grab-keyboard
	  ungrab-keyboard
	  grab-key
	  ungrab-key
	  allow-events
	  grab-server
	  ungrab-server
	  with-server-grabbed
	  query-pointer
	  pointer-position
	  global-pointer-position
	  motion-events
	  translate-coordinates
	  warp-pointer
	  warp-pointer-relative
	  warp-pointer-if-inside
	  warp-pointer-relative-if-inside
	  set-input-focus
	  input-focus
	  query-keymap
	  create-pixmap
	  free-pixmap
	  clear-area
	  copy-area
	  copy-plane
	  create-colormap
	  free-colormap
	  copy-colormap-and-free
	  install-colormap
	  uninstall-colormap
	  installed-colormaps
	  alloc-color
	  alloc-color-cells
	  alloc-color-planes
	  free-colors
	  store-color
	  store-colors
	  query-colors
	  lookup-color
	  create-cursor
	  create-glyph-cursor
	  free-cursor
	  recolor-cursor
	  query-best-cursor
	  query-best-tile
	  query-best-stipple
	  query-extension
	  list-extensions
	  change-keyboard-control
	  keyboard-control
	  bell
	  pointer-mapping
	  set-pointer-mapping
	  pointer-mapping
	  change-pointer-control
	  pointer-control
	  set-screen-saver
	  screen-saver
	  activate-screen-saver
	  reset-screen-saver
	  add-access-host
	  remove-access-host
	  access-hosts
	  access-control
	  set-access-control
	  access-control
	  close-down-mode
	  set-close-down-mode
	  kill-client
	  kill-temporary-clients
;;	  NO-OPERATION
	  ))

(defun create-window (&key
		      (parent (required-arg parent))
		      (x (required-arg x))
		      (y (required-arg y))
		      (width (required-arg width))
		      (height (required-arg height))
		      (depth 0) (border-width 0)
		      (class :copy) (visual :copy)
		      background border
		      bit-gravity gravity
		      backing-store backing-planes backing-pixel save-under
		      event-mask do-not-propagate-mask override-redirect
		      colormap cursor)
  ;; Display is obtained from parent.  Only non-nil attributes are passed on in
  ;; the request: the function makes no assumptions about what the actual protocol
  ;; defaults are.  Width and height are the inside size, excluding border.
  (declare (type window parent) ; required
	   (type int16 x y) ;required
	   (type card16 width height) ;required
	   (type card16 depth border-width)
	   (type (member :copy :input-output :input-only) class)
	   (type (or (member :copy) card29) visual)
	   (type (or null (member :none :parent-relative) pixel pixmap) background)
	   (type (or null (member :copy) pixel pixmap) border)
	   (type (or null bit-gravity) bit-gravity)
	   (type (or null win-gravity) gravity)
	   (type (or null (member :not-useful :when-mapped :always)) backing-store)
	   (type (or null pixel) backing-planes backing-pixel)
	   (type (or null event-mask) event-mask)
	   (type (or null device-event-mask) do-not-propagate-mask)
	   (type (or null (member :on :off)) save-under override-redirect)
	   (type (or null (member :copy) colormap) colormap)
	   (type (or null (member :none) cursor) cursor))
  (declare-values window)
  (let* ((display (window-display parent))
	 (window (make-window :display display))
	 (wid (allocate-resource-id display window 'window))
	 back-pixmap back-pixel
	 border-pixmap border-pixel)
    (declare (type display display)
	     (type window window)
	     (type resource-id wid)
	     (type (or null resource-id) back-pixmap border-pixmap)
	     (type (or null pixel) back-pixel border-pixel))
    (setf (window-id window) wid)
    (case background
      ((nil) nil)
      (:none (setq back-pixmap 0))
      (:parent-relative (setq back-pixmap 1))
      (otherwise
       (if (type? background 'pixmap)
	   (setq back-pixmap (pixmap-id background))
	 (if (integerp background)
	     (setq back-pixel background)
	   (x-type-error background
			 '(or null (member :none :parent-relative) integer pixmap))))))
    (case border
      ((nil) nil)
      (:copy (setq border-pixmap 1))
      (otherwise
       (if (type? border 'pixmap)
	   (setq border-pixmap (pixmap-id border))
	 (if (integerp border)
	     (setq border-pixel border)
	   (x-type-error border '(or null (member :copy) integer pixmap))))))
    (when event-mask
      (setq event-mask (encode-event-mask event-mask)))
    (when do-not-propagate-mask
      (setq do-not-propagate-mask (encode-device-event-mask do-not-propagate-mask)))

						;Make the request
    (with-buffer-request (display *x-createwindow*)
      (data depth)
      (resource-id wid)
      (window parent)
      (int16 x y)
      (card16 width height border-width)
      ((member16 :copy :input-output :input-only) class)
      (resource-id (if (eq visual :copy) 0 visual))
      (mask ((or null card32) back-pixmap back-pixel border-pixmap border-pixel)
	    ((or null (member-vector *bit-gravity-vector*)) bit-gravity)
	    ((or null (member-vector *win-gravity-vector*)) gravity)
	    ((or null (member :not-useful :when-mapped :always)) backing-store)
	    ((or null card32)  backing-planes backing-pixel)
	    ((or null (member :off :on)) override-redirect save-under)
	    ((or null card32) event-mask do-not-propagate-mask)
	    ((or null (member %error :copy) colormap) colormap)
	    ((or null (member :none) cursor) cursor)))
    window))

(defun destroy-window (window)
  (declare (type window window))
  (with-buffer-request ((window-display window) *x-destroywindow*)
    (window window)))

(defun destroy-subwindows (window)
  (declare (type window window))
  (with-buffer-request ((window-display window) *x-destroysubwindows*)
    (window window)))

(defun add-to-save-set (window)
  (declare (type window window))
  (with-buffer-request ((window-display window) *x-changesaveset*)
    (data 0)
    (window window)))

(defun remove-from-save-set (window)
  (declare (type window window))
  (with-buffer-request ((window-display window) *x-changesaveset*)
    (data 1)
    (window window)))

(defun reparent-window (window parent x y)
  (declare (type window window parent)
	   (type int16 x y))
  (with-buffer-request ((window-display window) *x-reparentwindow*)
    (window window parent)
    (int16 x y)))

(defun map-window (window)
  (declare (type window window))
  (with-buffer-request ((window-display window) *x-mapwindow*)
    (window window)))

(defun map-subwindows (window)
  (declare (type window window))
  (with-buffer-request ((window-display window) *x-mapsubwindows*)
    (window window)))

(defun unmap-window (window)
  (declare (type window window))
  (with-buffer-request ((window-display window) *x-unmapwindow*)
    (window window)))

(defun unmap-subwindows (window)
  (declare (type window window))
  (with-buffer-request ((window-display window) *x-unmapsubwindows*)
    (window window)))

(defun circulate-window-up (window)
  (declare (type window window))
  (with-buffer-request ((window-display window) *x-circulatewindow*)
    (data 0)
    (window window)))

(defun circulate-window-down (window)
  (declare (type window window))
  (with-buffer-request ((window-display window) *x-circulatewindow*)
    (data 1)
    (window window)))

(defun query-tree (window &key (result-type 'list))
  (declare (type window window)
	   (type t result-type)) ;;type specifier
  (declare-values (sequence window) parent root)
  (let ((display (window-display window))
	sequence parent root)
    (with-display (display)
      (with-buffer-request (display *x-querytree* :no-after)
	(window window))
      (wait-for-reply display nil)
      (reading-buffer-reply (display :sizes (8 16 32))
	(let ((nchildren (card16-get 16)))
	  (setq root (window-get 8)
		parent (resource-id-get 12)
		sequence (sequence-get :length nchildren :result-type result-type))
	  ;; Parent is NIL for root window
	  (setq parent (and (plusp parent) (lookup-window display parent)))
	  (dotimes (i nchildren)		; Convert ID's to window's
	    (setf (elt sequence i) (lookup-window display (elt sequence i)))))))
    (display-invoke-after-function display)
    (values sequence parent root)))

;; Although atom-ids are not visible in the normal user interface, atom-ids might
;; appear in window properties and other user data, so conversion hooks are needed.

(defun intern-atom (display name)
  (declare (type display display)
	   (type xatom name))
  (declare-values card29)
  (or (atom-id name display)
      (let ((string (string name))
	    id)
	(with-display (display)
	  (with-buffer-request (display *x-internatom* :no-after)
	    (data 0)
	    (card16 (length string))
	    (pad16 nil)
	    (string string))
	  (with-buffer-reply (display 12 :sizes 32)
	    (setq id (resource-id-get 8)))
	  (let ((keyword (if (keywordp name) name (kintern string))))
	    (setf (atom-id keyword display) id)
	    (save-id display id keyword)))
	(display-invoke-after-function display)
	id)))

(defun find-atom (display name)
  ;; Same as INTERN-ATOM, but with the ONLY-IF-EXISTS flag True
  (declare (type display display)
	   (type xatom name))
  (declare-values (or null card29))
  (or (atom-id name display)
      (let ((string (string name))
	    id)
	(with-display (display)
	  (with-buffer-request (display *x-internatom* :no-after)
	    (data 1)
	    (card16 (length string))
	    (pad16 nil)
	    (string string))
	  (with-buffer-reply (display 12 :sizes 32)
	    (setq id (or-get 8 null resource-id)))
	  (when id 
	    (let ((keyword (if (keywordp name) name (kintern string))))
	      (setf (atom-id keyword display) id)
	      (save-id display id keyword))))
	(display-invoke-after-function display)
	id)))

;; Use LOOKUP-XATOM instead
(proclaim '(inline atom-name))
(defun atom-name (display atom-id)
  (declare (type display display)
	   (type card29 atom-id))
  (declare-values keyword)
  (lookup-xatom display atom-id))

(defun atom-name-internal (display atom-id)
  ;; Called only by LOOKUP-XATOM
  (declare (type display display)
	   (type card29 atom-id)
	   (values keyword))
  (let (keyword)
    (with-display (display)
      (with-buffer-request (display *x-getatomname* :no-after)
	(card29 atom-id))
      (with-buffer-reply (display nil :sizes (16))
	(setq keyword (string-get (card16-get 8))))
      (setq keyword (kintern keyword))
      (setf (atom-id keyword display) atom-id))
    (display-invoke-after-function display)
    keyword))

(defun change-property (window property data type format
		       &key (mode :replace) (start 0) end transform)
  ; Start and end affect sub-sequence extracted from data.
  ; Transform is applied to each extracted element.
  (declare (type window window)
	   (type xatom property type)
	   (type (member 8 16 32) format)
	   (type sequence data)
	   (type (member :replace :prepend :append) mode)
	   (type array-index start)
	   (type (or null array-index) end)
	   (type t transform))			;(or null (function (t) integer))
  (unless end (setq end (length data)))
  (let* ((display (window-display window))
	 (length (- end start))
	 (property-id (intern-atom display property))
	 (type-id (intern-atom display type)))
    (declare (type display display)
	     (type array-index length)
	     (type resource-id property-id type-id))
    (with-buffer-request (display *x-changeproperty*)
      ((data (member :replace :prepend :append)) mode)
      (window window)
      (resource-id property-id type-id)
      (card8 format)
      (card32 length)
      (progn
	(ecase format
	  (8  (sequence-put 24 data :format card8
			    :start start :end end :transform transform))
	  (16 (sequence-put 24 data :format card16
			    :start start :end end :transform transform))
	  (32 (sequence-put 24 data :format card32
			    :start start :end end :transform transform)))))))

(defun delete-property (window property)
  (declare (type window window)
	   (type xatom property))
  (let* ((display (window-display window))
	 (property-id (intern-atom display property)))
    (declare (type display display)
	     (type resource-id property-id))
    (with-buffer-request (display *x-deleteproperty*)
      (window window)
      (resource-id property-id))))

(defun get-property (window property
		     &key type (start 0) end delete-p (result-type 'list) transform)
  ;; Transform is applied to each integer retrieved.
  (declare (type window window)
	   (type xatom property)
	   (type (or null xatom) type)
	   (type array-index start)
	   (type (or null array-index) end)
	   (type boolean delete-p)
	   (type t result-type)			;a sequence type
	   (type t transform))			;(or null (function (integer) t))
  (declare-values data (or null type) format bytes-after)
  (let* ((display (window-display window))
	 (data nil)
	 (property-id (intern-atom display property))
	 (type-id (and type (intern-atom display type)))
	 reply-type reply-format bytes-after)
    (declare (type display display)
	     (type resource-id property-id)
	     (type (or null resource-id) type-id))
    (with-display (display)
      (with-buffer-request (display *x-getproperty* :no-after)
	((data boolean) delete-p)
	(window window)
	(resource-id property-id)
	((or null resource-id) type-id)
	(card32 start)
	(card32 (- (or end 64000) start)))
      (with-buffer-reply (display nil :sizes (8 32))
	(setq reply-format (card8-get 1)
	      reply-type (card32-get 8)
	      bytes-after (card32-get 12))
	(let ((nitems (card32-get 16)))
	  (when (plusp nitems)
	    (setq data
		  (ecase reply-format
		    (0  nil) ;; (make-sequence result-type 0)) ;; Property not found.
		    (8  (sequence-get :result-type result-type :format card8
				      :length nitems :transform transform))
		    
		    (16 (sequence-get :result-type result-type :format card16
				      :length nitems :transform transform))
		    
		    (32 (sequence-get :result-type result-type :format card32
				      :length nitems :transform transform))))))))
    (display-invoke-after-function display)
    (values data (and (plusp reply-type) (lookup-xatom display reply-type))
	    reply-format bytes-after)))

(defun rotate-properties (window properties &optional (delta 1))
  ;; Positive rotates left, negative rotates right (opposite of actual protocol request).
  (declare (type window window)
	   (type sequence properties) ;; sequence of xatom
	   (type int16 delta))
  (let* ((display (window-display window))
	 (length (length properties))
	 (sequence (make-array length)))
    (declare (type display display)
	     (type array-index length))
    (with-vector (sequence vector)
      (with-display (display)
	;; Atoms must be interned before the RotateProperties request
	;; is started to allow InternAtom requests to be made.
	(dotimes (i length)
	  (setf (aref sequence i) (intern-atom display (elt properties i))))
	(with-buffer-request (display *x-rotateproperties*)
	  (window window)
	  (card16 length)
	  (int16 (- delta))
	  ((sequence :end length) sequence))
	nil))))

(defun list-properties (window &key (result-type 'list))
  (declare (type window window)
	   (type t result-type)) ;; a sequence type
  (declare-values (sequence keyword))
  (let ((display (window-display window))
	seq)
    (with-display (display)
      (with-buffer-request (display *x-listproperties* :no-after)
	(window window))
      (with-buffer-reply (display nil :sizes 16)
	(let ((nproperties (card16-get 8)))
	  (setq seq (sequence-get :result-type result-type :length nproperties)))))
    (display-invoke-after-function display)
    ;; lookup the atoms in the sequence
    (if (listp seq)
	(do ((elt seq (cdr elt)))
	    ((endp elt) seq)
	  (setf (car elt) (lookup-xatom display (car elt))))
      (dotimes (i (length seq) seq)
	(setf (aref seq i) (lookup-xatom display (aref seq i)))))))

(defun selection-owner (display selection)
  (declare (type display display)
	   (type xatom selection))
  (declare-values (or null window))
  (let ((selection-id (intern-atom display selection))
	window)
    (declare (type resource-id selection-id))
    (with-display (display)
      (with-buffer-request (display *x-getselectionowner* :no-after)
	(resource-id selection-id))
      (with-buffer-reply (display 12 :sizes 32)
	(setq window (resource-id-or-nil-get 8)))
      (when window
	(setq window (lookup-window display window))))
    (display-invoke-after-function display)
    window))

(defun set-selection-owner (display selection owner &optional time)
  (declare (type display display)
	   (type xatom selection)
	   (type (or null window) owner)
	   (type timestamp time))
  (let ((selection-id (intern-atom display selection)))
    (declare (type resource-id selection-id))
    (with-buffer-request (display *x-setselectionowner*)
      ((or null window) owner)
      (resource-id selection-id)
      ((or null card32) time))
    owner))

(defsetf selection-owner (display selection &optional time) (owner)
  ;; A bit strange, but retains setf form.
  `(set-selection-owner ,display ,selection ,owner ,time))

(defun convert-selection (selection type requestor &optional property time)
  (declare (type xatom selection type)
	   (type window requestor)
	   (type (or null xatom) property)
	   (type timestamp time))
  (let* ((display (window-display requestor))
	 (selection-id (intern-atom display selection))
	 (type-id (intern-atom display type))
	 (property-id (and property (intern-atom display property))))
    (declare (type display display)
	     (type resource-id selection-id type-id)
	     (type (or null resource-id) property-id))
    (with-buffer-request (display *x-convertselection*)
      (window requestor)
      (resource-id selection-id type-id)
      ((or null resource-id) property-id)
      ((or null card32) time))))

(defun send-event (window event-key event-mask &rest args
		   &key propagate-p display &allow-other-keys)
  ;; Additional arguments depend on event-key, and are as specified further below
  ;; with declare-event, except that both resource-ids and resource objects are
  ;; accepted in the event components.  The display argument is only required if the
  ;; window is :pointer-window or :input-focus.
  (declare (type (or window (member :pointer-window :input-focus)) window)
	   (type event-key event-key)
	   (type (or null event-mask) event-mask)
	   (type boolean propagate-p)
	   (type (or null display) display)
	   (special *event-send-vector*))
  (unless event-mask (setq event-mask 0))
  (unless display (setq display (window-display window)))
  (let ((internal-event-code (get-event-code event-key))
	(external-event-code (get-external-event-code display event-key)))
    (declare (type card8 internal-event-code external-event-code))
    (with-display (display)
      ;; Ensure keyword atom-id's are cached
      (dolist (arg (cdr (assoc event-key '((:property-notify :atom)
					   (:selection-clear :selection)
					   (:selection-request :selection :target :property)
					   (:selection-notify :selection :target :property))
			       :test #'eq)))
	(let ((keyword (getf args arg)))
	  (intern-atom display keyword)))
      ;; Make the sendevent request
      (with-buffer-request (display *x-sendevent*)
	((data boolean) propagate-p)
	(length 11) ;; 3 word request + 8 words for event = 11
	((or (member :pointer-window :input-focus) window) window)
	(card32 (encode-event-mask event-mask))
	(card8 external-event-code)
	(progn
	  (apply (aref *event-send-vector* internal-event-code) display args)
	  (incf (buffer-boffset display) 44))))))

(defun grab-pointer (window event-mask
		     &key owner-p sync-pointer-p sync-keyboard-p confine-to cursor time)
  (declare (type window window)
	   (type pointer-event-mask event-mask)
	   (type boolean owner-p sync-pointer-p sync-keyboard-p)
	   (type (or null window) confine-to)
	   (type (or null cursor) cursor)
	   (type timestamp time))
  (declare-values grab-status)
  (let ((display (window-display window))
	grab-status)
    (with-display (display)
      (with-buffer-request (display *x-grabpointer* :no-after)
	((data boolean) owner-p)
	(window window)
	(card16 (encode-pointer-event-mask event-mask))
	(boolean (not sync-pointer-p) (not sync-keyboard-p))
	((or null window) confine-to)
	((or null cursor) cursor)
	((or null card32) time)
	)
      (with-buffer-reply (display nil :sizes 8)
	(setq grab-status (member8-get 1 :success :already-grabbed
					 :invalid-time :not-viewable :frozen))))
    (display-invoke-after-function display)
    grab-status))

(defun ungrab-pointer (display &key time)
  (declare (type timestamp time))
  (with-buffer-request (display *x-ungrabpointer*)
    ((or null card32) time)))

(defun grab-button (window button event-mask
		    &key (modifiers 0)
			 owner-p sync-pointer-p sync-keyboard-p confine-to cursor)
  (declare (type window window)
	   (type (or (member :any) card8) button)
	   (type modifier-mask modifiers)
	   (type pointer-event-mask event-mask)
	   (type boolean owner-p sync-pointer-p sync-keyboard-p)
	   (type (or null window) confine-to)
	   (type (or null cursor) cursor))
  (with-buffer-request ((window-display window) *x-grabbutton*)
    ((data boolean) owner-p)
    (window window)
    (card16 (encode-pointer-event-mask event-mask))
    (boolean (not sync-pointer-p) (not sync-keyboard-p))
    ((or null window) confine-to)
    ((or null cursor) cursor)
    (card8 (if (eq button :any) 0 button))
    (pad8 1)
    (card16 (encode-modifier-mask modifiers))
    ))

(defun ungrab-button (window button &key (modifiers 0))
  (declare (type window window)
	   (type (or (member :any) card8) button)
	   (type modifier-mask modifiers))
  (with-buffer-request ((window-display window) *x-ungrabbutton*)
    (data (if (eq button :any) 0 button))
    (window window)
    (card16 (encode-modifier-mask modifiers))))

(defun change-active-pointer-grab (display event-mask &optional cursor time)
  (declare (type display display)
	   (type pointer-event-mask event-mask)
	   (type (or null cursor) cursor)
	   (type timestamp time))
  (with-buffer-request (display *x-changeactivepointergrab*)
    ((or null cursor) cursor)
    ((or null card32) time)
    (card16 (encode-pointer-event-mask event-mask))))

(defun grab-keyboard (window &key owner-p sync-pointer-p sync-keyboard-p time)
  (declare (type window window)
	   (type boolean owner-p sync-pointer-p sync-keyboard-p)
	   (type timestamp time))
  (declare-values grab-status)
  (let ((display (window-display window))
	grab-status)
    (with-display (display)
      (with-buffer-request (display *x-grabkeyboard* :no-after)
	((data boolean) owner-p)
	(window window)
	((or null card32) time)
	(boolean (not sync-pointer-p) (not sync-keyboard-p)))
      (with-buffer-reply (display nil :sizes 8)
	(setq grab-status (member8-get 1 :success :already-grabbed
					 :invalid-time :not-viewable :frozen))))
    (display-invoke-after-function display)
    grab-status))

(defun ungrab-keyboard (display &key time)
  (declare (type display display)
	   (type timestamp time))
  (with-buffer-request (display *x-ungrabkeyboard*)
    ((or null card32) time)))

(defun grab-key (window key &key (modifiers 0) owner-p sync-pointer-p sync-keyboard-p)
  (declare (type window window)
	   (type boolean owner-p sync-pointer-p sync-keyboard-p)
	   (type (or (member :any) card8) key)
	   (type modifier-mask modifiers))
  (with-buffer-request ((window-display window) *x-grabkey*)
    ((data boolean) owner-p)
    (window window)
    (card16 (encode-modifier-mask modifiers))
    (card8 (if (eq key :any) 0 key))
    (boolean (not sync-pointer-p) (not sync-keyboard-p))
    ))

(defun ungrab-key (window key &key (modifiers 0))
  (declare (type window window)
	   (type (or (member :any) card8) key)
	   (type modifier-mask modifiers))
  (with-buffer-request ((window-display window) *x-ungrabkey*)
    (data (if (eq key :any) 0 key))
    (window window)
    (card16 (encode-modifier-mask modifiers))))

(defun allow-events (display mode &optional time)
  (declare (type display display)
	   (type (member :async-pointer :sync-pointer :replay-pointer
			 :async-keyboard :sync-keyboard :replay-keyboard
			 :async-both :sync-both)
		 mode)
	   (type timestamp time))
  (with-buffer-request (display *x-allowevents*)
    ((data (member :async-pointer :sync-pointer :replay-pointer
		   :async-keyboard :sync-keyboard :replay-keyboard
		   :async-both :sync-both))
     mode)
    ((or null card32) time)))

(defun grab-server (display)
  (declare (type display display))
  (with-buffer-request (display *x-grabserver*)))

(defun ungrab-server (display)
  (with-buffer-request (display *x-ungrabserver*)))

(defmacro with-server-grabbed ((display) &body body)
  ;; The body is not surrounded by a with-display.
  (let ((disp (gensym)))
    `(let ((,disp ,display))
       (unwind-protect
	   (progn
	     (grab-server ,disp)
	     ,@body)
	 (ungrab-server ,disp)))))

(defun query-pointer (window)
  (declare (type window window))
  (declare-values x y same-screen-p child mask root-x root-y root)
  (let ((display (window-display window))
	x y same-screen-p child mask root-x root-y root)
    (with-display (display)
      (with-buffer-request (display *x-querypointer* :no-after)
	(window window))
      (with-buffer-reply (display 26 :sizes (8 16 32))
	(setq same-screen-p (boolean-get 1)
	      root (window-get 8)
	      child (or-get 12 null window)
	      root-x (int16-get 16)
	      root-y (int16-get 18)
	      x (int16-get 20)
	      y (int16-get 22)
	      mask (card16-get 24))))
    (display-invoke-after-function display)
    (values x y same-screen-p child mask root-x root-y root)))

(defun pointer-position (window)
  (declare (type window window))
  (declare-values x y same-screen-p)
  (let ((display (window-display window))
	x y same-screen-p)
    (with-display (display)
      (with-buffer-request (display *x-querypointer* :no-after)
	(window window))
      (with-buffer-reply (display 24 :sizes (8 16))
	(setq x (int16-get 20)
	      y (int16-get 22)
	      same-screen-p (boolean-get 1))))
    (display-invoke-after-function display)
    (values x y same-screen-p)))

(defun global-pointer-position (display)
  (declare (type display display))
  (declare-values root-x root-y root)
  (let (root root-x root-y)
    (with-display (display)
      (with-buffer-request (display *x-querypointer* :no-after)
	(window (screen-root (first (display-roots display)))))
      (with-buffer-reply (display 20 :sizes (16 32))
	(setq root (window-get 8)
	      root-x (int16-get 16)
	      root-y (int16-get 18))))
    (display-invoke-after-function display)
    (values root-x root-y root)))

(defun motion-events (window &key start stop (result-type 'list))
  (declare (type window window)
	   (type timestamp start stop)
	   (type t result-type)) ;; a type specifier
  (declare-values (repeat-seq (integer x) (integer y) (timestamp time)))
  (let ((display (window-display window))
	seq)
    (with-display (display)
      (with-buffer-request (display *x-getmotionevents* :no-after)
	(window window)
	((or null card32) start stop))
      (with-buffer-reply (display nil :sizes 32)
	(let ((nevents (card32-get 8)))
	  (setq seq (sequence-get :result-type result-type :length (* nevents 3))))))
    (display-invoke-after-function display)
    seq))

(defun translate-coordinates (src src-x src-y dst)
  ;; Returns NIL when not on the same screen
  (declare (type window src)
	   (type int16 src-x src-y)
	   (type window dst))
  (declare-values dst-x dst-y child)
  (let ((display (window-display src))
	dst-x dst-y child)
    (with-display (display)
      (with-buffer-request (display *x-translatecoords* :no-after)
	(window src dst)
	(int16 src-x src-y))
      (with-buffer-reply (display 16 :sizes (8 16 32))
	(when (boolean-get 1)
	  (setq dst-x (int16-get 12)
		dst-y (int16-get 14)
		child (window-get 8)))))
    (display-invoke-after-function display)
    (values dst-x dst-y child)))

(defun warp-pointer (dst dst-x dst-y)
  (declare (type window dst)
	   (type int16 dst-x dst-y))
  (with-buffer-request ((window-display dst) *x-warppointer*)
    (resource-id 0) ;; None
    (window dst)
    (int16 0 0)
    (card16 0 0)
    (int16 dst-x dst-y)))

(defun warp-pointer-relative (display x-off y-off)
  (declare (type display display)
	   (type int16 x-off y-off))
  (with-buffer-request (display *x-warppointer*)
    (resource-id 0) ;; None
    (resource-id 0) ;; None
    (int16 0 0)
    (card16 0 0)
    (int16 x-off y-off)))

(defun warp-pointer-if-inside (dst dst-x dst-y src src-x src-y
			       &optional src-width src-height)
  ;; Passing in a zero src-width or src-height is a no-op.
  ;; A null src-width or src-height translates into a zero value in the protocol request.
  (declare (type window dst src)
	   (type int16 dst-x dst-y src-x src-y)
	   (type (or null card16) src-width src-height))
  (unless (or (eql src-width 0) (eql src-height 0))
    (with-buffer-request ((window-display dst) *x-warppointer*)
      (window src dst)
      (int16 src-x src-y)
      (card16 (or src-width 0) (or src-height 0))
      (int16 dst-x dst-y))))

(defun warp-pointer-relative-if-inside (x-off y-off src src-x src-y
					&optional src-width src-height)
  ;; Passing in a zero src-width or src-height is a no-op.
  ;; A null src-width or src-height translates into a zero value in the protocol request.
  (declare (type window src)
	   (type int16 x-off y-off src-x src-y)
	   (type (or null card16) src-width src-height))
  (unless (or (eql src-width 0) (eql src-height 0))
    (with-buffer-request ((window-display src) *x-warppointer*)
      (window src)
      (resource-id 0) ;; None
      (int16 src-x src-y)
      (card16 (or src-width 0) (or src-height 0))
      (int16 x-off y-off))))

(defun set-input-focus (display focus revert-to &optional time)
  (declare (type display display)
	   (type (or (member :none :pointer-root) window) focus)
	   (type (member :none :parent :pointer-root) revert-to)
	   (type timestamp time))
  (with-buffer-request (display *x-setinputfocus*)
    ((data (member :none :parent :pointer-root)) revert-to)
    ((or window (member :none :pointer-root)) focus)
    ((or null card32) time)))

(defun input-focus (display)
  (declare (type display display))
  (declare-values focus revert-to)
  (let (focus revert-to)
    (with-display (display)
      (with-buffer-request (display *x-getinputfocus* :no-after))
      (with-buffer-reply (display 16 :sizes (8 32))
	(setq focus (or-get 8 (member :none :pointer-root) window)
	      revert-to (member8-get 1 :none :pointer-root :parent))))
    (display-invoke-after-function display)
    (values focus revert-to)))

(defun query-keymap (display &optional bit-vector)
  (declare (type display display)
	   (type (or null (bit-vector 256)) bit-vector))
  (declare-values (bit-vector 256))
  (let (result)
    (with-display (display)
      (with-buffer-request (display *x-querykeymap* :no-after))
      (with-buffer-reply (display 40 :sizes 8)
	(setq result (bit-vector256-get 8 8 bit-vector))))
    (display-invoke-after-function display)
    result))

(defun create-pixmap (&key
		      (width (required-arg width))
		      (height (required-arg height))
		      (depth (required-arg depth))
		      (drawable (required-arg drawable)))
  (declare (type card8 depth) ;; required
	   (type card16 width height) ;; required
	   (type drawable drawable)) ;; required
  (declare-values pixmap)
  (let* ((display (drawable-display drawable))
	 (pixmap (make-pixmap :display display))
	 (pid (allocate-resource-id display pixmap 'pixmap)))
    (setf (pixmap-id pixmap) pid)
    (with-buffer-request (display *x-createpixmap*)
      (data depth)
      (resource-id pid)
      (drawable drawable)
      (card16 width height))
    pixmap))

(defun free-pixmap (pixmap)
  (declare (type pixmap pixmap))
  (let ((display (pixmap-display pixmap)))
    (with-buffer-request (display *x-freepixmap*)
      (pixmap pixmap))
    (deallocate-resource-id display (pixmap-id pixmap) 'pixmap)))

(defun clear-area (window &key (x 0) (y 0) width height exposures-p)
  ;; Passing in a zero width or height is a no-op.
  ;; A null width or height translates into a zero value in the protocol request.
  (declare (type window window)
	   (type int16 x y)
	   (type (or null card16) width height)
	   (type boolean exposures-p))
  (unless (or (eql width 0) (eql height 0))
    (with-buffer-request ((window-display window) *x-cleartobackground*)
      ((data boolean) exposures-p)
      (window window)
      (int16 x y)
      (card16 (or width 0) (or height 0)))))

(defun copy-area (src gcontext src-x src-y width height dst dst-x dst-y)
  (declare (type drawable src dst)
	   (type gcontext gcontext)
	   (type int16 src-x src-y dst-x dst-y)
	   (type card16 width height))
  (with-buffer-request ((drawable-display src) *x-copyarea* :gc-force gcontext)
    (drawable src dst)
    (gcontext gcontext)
    (int16 src-x src-y dst-x dst-y)
    (card16 width height)))

(defun copy-plane (src gcontext plane src-x src-y width height dst dst-x dst-y)
  (declare (type drawable src dst)
	   (type gcontext gcontext)
	   (type pixel plane)
	   (type int16 src-x src-y dst-x dst-y)
	   (type card16 width height))
  (with-buffer-request ((drawable-display src) *x-copyplane* :gc-force gcontext)
    (drawable src dst)
    (gcontext gcontext)
    (int16 src-x src-y dst-x dst-y)
    (card16 width height)
    (card32 plane)))

(defun create-colormap (visual window &optional alloc-p)
  (declare (type card29 visual)
	   (type window window)
	   (type boolean alloc-p))
  (declare-values colormap)
  (let* ((display (window-display window))
	 (colormap (make-colormap :display display))
	 (id (allocate-resource-id display colormap 'colormap)))
    (setf (colormap-id colormap) id)
    (with-buffer-request (display *x-createcolormap*)
      ((data boolean) alloc-p)
      (resource-id id)
      (window window)
      (card29 visual))
    colormap))

(defun free-colormap (colormap)
  (declare (type colormap colormap))
  (let ((display (colormap-display colormap)))
    (with-buffer-request (display *x-freecolormap*)
      (colormap colormap))
    (deallocate-resource-id display (colormap-id colormap) 'colormap)))

(defun copy-colormap-and-free (colormap)
  (declare (type colormap colormap))
  (declare-values colormap)
  (let* ((display (colormap-display colormap))
	 (new-colormap (make-colormap :display display))
	 (id (allocate-resource-id display new-colormap 'colormap)))
    (setf (colormap-id new-colormap) id)
    (with-buffer-request (display *x-copycolormapandfree*)
      (resource-id id)
      (colormap colormap))
    new-colormap))

(defun install-colormap (colormap)
  (declare (type colormap colormap))
  (with-buffer-request ((colormap-display colormap) *x-installcolormap*)
    (colormap colormap)))

(defun uninstall-colormap (colormap)
  (declare (type colormap colormap))
  (with-buffer-request ((colormap-display colormap) *x-uninstallcolormap*)
    (colormap colormap)))

(defun installed-colormaps (window &key (result-type 'list))
  (declare (type window window)
	   (type t result-type)) ;; CL type
  (declare-values (sequence colormap))
  (let ((display (window-display window))
	seq)
    (labels ((get-colormap (id)
			   (or (lookup-resource-id display id)
			       (save-id display id (make-colormap :display display :id id)))))
      (with-display (display)
	(with-buffer-request (display *x-listinstalledcolormaps* :no-after)
	  (window window))
	(with-buffer-reply (display nil :sizes 16)
	  (let ((nmaps (card16-get 8)))
	    (setq seq (sequence-get :result-type result-type :length nmaps :transform #'get-colormap))))))
    (display-invoke-after-function display)
    seq))

(defun alloc-color (colormap color)
  (declare (type colormap colormap)
	   (type (or stringable color) color))
  (declare-values pixel screen-color exact-color)
  (let ((display (colormap-display colormap))
	pixel screen-color exact-color)
    (with-display (display)
      (etypecase color
	(color
	 (with-buffer-request (display *x-alloccolor* :no-after)
	   (colormap colormap)
	   (rgb-val (color-red color)
		    (color-green color)
		    (color-blue color))
	   (pad16 nil))
	 (with-buffer-reply (display 20 :sizes (16 32))
	   (setq pixel (card32-get 16)
		 screen-color (make-color :red (rgb-val-get 8)
					  :green (rgb-val-get 10)
					  :blue (rgb-val-get 12))
		 exact-color color)))
	 (stringable
	  (let* ((string (string color))
		 (length (length string)))
	   (with-buffer-request (display *x-allocnamedcolor* :no-after)
	     (colormap colormap)
	     (card16 length)
	     (pad16 nil)
	     (string string))
	   (with-buffer-reply (display 24 :sizes (16 32))
	     (setq pixel (card32-get 8)
		   screen-color (make-color :red (rgb-val-get 12)
					    :green (rgb-val-get 14)
					    :blue (rgb-val-get 16))
		   exact-color (make-color :red (rgb-val-get 18)
					   :green (rgb-val-get 20)
					   :blue (rgb-val-get 22))))))))
    (display-invoke-after-function display)
    (values pixel screen-color exact-color)))

(defun alloc-color-cells (colormap colors &key (planes 0) contiguous-p (result-type 'list))
  (declare (type colormap colormap)
	   (type card16 colors planes)
	   (type boolean contiguous-p)
	   (type t result-type)) ;; CL type
  (declare-values (sequence pixel) (sequence mask))
  (let ((display (colormap-display colormap))
	pixel-sequence mask-sequence)
    (with-display (display)
      (with-buffer-request (display *x-alloccolorcells* :no-after)
	((data boolean) contiguous-p)
	(colormap colormap)
	(card16 colors planes))
      (with-buffer-reply (display nil :sizes 16)
	(let ((npixels (card16-get 8))
	      (nmasks (card16-get 10)))
	  (setq pixel-sequence 
		(sequence-get :result-type result-type :length npixels))
	  (setq mask-sequence
		(sequence-get :result-type result-type :length nmasks)))))
    (display-invoke-after-function display)
    (values pixel-sequence mask-sequence)))

(defun alloc-color-planes (colormap colors
			   &key (reds 0) (greens 0) (blues 0)
				contiguous-p (result-type 'list))
  (declare (type colormap colormap)
	   (type card16 colors reds greens blues)
	   (type boolean contiguous-p)
	   (type t result-type)) ;; CL type
  (declare-values (sequence pixel) red-mask green-mask blue-mask)
  (let ((display (colormap-display colormap))
	seq red-mask green-mask blue-mask)
    (with-display (display)
      (with-buffer-request (display *x-alloccolorplanes* :no-after)
	((data boolean) contiguous-p)
	(colormap colormap)
	(card16 colors reds greens blues))
      (with-buffer-reply (display nil :sizes (16 32))
	(let ((npixels (card16-get 8)))
	  (setq red-mask (card32-get 12)
		green-mask (card32-get 16)
		blue-mask (card32-get 20)
		seq (sequence-get :result-type result-type :length npixels)))))
    (display-invoke-after-function display)
    (values seq red-mask green-mask blue-mask)))

(defun free-colors (colormap pixels &optional (plane-mask 0))
  (declare (type colormap colormap)
	   (type sequence pixels) ;; Sequence of integers
	   (type pixel plane-mask))
  (with-buffer-request ((colormap-display colormap) *x-freecolors*)
    (colormap colormap)
    (card32 plane-mask)
    (sequence pixels)))

(defun store-color (colormap pixel spec &key (red-p t) (green-p t) (blue-p t))
  (declare (type colormap colormap)
	   (type pixel pixel)
	   (type (or stringable color) spec)
	   (type boolean red-p green-p blue-p))
  (let ((display (colormap-display colormap))
	(flags 0))
    (declare (type display display)
	     (type card8 flags))
    (when red-p (setq flags 1))
    (when green-p (incf flags 2))
    (when blue-p (incf flags 4))
    (with-display (display)
      (etypecase spec
	(color
	 (with-buffer-request (display *x-storecolors*)
	   (colormap colormap)
	   (card32 pixel)
	   (rgb-val (color-red spec)
		    (color-green spec)
		    (color-blue spec))
	   (card8 flags)
	   (pad8 nil)))
	 (stringable
	  (let* ((string (string spec))
		 (length (length string)))
	   (with-buffer-request (display *x-storenamedcolor*)
	     (colormap colormap)
	     (card32 pixel)
	     (card16 length)
	     (pad16 nil)
	     (string string))))
	 ))))

(defun store-colors (colormap specs &key (red-p t) (green-p t) (blue-p t))
  ;; If stringables are specified for colors, it is unspecified whether all
  ;; stringables are first resolved and then a single StoreColors protocol request is
  ;; issued, or whether multiple StoreColors protocol requests are issued.
  (declare (type colormap colormap)
	   (type sequence specs) ;; (repeat-seq (integer pixel) ((or stringable color) color)) specs)
	   (type boolean red-p green-p blue-p))
  (etypecase specs
    (list
     (do* ((spec specs (cddr spec))
	   (pixel (car spec) (car spec))
	   (color (cadr spec) (cadr spec)))
	  ((endp spec))
       (store-color colormap pixel color :red-p red-p :green-p green-p :blue-p blue-p)))
    (vector
     (do* ((i 0 (+ i 2))
	   (len (length specs))
	   (pixel (aref specs i) (aref specs i))
	   (color (aref specs (1+ i)) (aref specs (1+ i))))
	  ((>= i len))
       (store-color colormap pixel color :red-p red-p :green-p green-p :blue-p blue-p)))))

(defun query-colors (colormap pixels &key (result-type 'list))
  (declare (type colormap colormap)
	   (type sequence pixels) ;; sequence of integer
	   (type t result-type))   ;; a type specifier
  (declare-values (sequence color))
  (let ((display (colormap-display colormap))
	sequence)
    (with-display (display)
      (with-buffer-request (display *x-querycolors* :no-after)
	(colormap colormap)
	(sequence pixels))
      (wait-for-reply display nil)
      (reading-buffer-reply (display :sizes (8 16))
	(let* ((ncolors (card16-get 8)))
	  (setq sequence (make-sequence result-type ncolors))
	  (dotimes (i ncolors sequence)
	    (buffer-input display buffer-bbuf 0 8)
	    (setf (elt sequence i)
		  (make-color :red (rgb-val-get 0)
			      :green (rgb-val-get 2)
			      :blue (rgb-val-get 4)))))))
    (display-invoke-after-function display)
    sequence))

(defun lookup-color (colormap name)
  (declare (type colormap colormap)
	   (type stringable name))
  (declare-values screen-color true-color)
  (let* ((display (colormap-display colormap))
	 (string (string name))
	 (length (length string))
	 screen-color true-color)
    (with-display (display)
      (with-buffer-request (display *x-lookupcolor* :no-after)
	(colormap colormap)
	(card16 length)
	(pad16 nil)
	(string string))
      (with-buffer-reply (display 20 :sizes 16)
	(setq screen-color (make-color :red (rgb-val-get 14)
				       :green (rgb-val-get 16)
				       :blue (rgb-val-get 18))
	      true-color (make-color :red (rgb-val-get 8)
				     :green (rgb-val-get 10)
				     :blue (rgb-val-get 12)))))
    (display-invoke-after-function display)
    (values screen-color true-color)))

(defun create-cursor (&key
		      (source (required-arg source))
		      mask
		      (x (required-arg x))
		      (y (required-arg y))
		      (foreground (required-arg foreground))
		      (background (required-arg background)))
  (declare (type pixmap source) ;; required
	   (type (or null pixmap) mask)
	   (type card16 x y) ;; required
	   (type (or null color) foreground background)) ;; required
  (declare-values cursor)
  (let* ((display (pixmap-display source))
	 (cursor (make-cursor :display display))
	 (cid (allocate-resource-id display cursor 'cursor)))
    (setf (cursor-id cursor) cid)
    (with-buffer-request (display *x-createcursor*)
      (resource-id cid)
      (pixmap source)
      ((or null pixmap) mask)
      (rgb-val (color-red foreground)
	       (color-green foreground)
	       (color-blue foreground))
      (rgb-val (color-red background)
	       (color-green background)
	       (color-blue background))
      (card16 x y))
    cursor))

(defun create-glyph-cursor (&key
			    (source-font (required-arg source-font))
			    (source-char (required-arg source-char))
			    mask-font
			    mask-char
			    (foreground (required-arg foreground))
			    (background (required-arg background)))
  (declare (type font source-font) ;; Required
	   (type card16 source-char) ;; Required
	   (type (or null font) mask-font)
	   (type (or null card16) mask-char)
	   (type color foreground background)) ;; required
  (declare-values cursor)
  (let* ((display (font-display source-font))
	 (cursor (make-cursor :display display))
	 (cid (allocate-resource-id display cursor 'cursor))
	 (source-font-id (font-id source-font))
	 (mask-font-id (if mask-font (font-id mask-font) 0)))
    (setf (cursor-id cursor) cid)
    (unless mask-char (setq mask-char 0))
    (with-buffer-request (display *x-createglyphcursor*)
      (resource-id cid source-font-id mask-font-id)
      (card16 source-char)
      (card16 mask-char)
      (rgb-val (color-red foreground)
	       (color-green foreground)
	       (color-blue foreground))
      (rgb-val (color-red background)
	       (color-green background)
	       (color-blue background)))
    cursor))

(defun free-cursor (cursor)
  (declare (type cursor cursor))
  (let ((display (cursor-display cursor)))
    (with-buffer-request (display *x-freecursor*)
      (cursor cursor))
    (deallocate-resource-id display (cursor-id cursor) 'cursor)))

(defun recolor-cursor (cursor foreground background)
  (declare (type cursor cursor)
	   (type color foreground background))
  (with-buffer-request ((cursor-display cursor) *x-recolorcursor*)
    (cursor cursor)
    (rgb-val (color-red foreground)
	     (color-green foreground)
	     (color-blue foreground))
    (rgb-val (color-red background)
	     (color-green background)
	     (color-blue background))
    ))

(defun query-best-cursor (width height display)
  (declare (type card16 width height)
	   (type display display))
  (declare-values width height)
  (let (rwidth rheight)
    (with-display (display)
      (with-buffer-request (display *x-querybestsize* :no-after)
	(data 0)
	(window (screen-root (display-default-screen display)))
	(card16 width height))
      (with-buffer-reply (display 12 :sizes 16)
	(setq rwidth (card16-get 8)
	      rheight (card16-get 10))))
    (display-invoke-after-function display)
    (values rwidth rheight)))

(defun query-best-tile (width height drawable)
  (declare (type card16 width height)
	   (type drawable drawable))
  (declare-values width height)
  (let ((display (drawable-display drawable))
	rwidth rheight)
    (with-display (display)
      (with-buffer-request (display *x-querybestsize* :no-after)
	(data 1)
	(drawable drawable)
	(card16 width height))
      (with-buffer-reply (display 12 :sizes 16)
	(setq rwidth (card16-get 8)
	      rheight (card16-get 10))))
    (display-invoke-after-function display)
    (values rwidth rheight)))

(defun query-best-stipple (width height drawable)
  (declare (type card16 width height)
	   (type drawable drawable))
  (declare-values width height)
  (let ((display (drawable-display drawable))
	rwidth rheight)
    (with-display (display)
      (with-buffer-request (display *x-querybestsize* :no-after)
	(data 2)
	(drawable drawable)
	(card16 width height))
      (with-buffer-reply (display 12 :sizes 16)
	(setq rwidth (card16-get 8)
	      rheight (card16-get 10))))
    (display-invoke-after-function display)
    (values rwidth rheight)))

(defun query-extension (display name)
  (declare (type display display)
	   (type stringable name))
  (declare-values major-opcode first-event first-error)
  (let ((string (string name))
	major-opcode first-event first-error)
    (with-display (display)
      (with-buffer-request (display *x-queryextension* :no-after)
	(card16 (length string))
	(pad16 nil)
	(string string))
      (with-buffer-reply (display 12 :sizes 8)
	(when (boolean-get 8)    ;; If present
	  (setq major-opcode (card8-get 9)
		first-event (card8-get 10)
		first-error (card8-get 11)))))
    (display-invoke-after-function display)
    (values major-opcode first-event first-error)))

(defun list-extensions (display &key (result-type 'list))
  (declare (type display display)
	   (type t result-type)) ;; CL type
  (declare-values (sequence string))
  (let (result)
    (with-display (display)
      (with-buffer-request (display *x-listextensions* :no-after))
      (reading-buffer-reply (display :sizes 8)
	(let ((length (- (wait-for-reply display nil) *replysize*))
	      (nextensions (card8-get 1)))
	  (setq result (read-sequence-string display length nextensions result-type)))))
    (display-invoke-after-function display)
    result))

(defun change-keyboard-control (display &key key-click-percent
				bell-percent bell-pitch bell-duration
				led led-mode key auto-repeat-mode)
  (declare (type display display)
	   (type (or null (member :default) int16) key-click-percent
						   bell-percent bell-pitch bell-duration)
	   (type (or null card8) led key)
	   (type (or null (member :on :off)) led-mode)
	   (type (or null (member :on :off :default)) auto-repeat-mode))
  (when (eq key-click-percent :default) (setq key-click-percent -1))
  (when (eq bell-percent :default) (setq bell-percent -1))
  (when (eq bell-pitch :default) (setq bell-pitch -1))
  (when (eq bell-duration :default) (setq bell-duration -1))
  (with-buffer-request (display *x-changekeyboardcontrol* :sizes (32))
    (mask
      ((or null integer)
       key-click-percent bell-percent bell-pitch bell-duration)
      ((or null card32) led)
      ((or null (member :off :on)) led-mode)
      ((or null card32) key)
      ((or null (member :off :on :default)) auto-repeat-mode)
      )))

(defun keyboard-control (display)
  (declare (type display display))
  (declare-values key-click-percent bell-percent bell-pitch bell-duration
		  led-mask global-auto-repeat auto-repeats)
  (let (key-click-percent bell-percent bell-pitch bell-duration
	led-mask global-auto-repeat auto-repeats)
    (with-display (display)
      (with-buffer-request (display *x-getkeyboardcontrol* :no-after))
      (with-buffer-reply (display 32 :sizes (8 16 32))
	(setq global-auto-repeat (member8-get 1 :off :on))
	(setq led-mask (card32-get 8))
	(setq key-click-percent (card8-get 12))
	(setq bell-percent (card8-get 13))
	(setq bell-pitch (card16-get 14))
	(setq bell-duration (card16-get 16))
	(setq auto-repeats (bit-vector256-get 32))))
    (display-invoke-after-function display)
    (values key-click-percent bell-percent bell-pitch bell-duration
	    led-mask global-auto-repeat auto-repeats)))

;;  The base volume should
;; be considered to be the "desired" volume in the normal case; that is, a
;; typical application should call XBell with 0 as the percent.  Rather
;; than using a simple sum, the percent argument is instead used as the
;; percentage of the remaining range to alter the base volume by.  That is,
;; the actual volume is:
;;	 if percent>=0:    base - [(base * percent) / 100] + percent
;;	 if percent<0:     base + [(base * percent) / 100]

(defun bell (display &optional (percent-from-normal 0))
  ;; It is assumed that an eventual audio extension to X will provide more complete control.
  (declare (type display display)
	   (type int8 percent-from-normal))
  (with-buffer-request (display *x-bell*)
    (data (int8->card8 percent-from-normal))))

(defun pointer-mapping (display &key (result-type 'list))
  (declare (type display display)
	   (type t result-type)) ;; CL type
  (declare-values sequence) ;; Sequence of card
  (let (seq)
    (with-display (display)
      (with-buffer-request (display *x-getpointermapping* :no-after))
      (with-buffer-reply (display nil :sizes 8)
	(let ((nelts (card8-get 1)))
	  (setq seq (sequence-get :length nelts :result-type result-type :format card8)))))
    (display-invoke-after-function display)
    seq))

(defun set-pointer-mapping (display map)
  ;; Can signal device-busy.
  (declare (type display display)
	   (type sequence map)) ;; Sequence of card8
  (let (busy?)
    (with-display (display)
      (with-buffer-request (display *x-setpointermapping* :no-after)
	(data (length map))
	((sequence :format card8) map))
      (with-buffer-reply (display 2 :sizes 8)
	(setq busy? (boolean-get 1))))
    (display-invoke-after-function display)
    (when busy?
      (x-error 'device-busy :display display))
    map))

(defsetf pointer-mapping set-pointer-mapping)

(defun change-pointer-control (display &key acceleration threshold)
  ;; Acceleration is rationalized if necessary.
  (declare (type display display)
	   (type (or null (member :default) number) acceleration)
	   (type (or null (member :default) integer) threshold)
	   (inline rationalize16))
  (flet ((rationalize16 (number)
	   ;; Rationalize NUMBER into the ratio of two signed 16 bit numbers
	   (declare (type number number)
		    (inline rationalize16))
	   (declare-values numerator denominator)
	   (do* ((rational (rationalize number))
		 (numerator (numerator rational) (ash numerator -1))
		 (denominator (denominator rational) (ash denominator -1)))
		((or (= numerator 1)
		     (and (< (abs numerator) #x8000)
			  (< denominator #x8000)))
		 (values numerator (min denominator #x7fff))))))

    (let ((acceleration-p 1)
	  (threshold-p 1)
	  (numerator 0)
	  (denominator 1))
      (declare (type card8 acceleration-p threshold-p)
	       (type int16 numerator denominator))
      (cond ((eq acceleration :default) (setq numerator -1))
	    (acceleration (multiple-value-setq (numerator denominator)
			    (rationalize16 acceleration)))
	    (t (setq acceleration-p 0)))
      (cond ((eq threshold :default) (setq threshold -1))
	    ((null threshold) (setq threshold -1
				    threshold-p 0)))
      (with-buffer-request (display *x-changepointercontrol*)
	(int16 numerator denominator threshold)
	(card8 acceleration-p threshold-p)))))

(defun pointer-control (display)
  (declare (type display display))
  (declare-values acceleration threshold)
  (let (acceleration threshold)
    (with-display (display)
      (with-buffer-request (display *x-getpointercontrol* :no-after))
      (with-buffer-reply (display 16 :sizes 16)
	(setq acceleration (/ (card16-get 8) (card16-get 10)) ;; Should we float this?
	      threshold (card16-get 12))))
    (display-invoke-after-function display)
    (values acceleration threshold)))

(defun set-screen-saver (display timeout interval blanking exposures)
  ;; Timeout and interval are in seconds, will be rounded to minutes.
  (declare (type display display)
	   (type (or (member :default) int16) timeout interval)
	   (type (member :yes :no :default) blanking exposures))
  (when (eq timeout :default) (setq timeout -1))
  (when (eq interval :default) (setq interval -1))
  (with-buffer-request (display *x-setscreensaver*)
    (int16 timeout interval)
    ((member8 :no :yes :default) blanking exposures)))

(defun screen-saver (display)
  ;; Returns timeout and interval in seconds.
  (declare (type display display))
  (declare-values timeout interval blanking exposures)
  (let (timeout interval blanking exposures)
    (with-display (display)
      (with-buffer-request (display *x-getscreensaver* :no-after))
      (with-buffer-reply (display 14 :sizes (8 16))
	(setq timeout (card16-get 8)
	      interval (card16-get 10)
	      blanking (member8-get 12 :no :yes :default)
	      exposures (member8-get 13 :no :yes :default))))
    (display-invoke-after-function display)
    (values timeout interval blanking exposures)))

(defun activate-screen-saver (display)
  (declare (type display display))
  (with-buffer-request (display *x-forcescreensaver*)
    (data 1)))

(defun reset-screen-saver (display)
  (declare (type display display))
  (with-buffer-request (display *x-forcescreensaver*)
    (data 0)))

(defun add-access-host (display host &optional (family :internet))
  ;; A string must be acceptable as a host, but otherwise the possible types for
  ;; host are not constrained, and will likely be very system dependent.
  ;; This implementation uses a list whose car is the family keyword
  ;; (:internet :DECnet :Chaos) and cdr is a list of network address bytes.
  (declare (type display display)
	   (type (or stringable list) host)
	   (type (or null (member :internet :decnet :chaos) card8) family))
  (change-access-host display host family nil))

(defun remove-access-host (display host &optional (family :internet))
  ;; A string must be acceptable as a host, but otherwise the possible types for
  ;; host are not constrained, and will likely be very system dependent.
  ;; This implementation uses a list whose car is the family keyword
  ;; (:internet :DECnet :Chaos) and cdr is a list of network address bytes.
  (declare (type display display)
	   (type (or stringable list) host)
	   (type (or null (member :internet :decnet :chaos) card8) family))
  (change-access-host display host family t))

(defun change-access-host (display host family remove-p)
  (declare (type display display)
	   (type (or stringable list) host)
	   (type (or null (member :internet :decnet :chaos) card8) family))
  (unless (consp host)
    (setq host (host-address host family)))
  (let ((family (car host))
	(address (cdr host)))
    (with-buffer-request (display *x-changehosts*)
      ((data boolean) remove-p)
      (card8 (encode-type (or null (member :internet :decnet :chaos) card32) family))
      (card16 (length address))
      ((sequence :format card8) address))))

(defun access-hosts (display &optional (result-type 'list))
  ;; The type of host objects returned is not constrained, except that the hosts must
  ;; be acceptable to add-access-host and remove-access-host.
  ;; This implementation uses a list whose car is the family keyword
  ;; (:internet :DECnet :Chaos) and cdr is a list of network address bytes.
  (declare (type display display)
	   (type t result-type)) ;; CL type
  (declare-values (sequence host) enabled-p)
  (let (sequence enabled-p)
  (with-display (display)
    (with-buffer-request (display *x-listhosts* :no-after))
    (with-buffer-reply (display nil :sizes (8 16))
      (setq enabled-p (boolean-get 1))
      (let* ((nhosts (card16-get 8)))
	(setq sequence (make-sequence result-type nhosts))
	(dotimes (i nhosts)
	  (buffer-input display buffer-bbuf 0 4)
	  (let ((family (card8-get 0))
		(len (card16-get 2)))
	    (setf (elt sequence i)
		  (cons (if (< family 3)
			    (aref '#(:internet :decnet :chaos) family)
			  family)
			(sequence-get :length len :format card8 :result-type 'list))))))))
    (display-invoke-after-function display)
    (values sequence enabled-p)))

(defun access-control (display)
  (declare (type display display))
  (declare-values boolean) ;; True when access-control is ENABLED
  (let (result)
    (with-display (display)
      (with-buffer-request (display *x-listhosts* :no-after))
      (with-buffer-reply (display 2 :sizes 8)
	(setq result (boolean-get 1))))
    (display-invoke-after-function display)
    result))
  
(defun set-access-control (display enabled-p)
  (declare (type display display)
	   (type boolean enabled-p))
  (with-buffer-request (display *x-changeaccesscontrol*)
    ((data boolean) enabled-p))
  enabled-p)

(defsetf access-control set-access-control)

(defun close-down-mode (display)
  ;; setf'able
  ;; Cached locally in display object.
  (declare (type display display))
  (declare-values (member :destroy :retain-permanent :retain-temporary nil))
  (display-close-down-mode display))

(defun set-close-down-mode (display mode)
  ;; Cached locally in display object.
  (declare (type display display)
	   (type (member :destroy :retain-permanent :retain-temporary) mode))
  (setf (display-close-down-mode display) mode)
  (with-buffer-request (display *x-changeclosedownmode* :sizes (32))
    ((data (member :destroy :retain-permanent :retain-temporary)) mode))
  mode)

(defsetf close-down-mode set-close-down-mode)

(defun kill-client (display resource-id)
  (declare (type display display)
	   (type resource-id resource-id))
  (with-buffer-request (display *x-killclient*)
    (resource-id resource-id)))

(defun kill-temporary-clients (display)
  (declare (type display display))
  (with-buffer-request (display *x-killclient*)
    (resource-id 0)))

#+comment ;; This is a protocol request, but its not very interesting...
(defun no-operation (display)
  (declare (type display display))
  (with-buffer-request (display *x-nooperation*)))
