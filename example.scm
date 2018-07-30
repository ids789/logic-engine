;; ------------------------------
;; An example logic-engine Script
;; ------------------------------

(import (chibi process))
(display "Example Script Started...\n")

;; logic-engine looks for a knxd url in the 'knx-url' symbol
(define knx-url "ip:192.168.1.15:1234")

(define-item 'light-1 "1/2/41")
(define-item 'light-2 "1/2/43")

;; Switch some lights on and off
(knx:send light-1 #t) (display "Lights On\n")
(sleep 3)
(knx:send light-2 #f) (display "Lights Off\n")

;; Display the value of something
(display "Value:")
(display (knx:read light-1))
(newline)

;; Create some callbacks for group events
(knx:watch
 (list
  (cons light-1
		(lambda (state)
		  (display "Script sees light 1 changed to ")
		  (display state)
		  (newline)))
  (cons light-2
		(lambda (state)
		  (display "Script sees light 2 changed to ")
		  (display state)
		  (newline)))))
