## Building from source

Since you are reading this, you are likely in the root directory of the project.

`make` (or `make -j 4` or whatever)

All intermediary objects get put into `./obj`

The binary executable is right next to you `./xdupes`

## Other `make` targets

Real quick: `make` == `make all`, `make all` == `make default`, and `make default` builds the executable.

`make clean`
    Removes `./obj` (all files that were created by `make` except for the executable)
    Also removes the executable. lol

`make strict`
    Uses a bunch of flags that are more strict than what `make default` uses

`make dev`
    Uses a lower optimization level, includes debugging symbols, and is ALLEGEDLY faster.

`make fresh`
    Does `make clean` and then `make`. Might appear pointless, but this lets you do:
    `make fresh -j 4`
    without any weirdness.

`make install`
    Intended for me. I commented it out, as well. Strips the executable and installs it to `/usr/local/bin/`.


## Using the program

Worry not, as `xdupes` does not (yet) do any auto-removal of any files. It only lists duplicate files, in groups, separated by newlines. By default, it does not search recursively. Also by default, it only uses 1 thread. Sort of.

```
one
two

three
four
five

```

In the hypothetical output above, `two` is a duplicate of `one`, and `four` and `five` are duplicates of `three`.

It's worth running the program on smaller directories first, to not overwhelm yourself with the output.

Usage:

    `./xdupes [OPTION...] SOURCE [SOURCE...]`

    Positional Arguments
        SOURCE
            Path(s) to 1 or more directories to search for duplicate files in.

    Optional Arguments
        `--threads N`/`-t N`
            How many threads to use for hashing

    Boolean Arguments
        `--recursive`/`-r`
            Walk all subdirectories found in SOURCE(S)
        `--timed`
            Show how long the entire operation took
        `--wasted`/`--wasted-space`
            Show how much space is being used by duplicate files (not including the original/unique one)
        `--noempty`/`--skip-empty`
            Skip empty files (otherwise all empty files will be listed, as they all hash to the same thing)
        `--zero`
            Use `\0` instead of `\n` as the line separator
        `--si`
            Use KiB, MiB, etc. instead of KB, MB, etc.

    Boolean Arguments (cont.) - Experimental
        `--progress`
            Shows a progress bar (WIP still, only shows the progress bar during file traversal, not during hashing)

    Boolean Arguments (cont.) - Options that affect how the output is written, and also the debug options.
        `--quiet`/`-q`
            Produce less output.
        `--verbose'/'-v'
            Produce more output.
        `--debug`
            Show some more information regarding what happened (currently just includes thread count and more detailed timing info).
