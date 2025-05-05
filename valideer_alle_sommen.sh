cat SINT_KLAAS_alle_sommen_met_flip.txt | awk -F'[+=]' 'function base92dec(s,    n, i, digit) {
    n = 0
    for (i = 1; i <= length(s); i++) {
        digit = substr(s, i, 1)
        if (digit < 0 || digit > 8) {
            print "Invalid base 9 digit:", digit > "/dev/stderr"
            return -1
        }
        n = n * 9 + digit
    }
    return n
} {print (base92dec($1) + base92dec($2)) ==  base92dec($3)}' | grep -v 1
