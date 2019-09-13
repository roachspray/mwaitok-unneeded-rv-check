
llvm pass to look for the following pattern:

```
foo = malloc(..., M_WAITOK)
if (foo == NULL) {
 ...
}
```

for malloc, mallocarray, realloc, and reallocf FreeBSD kernel functions.
M_WAITOK implies the non-return of NULL and makes that code path un-needed.

In llvm, it's looking roughly (after mem2reg):

```
%a = malloc(...., %v) # where %v is a ConstantInt
%b = bitcast .. %a ..
%c = cmp %b, null
```

meant for LLVM 6.0 and needs gmake
