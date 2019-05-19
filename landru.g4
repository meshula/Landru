
// MIT 3 clause, copyright 2019 Nick Porcino

grammar landru;

landruProgram :
    (l_requireLine | l_declaration | l_machine)+
    ;

l_requireLine :
    l_identifier '=' 'require' string_literal
    ;

l_machine:
    'machine' l_identifier
    '{'
    (l_declaration|l_state)*
    '}';

l_state:
    'state' l_identifier LBRAC l_statement* RBRAC
    ;

l_statement
    : ('on' l_functionDesignator LBRAC l_statement* RBRAC)
    | ('goto' l_identifier)
    | l_declaration
    | ((l_dottedIdentifier EQUAL)? l_functionDesignator)
;

l_declaration
    : 'declare' LBRAC ('shared'? l_typeIdentifier l_dottedIdentifier (EQUAL literal)?)* RBRAC
    ;

l_functionDesignator
    : l_dottedIdentifier LPAREN l_parameterList? RPAREN
    ;

l_parameterList
    : l_expression (',' l_expression)*
    ;

l_expression
    : logical_term ('||' logical_term)*
    ;

logical_term
    : logical_factor ('&&' logical_factor)*
    ;

logical_factor
    : ('!')? relation
    ;

relation
    : arithmetic_expression (rel_op arithmetic_expression)?
    ;

rel_op
    : '<='
    | '<'
    | '>='
    | '>'
    | '=='
    | '!='
    ;

arithmetic_expression
    : addend (add_op addend)*
    ;

add_op : '+' ;

addend
    : subend (sup_op subend)*
    ;

sub_op : '-' ;

subend
    : factor (mul_op factor)*
    ;

mul_op
    : '*'
    | '/'
    ;

factor :
    l_functionDesignator | l_dottedIdentifier | literal
    ;

l_typeIdentifier
    : l_dottedIdentifier
    | ( 'int' | 'float' | 'string')
    ;

l_dottedIdentifier: l_identifier ('.' l_identifier)*;
l_identifier: ALPHA (ALPHANUMERIC | '_')* ;

literal:
    float_number | int_number | string_literal
    ;

string_literal
    : '"' ( S_CHAR | S_ESCAPE )* '"'
    ;

S_CHAR
    : ~ ["\\]
    ;

S_ESCAPE
    : '\\' ('’' | '\'' | '"' | '?' | '\\' | 'a' | 'b' | 'f' | 'n' | 'r' | 't' | 'v')
    ;

EQUAL : '=' ;

ALPHA : [A-Za-z] ;
DIGIT : [0-9] ;
ALPHANUMERIC : [A-Za-z0-9]+ ;

exponent
    : ('e') ('+' | '-')? DIGIT+
    ;

float_number
    : ('+' | '-')?DIGIT+ (('.' DIGIT+ (exponent)?)? | exponent)
    ;

int_number
    :
    (('+' | '-')?DIGIT+)
    ;

LBRAC : '{' ;
RBRAC : '}' ;
LPAREN : '(' ;
RPAREN : ')' ;

Whitespace
    : [ \t]+ -> skip
    ;
Newline
    : ('\r' '\n'? | '\n') -> skip
    ;
BlockComment
    : '/*' .*? '*/' -> skip
    ;
LineComment
    : '//' ~ [\r\n]* -> skip
    ;
