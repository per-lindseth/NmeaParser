This project is en experiment on how to pars NMEA sentencs.

An NMEA sentences is a sequence of bytes representing ASCII characters.

<ul>
  <li>Legal characters are between in decimal: {10(LF) , 13(CR) , 32 ... 126, 127 (DEL)}.</li>
  <li>Reserved characters are between in decimal: {10(LF), 13(CR), '$', '*', ',', '!', '\', '^', '~', 127(DEL)}.</li>
  <li>Valid characters are Legal characters minus Reserved characters.</li>
</ul>

The focus here is to parse the fields.
The fields are separated by ',' and the last field is separated by '*' from the checksum. 
It can contain any legal and the reserved characters '\^'.
The reserved character '\^' (HEX 5E) is followed by two ASCII characters (0-9, A-F) representing
the HEX value of the character to be communicated.

It can be assumed that the sentence has been validated and the checksum is correct.

The **alpabeth** has the following properties:
<ul>
  <li>Consists of legal characters and the reserved character '^'</li>
 </ul>

The **input/output string** has the following properties:
<ul>
  <li>It is a possibly empty string of characters from the alpabeth.</li>
</ul>

The following describes how to decode input string to simple data types (integer, float and string)
and how to encode simple data types to valid strings. 

A **field descriptor** has the following properties:
<ul>
  <li>name: a unique name different from any other field descriptors.</li>
  <li>format specifier: a regular expression specifying a valid string.</li>
  <li>validator: a predicate function taking an input string and returning a 
                 boolean indicating whether the string is in the set of strings 
                 defined by the regular expression.</li>
  <li>description: A description of the field descriptor.</li>	
 </ul>

There shall be no more than one field descriptor for each format specifier. 
This does not mean that a particular string can not be valid for more than one field descriptor,
but it means that there is no more than one field descriptor with the same format specifier.

A **typed field** the following properties:
<ul>
  <li>name: a unique name different from any other typed field.</li>
  <li>field descriptor: the name of the field descriptor.</li>
  <li>data type: integer, float or string.</li>
  <li>decoder: a function taking an input string and returning the decoded value as 
               an optional of the datatype or an empty optional if the validator
               in the field descriptor faild to validate the input string.</li>  
  <li>encoder: a function taking the value of data type and returning the encoded 
               value as a valid string. 
               It assert the the given value is valid and can be encoded.</li>
  <li>description: A description of the field descriptor.</li>	
 </ul>

There shall be no more than one typed field for each combination of field descriptor and data type.

The following describes how convert simple datatypes to representation types like a bool, enum 
or a struct/class and how to convert representation types to simple datatypes.

We can define a **Field** having the following properties:
<ul>
  <li>name: a unique name different from any other fields.</li>
  <li>field type: a name referring to a field type.</li>
  <li>type: </li>
  <li>decoder: a function taking a sentence string and returning the decoded value as an optional of a record type having the fields as its properties.</li>
  <li>encoder: a function taking the value of its record type and returning the encoded value as an optional valid sentence string.</li>
  <li>description: A description of the sentence type.</li>
</ul>