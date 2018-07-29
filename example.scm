;; ------------------------------
;; An example logic-engine Script
;; ------------------------------

(import (chibi process))
(display "Example Script Started...\n")

;; logic-engine looks for a knxd url in the 'knx-url' symbol
(define knx-url "ip:192.168.1.15:1234")

;; Switch some lights on and off
(knx:send "1/1/41" #t) (display "Lights On\n")
(sleep 3)
(knx:send "1/1/41" #f) (display "Lights Off\n")

;; Display the value of something
(display "Value:")
(display (knx:read "4/5/2"))
(newline)

;; Create some callbacks for group events
(knx:watch
 (list
  (cons "1/1/41"
		(lambda (state)
		  (display "Script sees light 1 changed to ")
		  (display state)
		  (newline)))
  (cons "1/1/42"
		(lambda (state)
		  (display "Script sees light 2 changed to ")
		  (display state)
		  (newline)))))
