;; -*- Mode: LISP; Syntax: Common-lisp; Package: TCP; Base: 10; Lowercase: Yes -*-;;
;;
;; Interface to tcp for kcl
;;
;; Roman Budzianowski
;; Project Athena MIT
;;
;; First you have to compile the "c" files : kcltcp.c and socket.c
;; Then you have to compile the lisp file tcp.lisp 
;; Finally you load this file into kcl and you can call the
;; function (tcp:open-tcp-stream port)
;;
(in-package "TCP" :use '("LISP"))
;; It is assumed that the files reside in the tcp subdirectory
(si:faslink "tcp.o" "kcltcp.o socket.o -lc")
;;
