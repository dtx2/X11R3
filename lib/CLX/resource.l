;;; -*- Mode:Common-Lisp; Package:XLIB; Syntax:COMMON-LISP; Base:10; Lowercase:T -*-

;; RESOURCE - Lisp version of XLIB's Xrm resource manager

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

(export '(resource-database
	   resource-database-timestamp
	   make-resource-database
	   resource-key
	   get-resource
	   get-search-table
	   get-search-resource
	   add-resource
	   delete-resource 
	   map-resource 
	   merge-resources
	   read-resources
	   write-resources
	   wm-resources
	   set-wm-resources))

#+clue ;; for CLUE only
(defparameter *resource-subclassp* nil
  "When non-nil and no match found, search superclasses.")

;; The C version of this uses a 64 entry hash table at each entry.
;; Small hash tables loose in Lisp, so we do linear searches on lists.

;; This code uses resource-database-NAME for resource lookup, and 
;; resource-database-STRING when reading/writing resources.  This
;; is to enable us to upcase and intern the string for fast resource
;; lookup EQ tests while maintaing the original string for 
;; write-resources [it should be possible to read a upper/lower
;; case resource file, modify it and write it out correctly]

(defstruct (resource-database (:copier nil) (:print-function print-resource)
			      (:constructor make-resource-database-internal)
			      #+explorer (:callable-constructors nil)
			      )
  (name nil :type symbol :read-only t)
  (string "" :type stringable :read-only t)
  (value nil)
  (tight nil :type list) ;; List of resource-database
  (loose nil :type list) ;; List of resource-database
  )

;; The value slot of the top-level resource-database structure is used for a time-stamp.

(defun make-resource-database ()
  ;; Make a resource-database with initial timestamp of 0
  (make-resource-database-internal :name :top-level :string "Top-Level" :value 0))

