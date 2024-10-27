## Building from source

I've recently switched from Make to CMake. Here are the new build instructions:

```bash
cmake --workflow --preset release
# There is currently no install command. But the executable should be right in ./build/
# Try it out on the current working directory. If there is no output, there are no duplicates.
./build/xdupes --recursive ./
# If you're bold, go ahead and immediately run it on your Pictures directory. Files are only ever read, so don't worry
# about files being removed or anything. You should still be paranoid and try some test directories first.
```


## Using the program

Worry not, as `xdupes` does not (yet) do any auto-removal of any files. It only lists duplicate files, in groups, 
separated by newlines. By default, it does not search recursively. Also by default, it only uses 1 thread. Sort of. It
uses 2 threads; a producer and a consumer. One feeds filenames, one reads them and hashes them. This was the optimal
strategy.

Here is some example output:

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
        `--si`/`--binary`
            Use KiB, MiB, etc. instead of KB, MB, etc.
        `--progress`
            (NEW) This option enables progress reporting during the filesystem traversal (as a count, no bar), during
            the queueing of the files into the job pool (as an actual progress bar), and during the hashing of the files
            (also as an actual progress bar). Since files are already being hashed by all the jobs are queued, the
            progress bar that shows the hashing progress will almost always start above 0%. For small amounts of small
            files, the progress bar may not be visible for long, as the work is done very quickly.

    Boolean Arguments (cont.) - Options that affect how the output is written, and also the debug options.
        `--quiet`/`-q`
            Produce less output.
        `--quiet`/`-q`
            Produce no output.
        `--debug`
            Show some more information regarding what happened (currently just includes thread count and more detailed
            timing info) (and now the file count and hashed file count).
