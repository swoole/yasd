<?php

// Suppose the business code calls a function to determine whether to end the loop, 
// but the business code always returns false
function test() {
    return false;
}

$a = 0;

LABEL:

$a++;
if (test() || $a > 100000) {
    goto END;
}

goto LABEL;

END:
?>