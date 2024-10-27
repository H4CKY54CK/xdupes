# Changelog

All notable changes to this project will be documented in this file (if I can remember to do so).

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project strives to (when it remembers) adhere to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

- Start of changelog. Things I can remember to log right now? Currently, we are broken and I'm bugfixing.

### Added

- We have finally acquired a decent-looking `--help` menu. Which leads me into the first major addition...
- `xdupes` now depends on my custom, header-only argument parsing library. It generates very nice help menus, has informative parser errors, and was absolutely far more work to get into a "standalone" state than it would have been to just fix the problems with the hand-rolled parsing system we were using before. With that being said, I've sank a lot of time and effort into ensuring the correctness of the standalone parsing system. It's overkill for the purposes of this CLI tool, but I'm also sick of mucking around with it. This is one of the few times I am willing to depend on something that does more than what is needed. But it also just makes more sense this way, since now the parser will be looked after more carefully.
- Another new option, `--progress`, has made its way into the program! It slows down performance a bit, but this should be a welcome change to what is otherwise **nothing being displayed or updated** while work is being done.
- The new parser comes with many other beneficial changes, which will either get fully documented once and for all, or mentioned at least somewhere.
- A better configuration for CMake, so that we can easily and quickly build dev, debug, and release builds.
- This changelog.

### Fixed

- Duplicate directories, and subdirectories of other directories provided in the same argument list, now get deduplicated as needed. Work is no longer being done multiple times, and results are now reliable no matter how many times you provide the same directory to the program in the same invocation.

### Changed

- The way the work gets queued and subsequently completed has changed from "setting up the work and then doing it" to "starting a thread so that work can started as soon as work is available, and also so that we can continue to queue more work while work is already being done". AKA the custom ThreadPool I implemented is now a producer-consumer kind of deal. I'm not sure if I changed it before this changelog entry or not, but it's a change nonetheless.
- Previously (it might not have been published/released, but meh), "work" that was being submitted to the ThreadPool was implemented such that a function was being submitted as the "job", which would then get executed within the ThreadPool. Now, we only submit strings to the queue, and the hashing function is implemented directly into the ThreadPool logic. Whether it was this or the 15 other things I changed at the same time that caused the massive performance improvement, the world may never know. But it's faster.

### Removed

- My attention span. But it's getting slowly merged back in.
- Oh, and the TODO might get removed. I'm unsure yet.
