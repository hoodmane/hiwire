Requires either Emscripten >= 3.1.42 or clang more recent than June 10th 2023.
As of this writing, no stable clang has been released since June 10th, but
17.0.0-rc1 is a candidate.

I recommend compiling with `-Werror=int-conversion
-Werror=incompatible-pointer-types` to the compile flags, to ensure that no
implicit casts will happen between JsRef and any other type.
