;;; -*- Mode: LISP; Syntax: Common-lisp; Package: XLIB; Base: 10; Lowercase: Yes -*-

;;; This file contains definitions for the DISPLAY object for Common-Lisp X windows version 11

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
;;; 12/10/87	LGO	Created

(in-package 'xlib :use '(lisp))

(export '(
	  event-listen
	  queue-event
	  process-event
	  event-case
	  event-cond
	  discard-current-event
	  request-error
	  value-error
	  window-error
	  pixmap-error
	  atom-error
	  cursor-error
	  font-error
	  match-error
	  drawable-error
	  access-error
	  alloc-error
	  colormap-error
	  gcontext-error
	  id-choice-error
	  name-error
	  length-error
	  implementation-error
	  request-error
	  resource-error
	  unknown-error
	  access-error
	  alloc-error
	  atom-error
	  colormap-error
	  cursor-error
	  drawable-error
	  font-error
	  gcontext-error
	  id-choice-error
	  illegal-request-error
	  length-error
	  match-error
	  name-error
	  pixmap-error
	  value-error
	  window-error
	  implementation-error
	  type-error
	  closed-display
	  lookup-error
	  connection-failure
	  reply-length-error
	  reply-timeout
	  server-disconnect
	  sequence-error
	  unexpected-reply
	  missing-parameter
	  invalid-font
	  device-busy
	  get-external-event-code
	  define-extension
	  extension-opcode
	  define-error
	  decode-core-error
	  declare-event
	  ))

;; Event Resource
(defvar *event-free-list* nil) ;; List of unused (processed) events

(eval-when (eval compile load)
(defconstant *max-events* 64) ;; Maximum number of events supported (the X11 alpha release only has 34)
(defvar *event-key-vector* (make-array *max-events* :initial-element nil)
  "Vector of event keys - See define-event")
)
(defvar *event-macro-vector* (make-array *max-events* :initial-element nil)
  "Vector of event handler functions - See declare-event")
(defvar *event-handler-vector* (make-array *max-events* :initial-element nil)
  "Vector of event handler functions - See declare-event")
(defvar *event-send-vector* (make-array *max-events* :initial-element nil)
  "Vector of event sending functions - See declare-event")

(defun allocate-event ()
  (or (atomic-pop *event-free-list*)
      (make-reply-buffer *replysize*)))

(defun deallocate-event (event)
  (atomic-push event *event-free-list*))

