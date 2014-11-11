;;(string-starts-with "abc" "abc") -> "abc"
;;(string-starts-with "abc" "c")   -> #f
;;(string-starts-with "abc" "a")   -> "abc"
(define string-starts-with
  (λ (string prefix)
     (letrec
         ((loop
           (λ (str pre)
              (cond ((null? pre) string)
                    ((null? str) #f)
                    ((eq? (car str)
                          (car pre))
                     (loop (cdr str) (cdr pre)))
                    (else #f)))))
       (loop string prefix))))

;;(string-contains "abc" "abc")    -> "abc"
;;(string-contains "abc" "c")      -> "abc"
;;(string-contains "abc" "b")      -> "abc"
;;(string-contains "abc" "f")      -> #f
;;(string-contains "abc" "abcd")   -> #f
(define string-contains
  (λ (string substring)
     (letrec
         ((loop
           (λ (str sub)
              (cond ((null? str) #f)
                    ((and (eq? (car str)
                               (car sub))
                          (string-starts-with str sub)) string)
                    (else (loop (cdr str) sub))))))
       (loop string substring))))

(define eat-until
  (λ (string sub)
     (letrec
         ((loop
           (λ (str)
              (cond ((null? str) null)
                    ((string-starts-with str sub)
                     (drop str (length sub)))
                    (else (loop (cdr str)))))))
       (loop string))))


(define grep-relevant-lines
  (λ (lines)
     (let* ((raws '("Saving to:" "Gas Usage:" "Out of gas!"))
            (strings (map string->list raws))
            (epoch (list-ref strings 0))
            (usage (list-ref strings 1))
            (outofgas (list-ref strings 2)))
       (filter
        (λ (line)
           (or (string-contains line epoch)
               (string-contains line usage)
               (string-contains line outofgas)))
        lines))))

(define get-epoch-from-line
  (λ (line)
     (let* ((uptoasave (eat-until line (string->list "autosave/")))
            (rest (eat-until uptoasave '(#\-))))
       (letrec
           ((loop
             (λ (ls acc)
                (cond ((null? ls) null)
                      ((eq? (car ls) #\.) (reverse acc))
                      (else (loop (cdr ls) (cons (car ls) acc)))))))
         (loop rest null)))))

(define get-usage-from-line
  (λ (line)
     (eat-until line (string->list "Gas Usage: "))))

(define sum
  (curry apply +))

(define average
  (λ (xs)
     (exact->inexact
      (if (zero? (length xs)) 0
          (/ (sum xs) (length xs))))))

(define collect-on-lines
  (λ (lines)
     (let* ((rels (grep-relevant-lines lines))
            (raws '("Saving to:" "Gas Usage:" "Out of gas!"))
            (strings (map string->list raws))
            (epoch (list-ref strings 0))
            (usage (list-ref strings 1))
            (outofgas (list-ref strings 2)))
       (letrec
           ((loop
             (λ (ls pair-acc usage-acc dead-acc)
                (cond ((null? ls) (reverse pair-acc))
                      ((string-contains (car ls) epoch)
                       (loop (cdr ls) (cons (list (string->number (list->string (get-epoch-from-line (car ls))))
                                                  ;; per-car gas usage (map string->number (map list->string usage-acc))
                                                  (sum (map string->number (map list->string usage-acc)))
                                                  (average (map string->number (map list->string usage-acc)))
                                                  (length usage-acc)
                                                  dead-acc) pair-acc) null 0))
                      ((string-contains (car ls) usage)
                       (loop (cdr ls) pair-acc (cons (get-usage-from-line (car ls)) usage-acc) dead-acc))
                      ((string-contains (car ls) outofgas)
                       (loop (cdr ls) pair-acc usage-acc (add1 dead-acc)))
                      (else (loop (cdr ls) pair-acc usage-acc dead-acc))))))
         (loop rels null null 0)))))

(define file->lists
  (λ (filename)
     (map string->list (file->lines filename))))

(define collector
  (λ (filename)
     (let ((lists (file->lists filename)))
       (collect-on-lines lists))))

(define collector-print
  (λ (filename)
     (let* ((collections (collector filename))
            (printer (map (λ (x) (apply (curry printf "~s ~s ~s ~s ~s~n") x))
                          collections)))
       null)))




;; TESTS

(define test-string-starts-with
  (λ ()
     (let* ((raws '("abc" "abc" "c" "a"))
            (strings (map string->list raws))
            (a (list-ref strings 0))
            (b (list-ref strings 1))
            (c (list-ref strings 2))
            (d (list-ref strings 3)))
       (and (string-starts-with a b)
            (not(string-starts-with a c))
            (string-starts-with a d)))))

(define test-string-contains
  (λ ()
     (let* ((raws '("abc" "abc" "c" "b" "f" "abcd"))
            (strings (map string->list raws))
            (a (list-ref strings 0))
            (b (list-ref strings 1))
            (c (list-ref strings 2))
            (d (list-ref strings 3))
            (e (list-ref strings 4))
            (f (list-ref strings 5)))
       (and (string-contains a b)
            (string-contains a c)
            (string-contains a d)
            (not (string-contains a e))
            (not (string-contains a f))))))

(define run-tests
  (λ ()
     (and (test-string-starts-with)
          (test-string-contains))))
