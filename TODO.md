# Current Issues

## STOP PUSHING RANDOM UPDATES

- Start using semver, dude. Seriously.


## Argument

- ~~`Argument::action(std::string)` must do extra work to ensure a valid action is being used. Switch to an enum. Then the work can be eliminated.~~ Fixed

    - In a similar vain, it also current overwrites user-provided default and const values that are empty strings. If the user explicitly provided it, don't overwrite it. A simple member `user_provided` should suffice.

- All arguments are parsed as strings. That is just how it is. That doesn't mean we can't *also* check at parse-time whether it *looks* like the correct type. Provide a member function `Argument::as_type` to specify the desire type, so that we can push all of our type-checking (more like type-sort-of-checking but whatever) into the parser, instead of during the on-demand type retrieval system we currently have, using `Result`

- After some further thought, there is another problem pertaining to both of the above. When a user sets the default/const value via `Argument::default_value` and `Argument::const_value`, the value is set without any checks to the action. Is this okay? Python's `argparse` allows this, but I'm not actually sure if this is even *possible* to sanely handle. Maybe check `Argument::as_type` for that? Similarly, when the action is set using `Argument::action("store_true")`, there are again no checks in place for the default and const values for that argument except for a "is it empty?" check.


## ArgumentParser

- Accidentally misspelling a key when accessing the map returned by `ArgumentParser::parse_args(int, char**)` is currently a valid operation. This leads to stupid bugs that are hard to decode that should never have even been a possibility in the first place. Using a key that doesn't exist in the map is not the problem. Using a key that is an invalid argument is the problem. Possibly use a wrapper around `std::map` to throw errors when using a key that was not a possible argument in that first place.

    - The function signature is already a hot enough mess. Provide a typedef.

- `ArgumentParser::parse_args(int, char**)` currently contains ALL of the parsing logic. If we were to then implement `ArgumentParser::parse_unknown_args`, we wouldn't be able to use `ArgumentParser::parse_args(int, char**)` due to how it errors on unknown arguments (something that `ArgumentParser::parse_unknown_args` allows). Instead, the **least** restrictive parser should contain all the parsing logic, and then the **more** restrictive parsers would be able to utilize them without writing much extra code.


## ThreadPool

- ~~With the new parsing system finally in place, some changes were made regarding when values parsed by the parser are validated. My first and immediate concern is to make sure `ThreadPool` is getting the correct argument passed to it. It must be positive (this is actually checked during parsing, completely coincidentally)... And that's it. Passing 0 to it means "auto", and passing any other integer uses that integer.~~ I believe this is covered.


## Miscellaneous

- ~~There is a subtle error being triggered if `#include "parsing.hpp"` is *before* `#include "logging.hpp"`. For some reason, it is not possible to **not** `#include "logging.hpp"`. Investigate and fix.~~ Pretty sure this is fixed, or will be with the next commit.


## ProgressBar

- ~~If the program is cancelled or interrupted after a progress bar has been displayed at least once but before it has had a chance to reset the cursor and text color, then there is nothing that will reset the terminal to a known good state. This needs to be addressed by catching a few signals; possibly: INT EXIT QUIT TERM~~ Fixed, I think.