;; Extensions are handled as follows:
;; DEFINITION:	Use DEFINE-EXTENSION
;;
;; CODE:	Use EXTENSION-CODE to get the X11 opcode for an extension.
;;		This looks up the code on the display-extension-alist.
;;
;; EVENTS:	Use DECLARE-EVENT to define events. This calls ALLOCATE-EXTENSION-EVENT-CODE
;;		at LOAD time to define an internal event-code number
;;		(stored in the 'event-code property of the event-name)
;;		used to index the following vectors:
;;		*event-key-vector* 	Used for getting the event-key
;;		*event-macro-vector*	Used for getting the event-parameter getting macros
;;
;;		The GET-INTERNAL-EVENT-CODE function can be called at runtime to convert
;;		a server event-code into an internal event-code used to index the following
;;		vectors:
;;		*event-handler-vector*	Used for getting the event-handler function
;;		*event-send-vector*	Used for getting the event-sending function
;;
;;		The GET-EXTERNAL-EVENT-CODE function can be called at runtime to convert
;;		internal event-codes to external (server) codes.
;;
;; ERRORS:	Use DEFINE-ERROR to define new error decodings.
;;


;; Any event-code greater than 34 is for an extension
(defparameter *first-extension-event-code* 35)

(defvar *extensions* nil) ;; alist of (extension-name-symbol events errors)

(defmacro define-extension (name &key events errors)
  ;; Define extension NAME with EVENTS and ERRORS.
  ;; Note: The case of NAME is important.
  ;; To define the request, Use:
  ;;     (with-buffer-request (display (extension-opcode ,name)) ,@body)
  ;;     See the REQUESTS file for lots of examples.
  ;; To define event handlers, use declare-event.
  ;; To define error handlers, use declare-error and define-condition.
  (declare (type stringable name)
	   (type list events errors))
  (let ((name-symbol (kintern name)) ;; Intern name in the keyword package
	(event-list (mapcar #'canonicalize-event-name events)))
    `(eval-when (compile load eval)
       (setq *extensions* (cons (list ',name-symbol ',event-list ',errors)
				(delete ',name-symbol *extensions* :key #'car))))))

(eval-when (compile eval load)
(defun canonicalize-event-name (event)
  ;; Returns the event name keyword given an event name stringable
  (declare (type stringable event))
  (declare-values event-key)
  (kintern event))
) ;; end eval-when

(eval-when (compile eval load)
(defun allocate-extension-event-code (name)
  ;; Allocate an event-code for an extension
  ;; This is executed at COMPILE and LOAD time from DECLARE-EVENT.
  ;; The event-code is used at compile-time by macros to index the following vectors:
  ;; *event-key-vector* *event-macro-vector* *event-handler-vector* *event-send-vector*
  (let ((event-code (get name 'event-code)))
    (declare (type (or null card8) event-code))
    (unless event-code
      ;; First ensure the name is for a declared extension
      (unless (dolist (extension *extensions*)
		(when (member name (second extension))
		  (return t)))
	(x-type-error name 'event-key))
      (setq event-code (position nil *event-key-vector*
				 :start *first-extension-event-code*))
      (setf (aref *event-key-vector* event-code) name)
      (setf (get name 'event-code) event-code))
    event-code))
) ;; end eval-when

(defun get-internal-event-code (display code)
  ;; Given an X11 event-code, return the internal event-code.
  ;; The internal event-code is used for indexing into the following vectors:
  ;; *event-key-vector* *event-handler-vector* *event-send-vector*
  ;; Returns NIL when the event-code is for an extension that isn't handled.
  (declare (type display display)
	   (type card8 code))
  (declare-values (or nil card8))
  (setq code (logand #x7f code))
  (if (< code *first-extension-event-code*)
      code
    (let* ((code-offset (- code *first-extension-event-code*))
	   (event-extensions (display-event-extensions display))
	   (code (if (< code-offset (length event-extensions))
		     (aref event-extensions code-offset)
		   0)))
      (declare (type card8 code-offset code))
      (when (zerop code)
	(x-cerror "Ignore the event"
		  'unimplemented-event :event-code code :display display))
      code)))

(defun get-external-event-code (display event)
  ;; Given an X11 event name, return the event-code
  (declare (type display display)
	   (type event-key event))
  (declare-values card8)
  (let ((code (get-event-code event)))
    (declare (type (or null card8) code))
    (when (>= code *first-extension-event-code*)
      (setq code (+ *first-extension-event-code*
		    (or (position code (display-event-extensions display))
			(x-error 'undefined-event :display display :event-name event)))))
    code))

(defmacro extension-opcode (display name)
  ;; Returns the major opcode for extension NAME.
  ;; This is a macro to enable NAME to be interned for fast run-time
  ;; retrieval. 
  ;; Note: The case of NAME is important.
  (declare (type display display)
	   (type stringable name))
  (declare-values card8)
  (let ((name-symbol (kintern name))) ;; Intern name in the keyword package
    `(or (second (assoc ',name-symbol (display-extension-alist ,display)))
	 (x-error 'absent-extension :name ',name-symbol :display ,display))))

(defun initialize-extensions (display)
  ;; Initialize extensions for DISPLAY
  (let ((event-extensions (make-array 16 :element-type 'card8 :initial-element 0))
	(extension-alist nil))
    (declare (type vector event-extensions)
	     (type list extension-alist))
    (dolist (extension *extensions*)
      (let ((name (first extension))
	    (events (second extension)))
	(declare (type keyword name)
		 (type list events))
	(multiple-value-bind (major-opcode first-event first-error)
	    (query-extension display name)
	  (declare (type (or null card8) major-opcode first-event first-error))
	  (when (and major-opcode (plusp major-opcode))
	    (push (list name major-opcode first-event first-error)
		  extension-alist)
	    (when (plusp first-event) ;; When there are extension events
	      ;; Grow extension vector when needed
	      (let ((max-event (- (+ first-event (length events))
				  *first-extension-event-code*)))
		(declare (type card8 max-event))
		(when (>= max-event (length event-extensions))
		  (let ((new-extensions (make-array (+ max-event 16) :element-type 'card8
						    :initial-element 0)))
		    (declare (type vector new-extensions))
		    (replace new-extensions event-extensions)
		    (setq event-extensions new-extensions))))
	      (dolist (event events)
		(declare (type symbol event))
		(setf (aref event-extensions (- first-event *first-extension-event-code*))
		      (get-event-code event))
		(incf first-event)))))))
    (setf (display-event-extensions display) event-extensions)
    (setf (display-extension-alist display) extension-alist)))

;;
;; Reply handlers
;;

(defun wait-for-reply (display expected-size)
  ;; Wait for a reply to a request.
  ;; Expected-size is the length in BYTES,
  ;;   or NIL to only read the first 32 bytes,
  ;;   or T to read in the whole thing.
  ;; Returns with REPLY in the reply-buffer in DISPLAY
  ;; Handle error and event packets that are encountered
  ;; Internal function, MUST BE CALLED FROM WITHIN WITH-BUFFER
  (declare (type display display)
	   (type (or integer (member nil t)) expected-size))
  (declare-values length-in-bytes)
  (setf (display-waiting-reply-p display) t) ;; indicate awaiting reply
  (buffer-force-output display)
  (do ((sequence-error nil)
       (req-sequence (ldb (byte 16 0) (buffer-request-number display))))
      (nil) ;; forever
    (with-input-lock (display)
      (reading-buffer-reply (display)
	;; If waiting-reply-p is :in-buffer then the reply was read in while
	;; waiting for the input lock (see wait-for-event)
	(unless (eq (display-waiting-reply-p display) :in-buffer)
	  ;; Loop for reply-timeout condition return
	  (do ((timeout *reply-timeout* (floor timeout 0.666s0))
	       (eofp))
	      (()) ;; forever
	    (setq eofp (buffer-input display buffer-bbuf 0 *replysize* timeout))
	    (if eofp
		(if (eq eofp :timeout)
		    (if sequence-error
			(progn
			  (x-cerror "Proceed, altering display sequence number"
				    'sequence-error
				    :display display
				    :req-sequence req-sequence
				    :msg-sequence sequence-error)
			  (setf (buffer-request-number display) sequence-error
				req-sequence sequence-error)
			  (return nil))
		      (x-cerror "Retry with longer timeout"
				'reply-timeout
				:display display
				:timeout timeout))
		  (progn
		    (setf (display-dead display) t)
		    (x-error 'server-disconnect :display display)))
	      (return nil))))
	(case (read-card8 0) ;; Type
	  (0					; Error
	   (report-error display (buffer-reply-buffer display)))
	  (1					; Normal reply
	    (let ((msg-sequence (read-card16 2))      ;; Message sequence number
		  (length (+ (* (read-card32 4) 4) *replysize*))) ;; Length in bytes
	      (if (= msg-sequence req-sequence)	; Check for reply out of sequence
		  (progn 
		    (when expected-size
		      (if (and (not (eq expected-size t))
			       (> expected-size length))
			  (x-error 'reply-length-error
				   :display display
				   :expected-length expected-size
				   :reply-length length)
			(progn
			  (if (<= length (reply-size (buffer-reply-buffer display)))
			      (buffer-input display buffer-bbuf *replysize* length)
			    (progn
			      ;; reply buffer too small (should never happen!)
			      (cerror "Grow the reply buffer"
				      "Reply buffer too small, increase to ~d bytes"
				      length)
			      ;; Grow it
			      (let* ((new-buffer (make-reply-buffer (+ length 64)))
				     (new-bbuf (reply-ibuf8 new-buffer)))
				(buffer-replace new-bbuf buffer-bbuf 0 *replysize*)
				(buffer-input display new-bbuf *replysize* length)
				(setf (buffer-reply-buffer display) new-buffer)))))))
		    (setf (display-waiting-reply-p display) nil)
		    (return length))
		;; Reply out of sequence
		(if (> msg-sequence req-sequence)
		    (progn
		      (x-cerror "Proceed, altering display sequence number"
				'sequence-error
				:display display
				:req-sequence req-sequence
				:msg-sequence msg-sequence)
		      (setf (buffer-request-number display) msg-sequence))
		  ;; We usually get here because of an abort while waiting for a reply
		  ;; Loop back around and read another reply (hopefully the one we want)
		  (progn
		    ;; **** debug *****
		    (format t "~%wait-for-reply recovering from sequence-error. ~
			         Expected ~d Got ~d" req-sequence msg-sequence)
		    ;; Flush input
		    (buffer-input display buffer-bbuf *replysize* length *reply-timeout*)
		    (setq sequence-error msg-sequence))))))
	  (otherwise				; Event
	    ;; Push the event in the input buffer on the display's event queue
	    (let ((event (allocate-event)))
	      ;; Copy into event from reply buffer
	      (buffer-replace (reply-ibuf8 event)
			      (reply-ibuf8 (buffer-reply-buffer display))
			      0
			      *replysize*)
	      (setf (event-code event)
		    (get-internal-event-code display (read-card8 0)))
	      (enqueue-event event display))))))))

;; Its necessary to have a lock on input operations, to prevent two
;; process from reading from the server at the same time.  Its also
;; necessary to allow one process to be hung inside wait-for-event
;; waiting for an event while another process makes requests and waits
;; for replies.  With this situation, its possible that the process
;; that reads the reply is different from the process waiting for the
;; reply.  The process waiting for the reply must not hang because
;; there's another high priority process that's always waiting for
;; events, and has the input lock.  Because of this, wait-for-event
;; will wait for, and immediately give-up the display lock, when the
;; waiting-reply-p flag is set in the display.  This is sufficient to
;; break the deadlock.

(defun wait-for-event (display timeout force-output-p)
  ;; Wait for an event.
  ;; Handle error and event packets that are encountered
  ;; Returns :TIMEOUT on timeout, else NIL.
  (declare (type display display)
	   (type (or null number) timeout))
  (declare-values eof-or-timeout)
  (do ((do-force-output nil))
      (nil) ;; forever
    (block retry
      (when (display-waiting-reply-p display) ;; See comments above
	(with-display (display)
	  ;; Flag whould be already NIL.  Reset it here incase wait-for-reply aborted.
	  (setf (display-waiting-reply-p display) nil)))
      ;; When first pass determined no input available
      (when do-force-output
	(setq do-force-output nil)		; To avoid deadlocks, avoid display
	(display-force-output display)		; operations inside with-input-lock.
	(when (and timeout (zerop timeout))
	  (return :timeout)))			; Optimize zero timeout case.
      (with-input-lock (display)
	;; Return if events read while waiting for locks
	(when (display-new-events display)
	  (return nil))
	;; Give up input-lock if waiting event
	(when (display-waiting-reply-p display)
	  (return-from retry))
	(let ((event (allocate-event))
	      (eofp :timeout))
	  (reading-event (event)
	    ;; Check for input pending
	    (when force-output-p
	      (setq force-output-p nil)
	      ;; Read with timeout = 0, which doesn't hang when no input available
	      (when (setq eofp (buffer-input display buffer-bbuf 0 *replysize* 0))
		(deallocate-event event)
		(setq do-force-output t)	; Loop back and force output
		(return-from retry)))		; when no available input
	    ;; Hang waiting for an event
	    (when eofp
	      (when (eq eofp :timeout)
		(setq eofp (buffer-input display buffer-bbuf 0 *replysize* timeout)))
	      (when eofp
		(deallocate-event event)
		(if (eq eofp :timeout)
		    (return eofp)
		    (progn
		      (x-error 'server-disconnect :display display)
		      (setf (display-dead display) t)))))
	    ;; Check for replies and errors
	    (let ((type (read-card8 0)))
	      (case type
		(0				; Error
		  (if (display-lock display)
		      (buffer-replace (buffer-obuf8 display)
				      buffer-bbuf
				      0
				      *replysize*)
		      (report-error display event)))
		(1				; Normal reply
		  (if (display-waiting-reply-p display)
		      (progn
			(buffer-replace (reply-ibuf8 (display-reply-buffer display))
					buffer-bbuf
					0
					*replysize*)
			(setf (display-waiting-reply-p display) :in-buffer))
		      (x-cerror "Ignore"
				'unexpected-reply
				:display display
				:req-sequence (ldb (byte 16 0) (buffer-request-number display))
				:msg-sequence (read-card16 2)
				:length (+ (* (read-card32 4) 4) *replysize*))))
		(otherwise			; Event
		  (setf (event-code event)
			(get-internal-event-code display (read-card8 0)))
		  (enqueue-event event display)
		  (return nil))))))))))

;; The cons before the current event.
;; NIL outside EVENT-LOOP [used by event-case, event-cond, process-event]
(defvar *recursive-event-queue* nil)

(defun event-listen (display &optional (timeout 0))
  (declare (type display display)
	   (type (or null number) timeout))
  ;; Returns the number of events queued locally, if any, else nil.  Hangs waiting
  ;; for events, forever if timeout is nil, else for the specified number of seconds.
  (let ((queue (or *recursive-event-queue*
		   (display-event-queue display))))
    (if (cdr queue)
	(length (cdr queue))
      (wrap-event-listen
	(wait-for-event display timeout nil)
	(and (cdr queue) (length (cdr queue)))))))

(defun queue-event (display event-key &rest args &key append-p send-event-p &allow-other-keys)
  ;; The event is put at the head of the queue if append-p is nil, else the tail.
  ;; Additional arguments depend on event-key, and are as specified above with
  ;; declare-event, except that both resource-ids and resource objects are accepted
  ;; in the event components.
  (declare (type display display)
	   (type event-key event-key)
	   (type boolean append-p send-event-p))
  (unless (get event-key 'event-code)
    (x-type-error event-key 'event-key))
  (let* ((event (allocate-event))
	 (buffer (reply-ibuf8 event))
	 (event-code (get event-key 'event-code)))
    (unless event-code (x-type-error event-key 'event-key))
    (setf (event-code event) event-code)
    (with-display (display)
      (apply (aref *event-send-vector* event-code) display args)
      (buffer-replace buffer
		      (display-obuf8 display)
		      0
		      *replysize*
		      (index+ 12 (buffer-boffset display)))
      (setf (aref buffer 0) (if send-event-p (logior event-code #x80) event-code)
	    (aref buffer 2) 0
	    (aref buffer 3) 0))
    (with-event-queue (display)
      (if append-p
	  (enqueue-event event display)
	(with-event-queue-internal (display)
	  (let ((queue (display-event-queue display)))
	    (setf (cdr queue) (cons event (cdr queue)))))))))

(defun enqueue-event (new-event display)
  ;; Place EVENT at the end of the event queue for DISPLAY
  (let ((event (list new-event)))
    (declare (type list event))
    (let* ((event-code (event-code new-event))
	   (event-key (and (< event-code (length *event-key-vector*))
			   (aref *event-key-vector* event-code))))
      (if (null event-key)
	  (cerror "Ignore this event" "No handler for ~s event" event-key)
	(with-event-queue-internal (display)
	  (let ((new (display-new-events display)))
	    (declare (type list new))
	    (if new
		(setf (display-new-events display) (nconc new event))
	      (progn
		(setf (display-new-events display) event)
		(let ((old (display-event-queue display)))
		  (setf (display-event-queue display) (nconc old event)))))))))))


(defmacro define-event (name code)
  `(eval-when (eval compile load)
     (setf (aref *event-key-vector* ,code) ',name)
     (setf (get ',name 'event-code) ,code)))

;; Event names.  Used in "type" field in XEvent structures.  Not to be
;; confused with event masks above.  They start from 2 because 0 and 1
;; are reserved in the protocol for errors and replies. */

(define-event :key-press 2)
(define-event :key-release 3)
(define-event :button-press 4)
(define-event :button-release 5)
(define-event :motion-notify 6)
(define-event :enter-notify 7)
(define-event :leave-notify 8)
(define-event :focus-in 9)
(define-event :focus-out 10)
(define-event :keymap-notify 11)
(define-event :exposure 12)
(define-event :graphics-exposure 13)
(define-event :no-exposure 14)
(define-event :visibility-notify 15)
(define-event :create-notify 16)
(define-event :destroy-notify 17)
(define-event :unmap-notify 18)
(define-event :map-notify 19)
(define-event :map-request 20)
(define-event :reparent-notify 21)
(define-event :configure-notify 22)
(define-event :configure-request 23)
(define-event :gravity-notify 24)
(define-event :resize-request 25)
(define-event :circulate-notify 26)
(define-event :circulate-request 27)
(define-event :property-notify 28)
(define-event :selection-clear 29)
(define-event :selection-request 30)
(define-event :selection-notify 31)
(define-event :colormap-notify 32)
(define-event :client-message 33)
(define-event :mapping-notify 34)


(defmacro declare-event (event-codes &body declares)
  ;; Used to indicate the keyword arguments for handler functions in
  ;; process-event and event-case.
  ;; Generates the functions used in SEND-EVENT.
  ;; A compiler warning is printed when all of EVENT-CODES are not
  ;; defined by a preceding DEFINE-EXTENSION.
  ;; The body is a list of declarations, each of which has the form:
  ;; (type . items)  Where type is a data-type, and items is a list of
  ;; symbol names.  The item order corresponds to the order of fields
  ;; in the event sent by the server.  An item may be a list of items.
  ;; In this case, each item is aliased to the same event field.
  ;; This is used to give all events an EVENT-WINDOW item.
  ;; See the INPUT file for lots of examples.
  (declare (type (or keyword list) event-codes)
	   (type (alist (field-type symbol) (field-names list))
                 declares))
  (when (atom event-codes) (setq event-codes (list event-codes)))
  (setq event-codes (mapcar #'canonicalize-event-name event-codes))
  (let* (get-code get-index get-sizes
	 put-code put-index put-sizes keywords
	 (name (first event-codes))
	 (get-macro (xintern name '-event-get-macro))
	 (get-function (xintern name '-event-get))
	 (put-function (xintern name '-event-put))
	 (*buffer* #-kcl (gensym)
		   ;; XXX
		   #+kcl '*kcl-internal-buffer-symbol-needed-because-of-the-compiler-bug*
		   ))
    (multiple-value-setq (get-code get-index get-sizes)
      (get-put-items 2 declares nil
		     #'(lambda (type index item args)
			 (flet ((event-get (type index item args)
					   (unless (member type '(pad8 pad16))
					     `(,(kintern item)
					       (,(getify type) ,index ,@args)))))
			   (if (atom item)
			       (event-get type index item args)
			     (mapcan #'(lambda (item)
					 (event-get type index item args))
				     item))))))
    (multiple-value-setq (put-code put-index put-sizes)
      (get-put-items 2 declares t
		     #'(lambda (type index item args)
			 (unless (member type '(pad8 pad16))
			   (if (atom item)
			       (progn
				 (push item keywords)
				 `((,(putify type) ,index ,item ,@args)))
			     (let ((names (mapcar #'(lambda (name) (kintern name))
						  item)))
			       (setq keywords (append item keywords))
			       `((,(putify type) ,index
				  (check-consistency ',names ,@item) ,@args))))))))
    get-index put-index				; not used
    `(within-definition (,name declare-event)
       (defun ,get-macro (display event-key variable)
	 ;; Note: we take pains to macroexpand the get-code here to enable application
	 ;; code to be compiled without having the CLX macros file loaded.
	 (subst display ',*buffer*
		(getf `(:display (the display ,display)
			:event-key (the keyword ,event-key)
			:event-code (the card8 (logand #x7f (read-card8 0)))
			:send-event-p (the boolean (logbitp 7 (read-card8 0)))
			,@',(mapcar #'macroexpand get-code))
		      variable)))

       (defun ,get-function (display event handler)
	 (compiler-let ((*buffer* 'display))
	   (reading-event (event :sizes (8 16 ,@get-sizes))
	     (funcall handler
		      :display display
		      :event-key (aref *event-key-vector* (event-code event))
		      :event-code (logand #x7f (card8-get 0))
		      :send-event-p (logbitp 7 (card8-get 0))
		      ,@get-code))))

       (defun ,put-function (display &key ,@(setq keywords (nreverse keywords)) &allow-other-keys)
	 ,(when (member 'sequence keywords)
	    `(unless sequence (setq sequence (display-request-number display))))
	 (writing-buffer-send (display :sizes ,put-sizes
				       :index (index+ (buffer-boffset display) 12))
	   ,@put-code))
       
       ,@(mapcar #'(lambda (name)
		     (allocate-extension-event-code name)
		     `(let ((event-code (or (get ',name 'event-code)
					    (allocate-extension-event-code ',name))))
			(setf (aref *event-macro-vector* event-code)
			      (function ,get-macro))
			(setf (aref *event-handler-vector* event-code)
			      (function ,get-function))
			(setf (aref *event-send-vector* event-code)
			      (function ,put-function))))
		 event-codes)
       ',name)))

(defun check-consistency (names &rest args)
  ;; Ensure all args are nil or have the same value.
  ;; Returns the consistent non-nil value.
  (let ((value (car args)))
    (dolist (arg (cdr args))
      (if value
	  (when (and arg (not (eq arg value)))
	    (x-error 'inconsistent-parameters
		     :parameters (mapcan #'list names args)))
	(setq value arg)))
    value))

(declare-event (:key-press :key-release :button-press :button-release)
  ;; for key-press and key-release, code is the keycode
  ;; for button-press and button-release, code is the button number
  (data code)
  (card16 sequence)
  (card32 time)
  (window root (window event-window))
  ((or null window) child)
  (int16 root-x root-y x y)
  (card16 state)
  (boolean same-screen-p)
  )

(declare-event :motion-notify
  ((data boolean) hint-p)
  (card16 sequence)
  (card32 time)
  (window root (window event-window))
  ((or null window) child)
  (int16 root-x root-y x y)
  (card16 state)
  (boolean same-screen-p))

(declare-event (:enter-notify :leave-notify)
  ((data (member8 :ancestor :virtual :inferior :nonlinear :nonlinear-virtual)) kind)
  (card16 sequence)
  (card32 time)
  (window root (window event-window))
  ((or null window) child)
  (int16 root-x root-y x y)
  (card16 state)
  ((member8 :normal :grab :ungrab) mode)
  ((bit 0) focus-p)
  ((bit 1) same-screen-p))

(declare-event (:focus-in :focus-out)
  ((data (member8 :ancestor :virtual :inferior :nonlinear :nonlinear-virtual
		  :pointer :pointer-root :none))
   kind)
  (card16 sequence)
  (window (window event-window))
  ((member8 :normal :while-grabbed :grab :ungrab) mode))

(declare-event :keymap-notify
  ((bit-vector256 0) keymap))

(declare-event :exposure
  (card16 sequence)
  (window (window event-window))
  (card16 x y width height count))

(declare-event :graphics-exposure
  (card16 sequence)
  (drawable (drawable event-window))
  (card16 x y width height)
  (card16 minor)  ;; Minor opcode
  (card16 count)
  (card8 major))  ;; Major opcode

(declare-event :no-exposure
  (card16 sequence)
  (drawable (drawable event-window))
  (card16 minor)
  (card8  major))

(declare-event :visibility-notify
  (card16 sequence)
  (window (window event-window))
  ((member8 :unobscured :partially-obscured :fully-obscured) state))

(declare-event :create-notify
  (card16 sequence)
  (window (parent event-window) window)
  (int16 x y)
  (card16 width height border-width)
  (boolean override-redirect-p))

(declare-event :destroy-notify
  (card16 sequence)
  (window event-window window))

(declare-event :unmap-notify
  (card16 sequence)
  (window event-window window)
  (boolean configure-p))

(declare-event :map-notify
  (card16 sequence)
  (window event-window window)
  (boolean override-redirect-p))

(declare-event :map-request
  (card16 sequence)
  (window (parent event-window) window))

(declare-event :reparent-notify
  (card16 sequence)
  (window event-window window parent)
  (int16 x y)
  (boolean override-redirect-p))

(declare-event :configure-notify
  (card16 sequence)
  (window event-window window)
  ((or null window) above-sibling)
  (int16 x y)
  (card16 width height border-width)
  (boolean override-redirect-p))

(declare-event :configure-request
  ((data (member :above :below :top-if :bottom-if :opposite)) stack-mode)
  (card16 sequence)
  (window (parent event-window) window)
  ((or null window) above-sibling)
  (int16 x y)
  (card16 width height border-width value-mask))

(declare-event :gravity-notify
  (card16 sequence)
  (window event-window window)
  (int16 x y))

(declare-event :resize-request
  (card16 sequence)
  (window (window event-window))
  (card16 width height))

(declare-event :circulate-notify
  (card16 sequence)
  (window event-window window parent)
  ((member16 :top :bottom) place))

(declare-event :circulate-request
  (card16 sequence)
  (window (parent event-window) window)
  (pad16 1 2)
  ((member16 :top :bottom) place))

(declare-event :property-notify
  (card16 sequence)
  (window (window event-window))
  (keyword atom) ;; keyword
  (card32 time)
  ((member16 :new-value :deleted) state))

(declare-event :selection-clear
  (card16 sequence)
  (card32 time)
  (window (window event-window)) 
  (keyword selection) ;; keyword
  )

(declare-event :selection-request
  (card16 sequence)
  (card32 time)
  (window (window event-window) requestor)
  (keyword selection target)
  ((or null keyword) property)
  )

(declare-event :selection-notify
  (card16 sequence)
  (card32 time)
  (window (window event-window))
  (keyword selection target)
  ((or null keyword) property)
  )

(declare-event :colormap-notify
  (card16 sequence)
  (window (window event-window))
  ((or null colormap) colormap)
  (boolean new-p installed-p))

(declare-event :client-message
  (data format)
  (card16 sequence)
  (window (window event-window))
  (keyword type)
  ((client-message-sequence format) data))

(declare-event :mapping-notify
  (card16 sequence)
  ((member8 :modifier :keyboard :pointer) request)
  (card8 start) ;; first key-code
  (card8 count))


;;
;; EVENT-LOOP
;;
;;; (display-event-queue display) contains a cons whose CDR is the first event.
;;; CLX always passes around a cons BEFORE the next event to make it easy to
;;; remove events from the queue.  This is much easier than keeping track of
;;; the previous cons, and faster than using DELETE.

(defmacro event-loop ((display event timeout force-output-p discard-p) &body body)
  ;; Bind EVENT to the events for DISPLAY.
  ;; This is the "GUTS" of process-event and event-case.
  (let ((events (gensym)))
    `(with-event-queue (,display)
       (let ((,events (or *recursive-event-queue*	; The cons before the current event
			  (display-event-queue ,display))))
	 (declare (type cons ,events))
	 (loop
						; Read events when queue empty
	   (unless (cdr ,events)
	     (when (wait-for-event ,display ,timeout ,force-output-p)
	       (return nil)))			; return when timeout exceeded
	   
	   (new-event-update ,display ,events)	; Keep the new-event list updated
	   
	   (let ((,event (cadr ,events))	; Bind *recursive-event-queue* to
		 (*recursive-event-queue* (cdr ,events)))   ;; the cons before the next event.
	     (progn ,@body)			; Execute the body
	     (when (eq ,event (cadr ,events))	; Pop event if not discarded
	       (if ,discard-p
		   (discard-current-event ,display)
		 (pop ,events)))
	     ))))))

(defun discard-current-event (display)
  ;; Discard the current event for DISPLAY.
  ;; Returns NIL when the event queue is empty, else T.
  ;; To ensure events aren't ignored, application code should only call
  ;; this when throwing out of event-case or process-next-event, or from
  ;; inside even-case, event-cond or process-event when :peek-p is T and
  ;; :discard-p is NIL.
  (declare (type display display))
  (declare-values boolean)
  (if *recursive-event-queue*
      (do* ((previous (display-event-queue display) queue)
	    (queue (cdr previous) (cdr queue))
	    (current-event (car *recursive-event-queue*)))
	   ((eq current-event (car queue))
	    (when queue ;; return NIL when queue is empty
	      (when (eq queue (display-new-events display))
		;; Deleting a new (unseen) event.
		;;   Should this signal an error?
		;;   Should we return NIL here?
		;; Assume caller knows what he's doing and
		;; update the new-event pointer.
		(new-event-update display previous))

	      ;; Remove event from the queue
	      (setf (cdr previous) (cdr queue)) 
	      (deallocate-event current-event)
	      (setq *recursive-event-queue* previous)
	      t))
	(declare (type cons previous)
		 (type list queue)))
    
    ;; Called outside event-loop - call ourselves with the lock grabbed.
    (with-event-queue (display)
      (let ((*recursive-event-queue* (cdr (display-event-queue display))))
	(when *recursive-event-queue*
	  (discard-current-event display))))))

(defun new-event-update (display event-list)
  ;; Internal function called in EVENT-LOOP to keep the new-event list updated.
  (declare (inline new-event-update))
  ;; This isn't proclaimed in-line because the with-event-queue-internal
  ;; expansion is long.  Declare inline here so compiler saves
  ;; definition and this can be proclaimed inline later by users.
  (with-event-queue-internal (display)
    ;; When event is new, Pop the new-event list
    (let ((new (display-new-events display)))
      (when (eq (cdr event-list) new)
	(setf (display-new-events display) (cdr new))))))

;;
;; PROCESS-EVENT
;;
(defun process-event (display &key handler timeout peek-p discard-p (force-output-p t))
  ;; If force-output-p is true, first invokes display-force-output.  Invokes handler
  ;; on each queued event until handler returns non-nil, and that returned object is
  ;; then returned by process-event.  If peek-p is true, then the event is not
  ;; removed from the queue.  If discard-p is true, then events for which handler
  ;; returns nil are removed from the queue, otherwise they are left in place.  Hangs
  ;; until non-nil is generated for some event, or for the specified timeout (in
  ;; seconds, if given); however, it is acceptable for an implementation to wait only
  ;; once on network data, and therefore timeout prematurely.  Returns nil on
  ;; timeout.  If handler is a sequence, it is expected to contain handler functions
  ;; specific to each event class; the event code is used to index the sequence,
  ;; fetching the appropriate handler.  Handler is called with raw resource-ids, not
  ;; with resource objects.  The arguments to the handler are described using declare-event.
  ;;
  ;; T for peek-p means the event (for which the handler returns non-nil) is not removed
  ;; from the queue (it is left in place), NIL means the event is removed.
  
  (declare (type display display)
	   (type t handler) ;; (or (sequence (function (display &rest key-vals) t))
	   ;;	   (function (display event-key &rest key-vals) t))
	   (type (or null number) timeout)
	   (type boolean peek-p))
  (event-loop (display event timeout force-output-p discard-p)
    (let* ((event-code (event-code event)) ;; Event decoder defined by DECLARE-EVENT
	   (event-decoder (and (< event-code (length *event-handler-vector*))
			       (aref *event-handler-vector* event-code))))
      (if event-decoder
	  (let ((event-handler (if (functionp handler)
				   handler
				   (and (type? handler 'sequence)
					(< event-code (length handler))
					(elt handler event-code)))))
	    (if event-handler
		(let ((result (funcall event-decoder display event event-handler)))
		  (when result
		    (unless peek-p
		      (discard-current-event display))
		    (return result)))
	      (cerror "Ignore this event"
		      "No handler for ~s event"
		      (aref *event-key-vector* event-code))))
	(cerror "Ignore this event"
		"Server Error: event with unknown event code ~d received."
		event-code)))))

;;
;; EVENT-CASE
;; 

(defmacro event-case ((&rest args) &body clauses)
  ;; If force-output-p is true, first invokes display-force-output.  Executes the
  ;; matching clause for each queued event until a clause returns non-nil, and that
  ;; returned object is then returned by event-case.  If peek-p is true, then the
  ;; event is not removed from the queue.  If discard-p is true, then events for
  ;; which the clause returns nil are removed from the queue, otherwise they are left
  ;; in place.  Hangs until non-nil is generated for some event, or for the specified
  ;; timeout (in seconds, if given); however, it is acceptable for an implementation
  ;; to wait only once on network data, and therefore timeout prematurely.  Returns
  ;; nil on timeout.  In each clause, event-or-events is an event-key or a list of
  ;; event-keys (but they need not be typed as keywords) or the symbol t or otherwise
  ;; (but only in the last clause).  The keys are not evaluated, and it is an error
  ;; for the same key to appear in more than one clause.  Args is the list of event
  ;; components of interest; corresponding values (if any) are bound to variables
  ;; with these names (i.e., the args are variable names, not keywords, the keywords
  ;; are derived from the variable names).  An arg can also be a (keyword var) form,
  ;; as for keyword args in a lambda lists.  If no t/otherwise clause appears, it is
  ;; equivalent to having one that returns nil.
  (declare-arglist (display &key timeout peek-p discard-p force-output-p)
		   (event-or-events ((&rest args) |...|) &body body) |...|)
  ;; Event-case is just event-cond with the whole body in the test-form
  `(event-cond ,args
	       ,@(mapcar
		   #'(lambda (clause)
		       `(,(car clause) ,(cadr clause) (progn ,@(cddr clause))))
		   clauses)))

;;
;; EVENT-COND
;; 

(defmacro event-cond ((display &key timeout peek-p discard-p (force-output-p t))
		      &body clauses)
  ;; The clauses of event-cond are of the form:
  ;; (event-or-events binding-list test-form . body-forms)
  ;;
  ;; EVENT-OR-EVENTS	event-key or a list of event-keys (but they
  ;;			need not be typed as keywords) or the symbol t
  ;;			or otherwise (but only in the last clause).  If
  ;;			no t/otherwise clause appears, it is equivalent
  ;;			to having one that returns nil.  The keys are
  ;;			not evaluated, and it is an error for the same
  ;;			key to appear in more than one clause.
  ;;
  ;; BINDING-LIST	The list of event components of interest.
  ;;			corresponding values (if any) are bound to
  ;;			variables with these names (i.e., the binding-list
  ;;			has variable names, not keywords, the keywords are
  ;;			derived from the variable names).  An arg can also
  ;;			be a (keyword var) form, as for keyword args in a
  ;;			lambda list.
  ;;
  ;; The matching TEST-FORM for each queued event is executed until a
  ;; clause's test-form returns non-nil.  Then the BODY-FORMS are
  ;; evaluated, returning the (possibly multiple) values of the last
  ;; form from event-cond.  If there are no body-forms then, if the
  ;; test-form is non-nil, the value of the test-form is returned as a
  ;; single value.
  ;;
  ;; Options:
  ;; FORCE-OUTPUT-P	When true, first invoke display-force-output if no
  ;;		  	input is pending.
  ;;
  ;; PEEK-P		When true, then the event is not removed from the queue.
  ;;
  ;; DISCARD-P		When true, then events for which the clause returns nil
  ;; 			are removed from the queue, otherwise they are left in place.
  ;;
  ;; TIMEOUT		If NIL, hang until non-nil is generated for some event's
  ;;			test-form. Otherwise return NIL after TIMEOUT seconds have
  ;;			elapsed.
  ;;
  (declare-arglist (display &key timeout peek-p discard-p force-output-p)
		   (event-or-events (&rest args) test-form &body body) |...|)
  (let ((event (gensym))
	(disp (gensym)))
    `(let ((,disp ,display)
	   ,@(when (consp peek-p)
	       ;; If Peek-p is a function, only evaluate it once.
	       (let ((temp (gensym)))
		 (prog1
		   `((,temp ,peek-p))
		   (setq peek-p temp)))))
       (event-loop (,disp ,event ,timeout ,force-output-p ,discard-p)
	 (event-dispatch (,disp ,event ,peek-p) ,@clauses)))))

(defun get-event-code (event)
  ;; Returns the event code given an event-key
  (declare (type event-key event))
  (declare-values card8)
  (or (get event 'event-code)
      (x-type-error event 'event-key)))

(defun universal-event-get-macro (display event-key variable)
  (getf
    `(:display (the display ,display) :event-key (the keyword ,event-key) :event-code
	       (the card8 (logand 127 (read-card8 0))) :send-event-p
	       (the boolean (logbitp 7 (read-card8 0))))
    variable))

(defmacro event-dispatch ((display event peek-p) &body clauses)
  ;; Helper macro for event-case
  ;; CLAUSES are of the form:
  ;; (event-or-events binding-list test-form . body-forms)
  (let ((event-key (gensym))
	(all-events (make-array *max-events* :element-type 'bit :initial-element 0)))
    `(reading-event (,event)
       (let ((,event-key (aref *event-key-vector* (event-code ,event))))
	 (case ,event-key
	   ,@(mapcar
	       #'(lambda (clause)		; Translate event-cond clause to case clause
		   (let* ((events (first clause))
			  (arglist (second clause))
			  (test-form (third clause))
			  (body-forms (cdddr clause)))
		     (flet ((event-clause (display peek-p first-form rest-of-forms)
			      (if rest-of-forms
				  `(when ,first-form
				     (unless ,peek-p (discard-current-event ,display))
				     (return (progn ,@rest-of-forms)))
				;; No body forms, return the result of the test form
				(let ((result (gensym)))
				  `(let ((,result ,first-form))
				     (when ,result
				       (unless ,peek-p (discard-current-event ,display))
				       (return ,result)))))))

		       (if (member events '(otherwise t))
			   ;; code for OTHERWISE clause.
			   ;; Find all events NOT used by other clauses
			   (let ((keys (do ((i 0 (1+ i))
					    (key nil)
					    (result nil))
					   ((>= i *max-events*) result)
					 (setq key (aref *event-key-vector* i))
					 (when (and key (zerop (aref all-events i)))
					   (push key result)))))
			     `(otherwise (binding-event-values
					   (,display ,event-key ,(or keys :universal) ,@arglist)
					   ,(event-clause display peek-p test-form body-forms))))

			 ;; Code for normal clauses
			 (let (true-events) ;; canonicalize event-names
			   (if (consp events)
			       (progn
				 (setq true-events (mapcar #'canonicalize-event-name events))
				 (dolist (event true-events)
				   (setf (aref all-events (get-event-code event)) 1)))
			     (setf true-events (canonicalize-event-name events)
				   (aref all-events (get-event-code true-events)) 1))
			   `(,true-events (binding-event-values
					    (,display ,event-key ,true-events ,@arglist)
					    ,(event-clause display peek-p test-form body-forms))))))))
	       clauses))))))

(defmacro binding-event-values ((display event-key event-keys &rest value-list) &body body)
  ;; Execute BODY with the variables in VALUE-LIST bound to components of the
  ;; EVENT-KEYS events.
  (unless (consp event-keys) (setq event-keys (list event-keys)))
  (flet ((var-key (var) (kintern (if (consp var) (first var) var)))
	 (var-symbol (var) (if (consp var) (second var) var)))
    ;; VARS is an alist of:
    ;;  (component-key ((event-key event-key ...) . extraction-code)
    ;;		       ((event-key event-key ...) . extraction-code) ...)
    ;; There should probably be accessor macros for this, instead of things like cdadr.
    (let ((vars (mapcar #'(lambda (var) (list var)) value-list))
	  (multiple-p nil))
      ;; Fill in the VARS alist with event-keys and extraction-code
      (do ((keys event-keys (cdr keys))
	   (temp nil))
	  ((endp keys))
	(let* ((key (car keys))
	       (binder (case key
			 (:universal #'universal-event-get-macro)
			 (otherwise (aref *event-macro-vector* (get-event-code key))))))
	  (dolist (var vars)
	    (let ((code (funcall binder display event-key (var-key (car var)))))
	      (unless code (format t "~%Warning: ~a isn't a component of the ~s event"
				   (var-key (car var)) key))
	      (if (setq temp (member code (cdr var) :key #'cdr :test #'equal))
		  (push key (caar temp))
		(push `((,key) . ,code) (cdr var)))))))
      ;; Bind all the values
      `(let ,(mapcar #'(lambda (var)
			 (if (cddr var) ;; if more than one binding form
			     (progn (setq multiple-p t)
				    (var-symbol (car var)))
			   (list (var-symbol (car var)) (cdadr var))))
		     vars)
	 ;; When some values come from different places, generate code to set them
	 ,(when multiple-p
	    `(case ,event-key
	       ,@(do ((keys event-keys (cdr keys))
		      (clauses nil) ;; alist of (event-keys bindings)
		      (clause nil nil)
		      (temp))
		     ((endp keys)
		      (dolist (clause clauses)
			(unless (cdar clause) ;; Atomize single element lists
			  (setf (car clause) (caar clause))))
		      clauses)
		   ;; Gather up all the bindings associated with (car keys)
		   (dolist (var vars)
		     (when (cddr var) ;; when more than one binding form
		       (dolist (events (cdr var))
			 (when (member (car keys) (car events))
			   ;; Optimize for event-window being the same as some other binding
			   (if (setq temp (member (cdr events) clause :key #'caddr :test #'equal))
			       (setq clause (nconc clause `((setq ,(car var) ,(second (car temp))))))
			     (push `(setq ,(car var) ,(cdr events)) clause))))))
		   ;; Merge bindings for (car keys) with other bindings
		   (when clause
		     (if (setq temp (member clause clauses :key #'cdr :test #'equal))
			 (push (car keys) (caar temp))
		       (push `((,(car keys)) . ,clause) clauses))))))
	 ,@body))))


;;;-----------------------------------------------------------------------------
;;; Error Handling
;;;-----------------------------------------------------------------------------

(eval-when (eval compile load)
(defparameter
  *xerror-vector*
  '#(unknown-error
     request-error				; 1  bad request code
     value-error				; 2  integer parameter out of range
     window-error				; 3  parameter not a Window
     pixmap-error				; 4  parameter not a Pixmap
     atom-error					; 5  parameter not an Atom
     cursor-error				; 6  parameter not a Cursor
     font-error					; 7  parameter not a Font
     match-error				; 8  parameter mismatch
     drawable-error				; 9  parameter not a Pixmap or Window
     access-error				; 10 attempt to access private resource"
     alloc-error				; 11 insufficient resources
     colormap-error				; 12 no such colormap
     gcontext-error				; 13 parameter not a GContext
     id-choice-error				; 14 invalid resource ID for this connection
     name-error					; 15 font or color name does not exist
     length-error				; 16 request length incorrect;
						;    internal Xlib error
     implementation-error			; 17 server is defective
     ))
)

(defun report-error (display event)
  ;; All errors (synchronous and asynchronous) are processed by calling
  ;; an error handler in the display.  The handler is called with the display
  ;; as the first argument and the error-key as its second argument. If handler is
  ;; an array it is expected to contain handler functions specific to
  ;; each error; the error code is used to index the array, fetching the
  ;; appropriate handler. Any results returned by the handler are ignored;;
  ;; it is assumed the handler either takes care of the error completely,
  ;; or else signals. For all core errors, additional keyword/value argument
  ;; pairs are:
  ;;    :major integer
  ;;    :minor integer
  ;;    :sequence integer
  ;;    :current-sequence integer
  ;; For :colormap, :cursor, :drawable, :font, :GContext, :id-choice, :pixmap, and :window
  ;; errors another pair is:
  ;;    :resource-id integer
  ;; For :atom errors, another pair is:
  ;;    :atom-id integer
  ;; For :value errors, another pair is:
  ;;    :value integer
  (reading-event (event)
    (let* ((error-code (read-card8 1))
	   (handler (display-error-handler display))
	   (handler-function
	     (if (type? handler 'sequence)
		 (elt handler error-code)
	       handler))
	   (error-key (get-error-key display error-code))
	   (params (funcall (get error-key 'error-decode-function)
			    display event)))
      (unwind-protect
	  (apply handler-function display error-key params)
	;; Eat up any remaining server information
	(do ((sequence (read-card16 2))
	     (current-sequence (ldb (byte 16 0) (buffer-request-number display))))
	    ((or (>= sequence current-sequence)
		 (buffer-input display buffer-bbuf 0 *replysize* 0)))
	  (case (read-card8 0) ;; type
	    (0					; Another error
	      (report-error display event))
	    (1					; Reply
	      (return t))
	    (otherwise				; Event
	      ;; Push the event in the input buffer on the display's event queue
	      (let ((event (allocate-event)))
		;; Copy into event from reply buffer
		(buffer-replace (reply-ibuf8 event)
				(reply-ibuf8 (buffer-reply-buffer display))
				0
				*replysize*)
		(setf (event-code event)
		      (get-internal-event-code display (read-card8 0)))
		(enqueue-event event display)))))))))

(defun request-name (code &optional display)
  (if (< code (length *request-names*))
      (aref *request-names* code)
    (dolist (extension (and display (display-extension-alist display)) "unknown")
      (when (= code (second extension))
	(return (first extension))))))

(define-condition request-error (x-error)
  (display
   error-key
   major
   minor
   sequence
   current-sequence)
  (:report report-request-error))
  
(defun report-request-error (condition stream)
  (let ((error-key (request-error-error-key condition))
	(major (request-error-major condition))
	(minor (request-error-minor condition))
	(sequence (request-error-sequence condition))
	(current-sequence (request-error-current-sequence condition)))		   
    (format stream "~a in ~:[request ~d (last request was ~d) ~;current request~2* ~] Code ~d.~d [~a]"
	    error-key (= sequence current-sequence) sequence current-sequence major minor
	    (request-name major (request-error-display condition)))))

(define-condition resource-error (request-error)
  (resource-id)
  (:report (lambda (condition stream)
	     (report-request-error condition stream)
	     (format stream " ID #x~x" (resource-error-resource-id condition)))))  

(define-condition unknown-error (request-error)
  (error-code)
  (:report (lambda (condition stream)
	     (report-request-error condition stream)
	     (format stream " Error Code ~d." (unknown-error-error-code condition)))))

(define-condition access-error (request-error))

(define-condition alloc-error (request-error))

(define-condition atom-error (request-error)
  (atom-id)
  (:report (lambda (condition stream)
	     (report-request-error condition stream)
	     (format stream " Atom-ID #x~x" (atom-error-atom-id condition)))))

(define-condition colormap-error (resource-error))

(define-condition cursor-error (resource-error))

(define-condition drawable-error (resource-error))

(define-condition font-error (resource-error))

(define-condition gcontext-error (resource-error))

(define-condition id-choice-error (resource-error))

(define-condition illegal-request-error (request-error))

(define-condition length-error (request-error))

(define-condition match-error (request-error))

(define-condition name-error (request-error))

(define-condition pixmap-error (resource-error))

(define-condition value-error (request-error)
  (value)
  (:report (lambda (condition stream)
	     (report-request-error condition stream)
	     (format stream " Value ~d." (value-error-value condition)))))

(define-condition window-error (resource-error))

(define-condition implementation-error (request-error))

;;-----------------------------------------------------------------------------
;; Internal error conditions signaled by CLX

(define-condition type-error (x-error)
  (object
   type
   type-string)
  (:report (lambda (condition stream)
	     (format stream "~s isn't ~@[a ~] ~s"
		     (type-error-object condition)
		     (type-error-type-string condition)
		     (type-error-type condition)))))

(define-condition closed-display (x-error)
  (display)
  (:report (lambda (condition stream)
	     (format stream "Attempt to use closed display ~s"
		     (closed-display-display condition)))))

(define-condition lookup-error (x-error)
  (id display type object)
  (:report (lambda (condition stream)
	     (format stream "ID ~d from display ~s should have been a ~s, but was ~s"
		     (lookup-error-id condition)
		     (lookup-error-display condition)
		     (lookup-error-type condition)
		     (lookup-error-object condition)))))  

(define-condition connection-failure (x-error)
  (major-version
   minor-version
   host
   display
   reason)
  (:report (lambda (condition stream)
	     (format stream "Connection failure to X~d.~d server ~a display ~d: ~a"
		     (connection-failure-major-version condition)
		     (connection-failure-minor-version condition)
		     (connection-failure-host condition)
		     (connection-failure-display condition)
		     (connection-failure-reason condition)))))
  
(define-condition reply-length-error (x-error)
  (reply-length
   expected-length
   display)
  (:report (lambda (condition stream)
	     (format stream "Reply length was ~d when ~d words were expected for display ~s"
		     (reply-length-error-reply-length condition)
		     (reply-length-error-expected-length condition)
		     (reply-length-error-display condition)))))  

(define-condition reply-timeout (x-error)
  (timeout
   display)
  (:report (lambda (condition stream)
	     (format stream "Timeout after waiting ~d seconds for a reply for display ~s"
		     (reply-timeout-timeout condition)
		     (reply-timeout-display condition)))))  

(define-condition server-disconnect (x-error)
  (display)
  (:report (lambda (condition stream)
	     (format stream "Server disconnect for display ~s"
		     (server-disconnect-display condition)))))

(define-condition sequence-error (x-error)
  (display
   req-sequence
   msg-sequence)
  (:report (lambda (condition stream)
	     (format stream "Reply out of sequence for display ~s.~%  Expected ~d, Got ~d"
		     (sequence-error-display condition)
		     (sequence-error-req-sequence condition)
		     (sequence-error-msg-sequence condition)))))  

(define-condition unexpected-reply (x-error)
  (display
   msg-sequence
   req-sequence
   length)
  (:report (lambda (condition stream)
	     (format stream "Display ~s received a server reply when none was expected.~@
		             Last request sequence ~d Reply Sequence ~d Reply Length ~d bytes."
		  (unexpected-reply-display condition)
		  (unexpected-reply-req-sequence condition)
		  (unexpected-reply-msg-sequence condition)
		  (unexpected-reply-length condition)))))

(define-condition missing-parameter (x-error)
  (parameter)
  (:report (lambda (condition stream)
	     (let ((parm (missing-parameter-parameter condition)))
	       (if (consp parm)
		   (format stream "One or more of the required parameters ~a is missing."
			   parm)
		 (format stream "Required parameter ~a is missing or null." parm))))))

;; This can be signalled anywhere a pseudo font access fails.
(define-condition invalid-font (x-error)
  (font)
  (:report (lambda (condition stream)
	     (format stream "Can't access font ~s" (invalid-font-font condition)))))

(define-condition device-busy (x-error)
  (display)
  (:report (lambda (condition stream)
	     (format stream "Device busy for display ~s"
		     (device-busy-display condition)))))

(define-condition unimplemented-event (x-error)
  (display
   event-code)
  (:report (lambda (condition stream)
	     (format stream "Event code ~d not implemented for display ~s"
		     (unimplemented-event-event-code condition)
		     (unimplemented-event-display condition)))))

(define-condition undefined-event (x-error)
  (display
   event-name)
  (:report (lambda (condition stream)
	     (format stream "Event code ~d undefined for display ~s"
		     (undefined-event-event-name condition)
		     (undefined-event-display condition)))))

(define-condition absent-extension (x-error)
  (name display)
  (:report (lambda (condition stream)
	     (format stream "Extension ~a isn't defined for display ~s"
		     (absent-extension-name condition)
		     (absent-extension-display condition)))))

(define-condition inconsistent-parameters (x-error)
  (parameters)
  (:report (lambda (condition stream)
	     (format stream "inconsistent-parameters:~{ ~s~}"
		     (inconsistent-parameters-parameters condition)))))

(defun get-error-key (display error-code)
  ;; Return the error-key associated with error-code
  (if (< error-code (length *xerror-vector*))
      (aref *xerror-vector* error-code)
    ;; Search the extensions for the error
    (dolist (entry (display-extension-alist display) 'unknown-error)
      (let* ((event-name (first entry))
	     (first-error (fourth entry))
	     (errors (third (assoc event-name *extensions*))))
	(declare (type keyword event-name)
		 (type card8 first-error)
		 (type list errors))
	(when (and errors
		   (<= first-error error-code (+ first-error (1- (length errors)))))
	  (return (nth (- error-code first-error) errors)))))))

(defmacro define-error (error-key function)
  ;; Associate a function with ERROR-KEY which will be called with
  ;; parameters DISPLAY and REPLY-BUFFER and returns a plist of
  ;; keyword/value pairs which will be passed on to the error handler.
  ;; A compiler warning is printed when ERROR-KEY is not defined in a
  ;; preceding DEFINE-EXTENSION.
  ;; Note: REPLY-BUFFER may used with the READING-EVENT and READ-type
  ;;       macros for getting error fields. See DECODE-CORE-ERROR for
  ;;       an example.
  (declare (type symbol error-key)
	   (type function function))
  ;; First ensure the name is for a declared extension
  (unless (or (find error-key *xerror-vector*)
	      (dolist (extension *extensions*)
		(when (member error-key (third extension))
		  (return t))))
    (x-type-error error-key 'error-key))
  `(setf (get ',error-key 'error-decode-function) (function ,function)))

;; All core errors use this, so we make it available to extensions.
(defun decode-core-error (display event &optional arg)
  ;; All core errors have the following keyword/argument pairs:
  ;;    :major integer
  ;;    :minor integer
  ;;    :sequence integer
  ;;    :current-sequence integer
  ;; In addition, many have an additional argument that comes from the
  ;; same place in the event, but is named differently.  When the ARG
  ;; argument is specified, the keyword ARG with card32 value starting
  ;; at byte 4 of the event is returned with the other keyword/argument
  ;; pairs.
  (declare (type display display)
	   (type reply-buffer event)
	   (type (or null keyword) arg))
  (declare-values keyword/arg-plist)
  (reading-event (event)
    (let* ((sequence (read-card16 2))
	   (minor-code (read-card16 8))
	   (major-code (read-card8 10))
	   (current-sequence (ldb (byte 16 0) (buffer-request-number display)))
	   (result (list :major major-code
			 :minor minor-code
			 :sequence sequence
			 :current-sequence current-sequence)))
      (when arg
	(setq result (list* arg (read-card32 4) result)))
      result)))

(defun decode-resource-error (display event)
  (decode-core-error display event :resource-id))

(define-error unknown-error
  (lambda (display event)
    (list* :error-code (aref (reply-ibuf8 event) 1)
	   (decode-core-error display event))))

(define-error request-error decode-core-error)		; 1  bad request code

(define-error value-error				; 2  integer parameter out of range
  (lambda (display event)
    (decode-core-error display event :value)))

(define-error window-error decode-resource-error)	; 3  parameter not a Window

(define-error pixmap-error decode-resource-error)	; 4  parameter not a Pixmap

(define-error atom-error				; 5  parameter not an Atom
  (lambda (display event)
    (decode-core-error display event :atom-id)))

(define-error cursor-error decode-resource-error)	; 6  parameter not a Cursor

(define-error font-error decode-resource-error)		; 7  parameter not a Font

(define-error match-error decode-core-error)		; 8  parameter mismatch

(define-error drawable-error decode-resource-error)	; 9  parameter not a Pixmap or Window

(define-error access-error decode-core-error)		; 10 attempt to access private resource"

(define-error alloc-error decode-core-error)		; 11 insufficient resources

(define-error colormap-error decode-resource-error)	; 12 no such colormap

(define-error gcontext-error decode-resource-error)	; 13 parameter not a GContext

(define-error id-choice-error decode-resource-error)	; 14 invalid resource ID for this connection

(define-error name-error decode-core-error)		; 15 font or color name does not exist

(define-error length-error decode-core-error)		; 16 request length incorrect;
							;    internal Xlib error

(define-error implementation-error decode-core-error)	; 17 server is defective