(proclaim '(inline resource-database-timestamp))
(defun resource-database-timestamp (database)
  (declare (type resource-database database))
  (resource-database-value database))

(defun incf-resource-database-timestamp (database)
  ;; Increment the timestamp
  (declare (type resource-database database))
  (let ((timestamp (resource-database-value database)))
    (setf (resource-database-value database)
	  (if (= timestamp most-positive-fixnum)
	      most-negative-fixnum
	    (1+ timestamp)))))

;; Without this, resources take forever to print...
(defun print-resource (database stream depth)
  (declare (type resource-database database))
  (declare (ignore depth))
  (princ "#<Resource " stream)
  (princ (resource-database-name database))
  (when (resource-database-value database)
    (princ " " stream)
    (prin1 (resource-database-value database)))
  (princ ">"))

;; DEBUG FUNCTION  (not exported)
(defun print-db (entry &optional (level 0) type)
  ;; Debug function to print a resource database
  (format t "~%~v@t~s~:[~; *~]~@[ Value ~s~]"
	  level
	  (resource-database-name entry)
	  (eq type 'loose)
	  (resource-database-value entry))
  (when (resource-database-tight entry)
    (dolist (tight (resource-database-tight entry))
      (print-db tight (+ 2 level) 'tight)))
  (when (resource-database-loose entry)
    (dolist (loose (resource-database-loose entry))
      (print-db loose (+ 2 level) 'loose))))

;; DEBUG FUNCTION
#+comment
(defun print-search-table (table)
  (terpri)
  (dolist (dbase-list table)
    (format t "~%~s" dbase-list)
    (dolist (db dbase-list)
      (print-db db)
      (dolist (dblist table)
	(unless (eq dblist dbase-list)
	  (when (member db dblist)
	    (format t "  duplicate at ~s" db))))
      )))

(proclaim '(inline resource-key))
(defun resource-key (stringable)
  ;; Ensure STRINGABLE is a keyword.
  (declare (type stringable stringable))
  (if (keywordp stringable)
      (the keyword stringable)
    (resource-key-internal stringable)))

(defun resource-key-internal (stringable)
  ;; Ensure STRINGABLE is a keyword.
  (declare (type stringable stringable))
  (if (symbolp stringable)
      (kintern stringable)
    (kintern (string-upcase (the string stringable)))))

;; This is in the inner-loop of the resource search.
;; Try to be as fast as possible.
(proclaim '(inline stringable-equal))
(defun stringable-equal (a b)
  ;; Compare two stringables.
  ;; Ignore case when comparing to a symbol.
  (declare (type stringable a b))
  (declare-values boolean)
  (macrolet (#+(or ti lmi) ;; Over twice as fast as common-lisp string-equal
	     (string-equal (a b)
	       `(si:%string-equal ,a 0 ,b 0 nil)))
    (if (symbolp a)
	(if (symbolp b)
	    (equal (the string (symbol-name a)) (the string (symbol-name b)))
	  (string-equal (the string (symbol-name a)) (the string b)))
      (if (symbolp b)
	  (string-equal (the string a) (the string (symbol-name b)))
	#+lispm (equal a b) ;; EQUAL is microcoded on lispm's
	#-lispm (string= (the string a) (the string b))))))


;;;-----------------------------------------------------------------------------
;;; Add/delete resource

(defun add-resource (database name-list value)
  ;; name-list is a list of either strings or symbols. If a symbol, 
  ;; case-insensitive comparisons will be used, if a string,
  ;; case-sensitive comparisons will be used.  The symbol '* or
  ;; string "*" are used as wildcards, matching anything or nothing.
  (declare (type resource-database database)
	   (type list name-list) ;; (list stringable)
	   (type t value))
  (unless value (error "Null resource values are ignored"))
  (incf-resource-database-timestamp database)
  (do* ((list name-list (cdr list))
	(string (car list) (car list))
	(node database)
	(loose-p nil))
       ((endp list)
	(setf (resource-database-value node) value))
    ;; Key is the first name that isn't *
    (if (or (equal string "*") (eq string '*))
	(setq loose-p t)
      ;; find the entry associated with name
      (progn
	(do ((entry (if loose-p
			(resource-database-loose node)
		      (resource-database-tight node))
		    (cdr entry))
	     ;; Terminal names are ALWAYS in the keyword package
	     (name (resource-key string)))
	    ((endp entry)
	     ;; Entry not found - create a new one
	     (setq entry (make-resource-database-internal :name name :string string))
	     (if loose-p
		 (push entry (resource-database-loose node))
	       (push entry (resource-database-tight node)))
	     (setq node entry))
	  (when (stringable-equal string (resource-database-string (car entry)))
	    ;; Found entry - use it
	    (return (setq node (car entry)))))
	(setq loose-p nil)))))


(defun delete-resource (database name-list)
  (declare (type resource-database database)
	   (type list name-list))
  (incf-resource-database-timestamp database)
  (delete-resource-internal database name-list))

(defun delete-resource-internal (database name-list)
  (declare (type resource-database database)
	   (type list name-list)) ;; (list stringable)
  (do* ((list name-list (cdr list))
	(string (car list) (car list))
	(node database)
	(loose-p nil))
       ((endp list) nil)
    ;; Key is the first name that isn't *
    (if (or (equal string "*") (eq string '*))
	(setq loose-p t)
      ;; find the entry associated with name
      (progn 
	(do* ((first-entry (if loose-p
			       (resource-database-loose node)
			     (resource-database-tight node)))
	      (entry-list first-entry (cdr entry-list))
	      (entry (car entry-list) (car entry-list)))
	     ((endp entry-list)
	      ;; Entry not found - exit
	      (return-from delete-resource-internal nil))
	  (when (stringable-equal string (resource-database-string entry))
	    (when (cdr list) (delete-resource-internal entry (cdr list)))
	    (when (and (null (resource-database-loose entry))
		       (null (resource-database-tight entry)))
	      (if loose-p
		  (setf (resource-database-loose node)
			(delete entry (resource-database-loose node) :test #'eq :count 1))
		(setf (resource-database-tight node)
		      (delete entry (resource-database-tight node) :test #'eq :count 1))))
	    (return-from delete-resource-internal t)))
	(setq loose-p nil)))))

;;;-----------------------------------------------------------------------------
;;; Get Resource

(defun get-resource (database value-name value-class full-name full-class)
  ;; Return the value of the resource in DATABASE whose partial name
  ;; most closely matches (append full-name (list value-name)) and
  ;;                      (append full-class (list value-class)).
  (declare (type resource-database database)
	   (type stringable value-name value-class)
	   (type list full-name full-class)) ;; (list stringable)
  (declare-values value)
  (let ((names (append full-name (list (resource-key value-name))))
	(classes (append full-class (list (resource-key value-class)))))
    (let* ((result (get-entry (resource-database-tight database) (resource-database-loose database)
			      names classes)))
      (when result
	(resource-database-value result)))))

(defun get-entry-lookup (table name names classes)
  (declare (type list table names classes)
	   (symbol name))
  (declare (inline get-entry-lookup))
  (dolist (entry table)
    (declare (type resource-database entry))
    (when (stringable-equal name (resource-database-name entry))
      (if (null (cdr names))
	  (return entry)
	(let ((result (get-entry (resource-database-tight entry) (resource-database-loose entry)
				 (cdr names) (cdr classes))))
	  (declare (type (or null resource-database) result))
	  (when result
	    (return result)
	    ))))))

(defun get-entry (tight loose names classes &aux result)
  (declare (type list tight loose names classes))
;;  (DECLARE (inline get-entry-lookup))
  (declare (inline resource-key))
  (let ((name (car names))
	(class (car classes)))
    (declare (type symbol name class))
    (cond ((and tight
		(get-entry-lookup tight name names classes)))
	  ((and loose
		(get-entry-lookup loose name names classes)))
	  ((and tight
		(not (equal name class))
		(get-entry-lookup tight class names classes)))
	  ((and loose
		(not (equal name class))
		(get-entry-lookup loose class names classes)))
	  #+clue ;; for CLUE only
	  ((and *resource-subclassp*
		(or loose tight)
		(dolist (class (cluei::class-all-superclasses class))
		  (when tight
		    (when (setq result (get-entry-lookup tight class names classes))
		      (return result)))
		  (when loose
		    (when (setq result (get-entry-lookup loose class names classes))
		      (return result))))))	
	  (loose
	   (loop
	     (pop names) (pop classes)
	     (unless (and names classes) (return nil))
	     (setq name (car names)
		   class (car classes))
	     (when (setq result (get-entry-lookup loose name names classes))
	       (return result))
	     (when (and (not (equal name class))
			(setq result (get-entry-lookup loose class names classes)))
	       (return result))
	     #+clue ;; for CLUE only
	     (when *resource-subclassp*
	       (dolist (class (cluei::class-all-superclasses class))
		 (when (setq result (get-entry-lookup loose class names classes))
		   (return-from get-entry result))))
	     )))))


;;;-----------------------------------------------------------------------------
;;; Get-resource with search-table

(defun get-search-resource (table name class)
  ;; (get-search-resource (get-search-table database full-name full-class) value-name value-class)
  ;; is equivalent to 
  ;; (get-resource database value-name value-class full-name full-class)
  ;; But since most of the work is done by get-search-table, get-search-resource is MUCH
  ;; faster when getting several resources with the same full-name/full-class
  (declare (type list table)
	   (type stringable name class))
  (declare (inline resource-key))
  (let ((name (resource-key name))
	(class (and class (resource-key class))))
    (declare (type stringable name class))
    (block exit
      (dolist (dbase-list table)
	(declare (type list dbase-list))
	(dolist (dbase dbase-list)
	  (declare (type resource-database dbase))
	  (when (eq name (resource-database-name dbase))
	    (return-from exit (resource-database-value dbase))))
	(when (and class (not (eq name class)))
	  (dolist (dbase dbase-list)
	    (declare (type resource-database dbase))
	    (when (eq class (resource-database-name dbase))
	      (return-from exit (resource-database-value dbase)))))))))

(defun get-search-table (database full-name full-class)
  ;; Return a search table for use with get-search-resource.
  (declare (type resource-database database)
	   (type list full-name full-class)) ;; (list stringable)
  (declare-values value)
  (let* ((tight (resource-database-tight database))
	 (loose (resource-database-loose database))
	 (result (cons nil nil))
	 (*result* result))
    (declare (type list tight loose)
	     (type cons result)
	     (special *result*))
    (when (or tight loose)
      (when full-name
	(get-tables tight loose full-name full-class))
      ;;(vector-push loose table) ;; This catches entries like: (* foreground) :white
      )
    (cdr result)))

(proclaim '(inline get-tables-lookup))
(defun get-tables-lookup (dbase name names classes)
  (declare (type list dbase names classes)
	   (type symbol name))
  (declare (optimize speed))
  (declare (special *result*))
  (dolist (entry dbase)
    (declare (type resource-database entry))
    (when (stringable-equal name (resource-database-name entry))
      (let ((tight (resource-database-tight entry))
	    (loose (resource-database-loose entry)))
	(declare (type list tight loose))
	(when (or tight loose)
	  (if (cdr names)
	      (get-tables tight loose (cdr names) (cdr classes))
	    (when tight
	      (let ((result *result*)) ;; Put tight at end of *result*
		(setf (cdr result) (setq *result* (cons tight nil))))))
	  (when loose
	    (let ((result *result*)) ;; Put loose at end of *result*
	      (setf (cdr result) (setq *result* (cons loose nil))))))))))

(defun get-tables (tight loose names classes)
  (declare (type list tight loose names classes))
  (declare (inline resource-key)
	   (inline get-tables-lookup)
	   (optimize speed))
  (let ((name (car names))
	(class (car classes)))
    (declare (type symbol name class))
    (when tight
      (get-tables-lookup tight name names classes))
    (when loose
      (get-tables-lookup loose name names classes))
    (when (and tight (not (equal name class)))
      (get-tables-lookup tight class names classes))
    (when (and loose (not (equal name class)))
      (get-tables-lookup loose class names classes))
    #+clue ;; for CLUE only
    (when *resource-subclassp*
      (dolist (class (cluei::class-all-superclasses class))
	(declare (type symbol class))
	(setq class class)
	(when tight
	  (get-tables-lookup tight class names classes))
	(when loose
	  (get-tables-lookup loose class names classes))))
    (when loose
      (loop
	(pop names) (pop classes)
	(unless (and names classes) (return nil))
	(setq name (car names)
	      class (car classes))
	(get-tables-lookup loose name names classes)
	(unless (equal name class)
	  (get-tables-lookup loose class names classes))
	#+clue ;; for CLUE only
	(when *resource-subclassp*
	  (dolist (class (cluei::class-all-superclasses class))
	    (get-tables-lookup loose class names classes)))
	))))


;;;-----------------------------------------------------------------------------
;;; Utility functions

(defun map-resource (database function &rest args)
  ;; Call FUNCTION on each resource in DATABASE.
  ;; FUNCTION is called with arguments (name-list value . args)
  (declare (type resource-database database)
	   (type (function (list t &rest t) t) function))
  (declare-values nil)
  (labels ((map-resource-internal (database function args name)
	     (let ((tight (resource-database-tight database))
		   (loose (resource-database-loose database)))
	       (dolist (resource tight)
		 (let ((value (resource-database-value resource))
		       (name (append name (list (resource-database-name resource)))))
		   (if value
		       (apply function name value args)
		     (map-resource-internal resource function args name))))
	       (dolist (resource loose)
		 (let ((value (resource-database-value resource))
		       (name (append name (list '* (resource-database-name resource)))))
		   (if value
		       (apply function name value args)
		     (map-resource-internal resource function args name)))))))
    (map-resource-internal database function args nil)))

(defun merge-resources (database with-database)
  (declare (type resource-database database with-database))
  (declare-values resource-database)
  (map-resource #'add-resource database with-database)
  with-database)

(defun char-memq (key char)
  ;; Used as a test function for POSITION
  (declare (type string-char char))
  (member char key))

(eval-when (compile eval) ;; Not needed after compilation
(defmacro resource-with-open-file ((stream pathname &rest options) &body body)
  ;; Private WITH-OPEN-FILE, which, when pathname is a stream, uses it as the stream
  (let ((abortp (gensym))
	(streamp (gensym)))
    `(let* ((,abortp t)
	    (,streamp (streamp pathname))
	    (,stream (if ,streamp pathname (open ,pathname ,@options))))
       (unwind-protect
	   (progn
	     ,@body
	     (setq ,abortp nil))
	 (unless ,streamp
	   (close stream :abort ,abortp))))))
) ;; end eval-when

(defun read-resources (database pathname &key key test test-not)
  ;; Merges resources from a file in standard X11 format with DATABASE.
  ;; KEY is a function used for converting value-strings, the default is
  ;; identity.  TEST and TEST-NOT are predicates used for filtering
  ;; which resources to include in the database.  They are called with
  ;; the name and results of the KEY function.
  (declare (type resource-database database)
	   (type (or pathname string stream) pathname)
	   (type (or null (function (string) t)) key)
	   (type (or null (function (list t) boolean))
                 test test-not))
  (declare-values resource-database)
  (resource-with-open-file (stream pathname)
    (loop
      (let ((string (read-line stream nil :eof)))
	(declare (type string string))
	(when (eq string :eof) (return database))
	(let* ((end (length string))
	       (i (position '(#\tab #\space) string :test-not #'char-memq :end end))
	       (term nil))
	  (declare (type array-index end)
		   (type (or null array-index) i term))
	  (when i ;; else blank line
	    (case (char string i)
	      (#\! nil)  ;; Comment - skip
	      (#.(int-char 0) nil) ;; terminator for C strings - skip
	      (#\#       ;; Include
	       (setq term (position '(#\tab #\space) string :test #'char-memq
				    :start i :end end))
	       (if (not (string-equal string "#INCLUDE" :start1 i :end1 term))
		   (format t "~%Resource File error. Ignoring: ~a" string)
		 (let ((path (merge-pathnames (subseq string (1+ term)) (truename stream))))
		   (read-resources database path :key key :test test :test-not test-not))))
	      (otherwise
	       (multiple-value-bind (name-list value)
		   (parse-resource string i end)
		 (when
		   (cond (test (funcall test name-list value))
			 (test-not (not (funcall test-not name-list value)))
			 (t t))
		   (when key (setq value (funcall key value)))
		   (add-resource database name-list value)))))))))))

(defun parse-resource (string &optional (start 0) end)
  ;; Parse a resource specfication string into a list of names and a value string
  (declare (type string string)
	   (type array-index start)
	   (type (or null array-index) end))
  (declare-values name-list value)
  (do ((i start)
       (end (or end (length string)))
       (term)
       (name-list)
       (previous))
      ((>= i end))
    (declare (type array-index end)
	     (type (or null array-index) i term))
    (setq term (position '(#\. #\* #\:) string
			 :test #'char-memq :start i :end end))
    (case (and term (char string term))
      ;; Name seperator
      (#\. (when (> term i)
	     (push (subseq string i term) name-list)))
      ;; Wildcard seperator
      (#\* (when (> term i)
	     (push (subseq string i term) name-list))
	   (push '* name-list))
      ;; Value seperator
      (#\: (push (subseq string i term) name-list)
	   (let* ((start (position '(#\tab #\space) string
				   :test-not #'char-memq
				   :start (1+ term) :end end))
		  (value (and (> end start) (subseq string start))))
	     (return (values (nreverse name-list) (string-right-trim '(#\tab #\space) value))))))
    (setq previous (char string term))
    (setq i (1+ term))))

(defun write-resources (database pathname &key write test test-not)
  ;; Write resources to PATHNAME in the standard X11 format.
  ;; WRITE is a function used for writing values, the default is #'princ
  ;; TEST and TEST-NOT are predicates used for filtering which resources
  ;; to include in the database.  They are called with the name and value.
  (declare (type resource-database database)
	   (type (or pathname string stream) pathname)
	   (type (or null (function (string stream) t)) write)
	   (type (or null (function (list t) boolean))
                 test test-not))
  (resource-with-open-file (stream pathname :direction :output)
    (map-resource
      database
      #'(lambda (name-list value stream write test test-not)
	  (when
	    (cond (test (funcall test name-list value))
		  (test-not (not (funcall test-not name-list value)))
		  (t t))
	    (let ((previous (car name-list)))
	      (princ previous stream)
	      (dolist (name (cdr name-list))
		(unless (or (eq name '*) (eq previous '*))
		  (write-char #\. stream))
		(setq previous name)
		(princ name stream)))
	    (write-string ":	" stream)
	    (funcall write value stream)
	    (terpri stream)))
      stream (or write #'princ) test test-not))
  database)


(defun wm-resources (database window &key key test test-not)
  ;; Takes the resources associated with the RESOURCE_MANAGER property
  ;; of WINDOW (if any) and merges them with DATABASE.
  ;; KEY is a function used for converting value-strings, the default is
  ;; identity.  TEST and TEST-NOT are predicates used for filtering
  ;; which resources to include in the database.  They are called with
  ;; the name and results of the KEY function.
  (declare (type resource-database database)
	   (type window window)
	   (type (or null (function (string) t)) key)
	   (type (or null (function (list t) boolean))
                 test test-not))
  (declare-values resource-database)
  (let ((string (get-property window :resource_manager :type :string
			      :result-type 'string :transform #'xlib::card8->char)))
    (when string
      (with-input-from-string (stream string)
	(read-resources database stream :key key :test test :test-not test-not)))))

(defun set-wm-resources (database window &key write test test-not)
  ;; Sets the resources associated with the RESOURCE_MANAGER property
  ;; of WINDOW.
  ;; WRITE is a function used for writing values, the default is #'princ
  ;; TEST and TEST-NOT are predicates used for filtering which resources
  ;; to include in the database.  They are called with the name and value.
  (declare (type resource-database database)
	   (type window window)
	   (type (or null (function (string stream) t)) write)
	   (type (or null (function (list t) boolean))
                 test test-not))
  (xlib::set-string-property
    window :resource_manager
    (with-output-to-string (stream)
      (write-resources database stream :write write
		       :test test :test-not test-not))))
