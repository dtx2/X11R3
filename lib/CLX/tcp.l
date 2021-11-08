;; -*- Mode: LISP; Syntax: Common-lisp; Package: TCP; Base: 10; Lowercase: Yes -*-;;
;;
;; kcl lisp interface to the socket code
;; Roman Budzianowski
;; Project Athena MIT
;;
(in-package 'tcp)
(export 'open-tcp-stream)
(defentry open-tcp-stream (object object) (object open_tcp_stream))
;;
