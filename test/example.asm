%include "lib/languagedefinition.inc" 
push_value_from variable_a
push_value_from variable_b
add 
debug
exit

variable_a:
dq 1
variable_b:
dq 10
