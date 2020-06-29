---
title: Input
weight: 20
pre: ""
chapter: false
---

## getInput ##

**Syntax**  

    emu.getInput(port)

**Parameters**  
port - *Integer* The port number to read (0 to 4)

**Return value**  
*Table* A table containing the status of all 12 buttons. 

**Description**  
Returns a table containing the status of all 12 buttons: { a, b, x, y, l, r, select, start, up, down, left, right }   

## getMouseState ##

**Syntax**  

    emu.getMouseState()

**Return value**  
*Table* The mouse's state 

**Description**  
Returns a table containing the position and the state of all 3 buttons: { x, y, left, middle, right }   

## isKeyPressed ##

**Syntax**  

    emu.isKeyPressed(keyName)

**Parameters**  
keyName - *String* The name of the key to check

**Return value**  
*Boolean* The key's state (true = pressed)

**Description**  
Returns whether or not a specific key is pressed. The "keyName" must be the same as the string shown in the UI when the key is bound to a button.
