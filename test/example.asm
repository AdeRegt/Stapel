%include "lib/languagedefinition.inc" 
push_value_with 4
push_value_with 1
push_value_addr variable_a 
push_value_with 12
push_value_with 0
push_value_with 0
int
exit

variable_a:
db "Hello world!"
