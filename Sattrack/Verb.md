# Verb
A verb is entered first in the sentence, followed by a noun. Together both form a command which can be executed by the computer. Using unassigned verbs to not have any effect. Verbs are also being used to display and manage the values stored in the computer memory. 

## Reserved
```
00                  not in use
01                  display NOUN
02                  display NOUN, NOUN+1
03                  display NOUN, NOUN+2, NOUN+3
.
.
11                  monitor NOUN
12                  monitor NOUN, NOUN+1
13                  monitor NOUN, NOUN+1, NOUN+2
.
.
21                  load NOUN
22                  load NOUN, NOUN+1
23                  load NOUN, NOUN+1, NOUN+2
.
.
32                  recycle program
34                  terminate program
36                  restart system
37                  execute program
```