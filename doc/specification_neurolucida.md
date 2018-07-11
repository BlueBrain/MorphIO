# Soma
- Files without a soma are valid.
- Multiple soma
ASC files with *multiple* CellBody tags will raise an error.
[Unit test](https://github.com/BlueBrain/MorphIO/blob/5e111f3141f7a1ee72e0260111ce569741d80acb/tests/test_neurolucida.py#L58)

# Ignored keywords
All S-expressions starting with a keyword other than `CellBody, Axon, Dendrite, Apical` will be skipped.
# Duplicate points
When reading an ASC file, the last point of a section will be added as the first point of the
child sections if not already present. That means these two representations are equivalent:

```lisp
( (Dendrite)
  (3 -4 0 2)
  (3 -10 0 2)
  (
    (0 -10 0 2)
    (-3 -10 0 2)
  |
    (6 -10 0 2)
    (9 -10 0 2)
  )
)
```

```lisp
( (Dendrite)
  (3 -4 0 2)
  (3 -10 0 2)
  (
    (3 -10 0 2) ; <- duplicate
    (0 -10 0 2)
    (-3 -10 0 2)
  |
    (3 -10 0 2) ; <- duplicate
    (6 -10 0 2)
    (9 -10 0 2)
  )
)
```

[Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_neurolucida.py#L162), [Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_writers.py#L191)

Note: As of today, it is **OK** for a duplicated point to have a different radius than the original point.


When writing the file the duplicate point is **not** automatically added. However, a warning will be displayed if the first point of a section differs from the last point of the previous section.

[Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_mut.py#L125)

# Single point section
A section made of only one point will be treated differently depending on whether the point is a duplicate of the parent section last point or not.
- If it is a duplicate, the section is discarded.
Example:

```lisp
((Dendrite)
  (3 -4 0 2)
  (3 -10 0 2)
  (
    (3 -10 0 2)  ; duplicate point
  )
)
```
will become:
```lisp
((Dendrite)
  (3 -4 0 2)
  (3 -10 0 2)
)
```

See [here](https://github.com/BlueBrain/MorphIO/blob/5e111f3141f7a1ee72e0260111ce569741d80acb/tests/test_neurolucida.py#L350) and [here for a more complex case](https://github.com/BlueBrain/MorphIO/blob/5e111f3141f7a1ee72e0260111ce569741d80acb/tests/test_neurolucida.py#L374)

- If the point is _not_ a duplicate, then the duplicate point is prepended at the beggining of the section and the section is considered perfectly valid. See [here](https://github.com/BlueBrain/MorphIO/blob/5e111f3141f7a1ee72e0260111ce569741d80acb/tests/test_neurolucida.py#L253)
```lisp
((Dendrite)
  (3 -4 0 2)
  (3 -10 0 2)
  (
    (3 -100 100 4)  ; not a duplicate point
  )
)
```
will be equivalent to:
```lisp
((Dendrite)
  (3 -4 0 2)
  (3 -10 0 2)
  (
    (3 -10 0 2)    ; added duplicate
    (3 -100 100 4)
  )
)
```

# Single child section
Section with only one child section will have their child merged with.
```lisp
'''
((Dendrite)
 (3 -4 0 2)
 (3 -6 0 2)
 (3 -8 0 2)
 (3 -10 0 2)
 (
   (3 -10 0 2)  ; merged with parent section
   (0 -10 0 2)  ; merged with parent section
   (-3 -15 0 2) ; merged with parent section
   (
     (-5 -5 5 5)
     |
     (-6 -6 6 6)
   )
 )
)
```

will be interpreted the same as:

```lisp
((Dendrite)
 (3 -4 0 2)
 (3 -6 0 2)
 (3 -8 0 2)
 (3 -10 0 2)
 (0 -10 0 2)
 (-3 -15 0 2)
 (
   (-5 -5 5 5)
   |
   (-6 -6 6 6)
 )
)
```
[Unit test](https://github.com/BlueBrain/MorphIO/blob/5e111f3141f7a1ee72e0260111ce569741d80acb/tests/test_neurolucida.py#L285)

## Empty siblings
File with empty siblings are handled correctly:
```lisp
((Dendrite)
 (3 -4 0 2)
 (3 -6 0 2)
 (3 -8 0 2)
 (3 -10 0 2)
 (
   (3 -10 0 2)
   (0 -10 0 2)
   (-3 -10 0 2)
   |       ; <-- empty sibling but still works
  )
 )
```

will be interpreted the same as:
```lisp
((Dendrite)
 (3 -4 0 2)
 (3 -6 0 2)
 (3 -8 0 2)
 (3 -10 0 2)
 (0 -10 0 2)
 )
```
