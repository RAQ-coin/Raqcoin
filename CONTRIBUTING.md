Contributing to Raqcoin Core
============================

The Raqcoin Core project operates an open contributor model where anyone is
welcome to contribute towards development in the form of peer review, testing
and patches. This document explains the practical process and guidelines for
contributing.

Getting Started
---------------

New contributors are very welcome and needed.

Reviewing and testing is highly valued and the most effective way you can contribute
as a new contributor. It also will teach you much more about the code and
process than opening pull requests.

Before you start contributing, familiarize yourself with the Raqcoin Core build
system and tests. Refer to the documentation in the repository on how to build
Raqcoin Core and how to run the unit tests.

There are many open issues of varying difficulty waiting to be fixed.
If you're looking for somewhere to start contributing, check out the
[good first issue](https://github.com/RAQ-coin/Raqcoin/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22)
list or changes that are
[up for grabs](https://github.com/RAQ-coin/Raqcoin/issues?utf8=%E2%9C%93&q=label%3A%22Up+for+grabs%22).
Some of them might no longer be applicable. So if you are interested, but
unsure, you might want to leave a comment on the issue first.

### Good First Issue Label

The purpose of the `good first issue` label is to highlight which issues are
suitable for a new contributor without a deep understanding of the codebase.

However, good first issues can be solved by anyone. If they remain unsolved
for a longer time, a frequent contributor might address them.

You do not need to request permission to start working on an issue. However,
you are encouraged to leave a comment if you are planning to work on it. This
will help other contributors monitor which issues are actively being addressed
and is also an effective way to request assistance if and when you need it.

Communication Channels
----------------------

Most communication about Raqcoin Core development happens on GitHub Issues and Pull Requests.

Contributor Workflow
--------------------

The codebase is maintained using the "contributor workflow" where everyone
without exception contributes patch proposals using "pull requests" (PRs). This
facilitates social contribution, easy testing and peer review.

To contribute a patch, the workflow is as follows:

  1. Fork repository
  1. Create topic branch
  1. Commit patches
  1. Push changes to your fork
  1. Create pull request

The project coding conventions in [doc/coding.md](doc/coding.md) must be followed.

### Committing Patches

In general, commits should be atomic and diffs should be easy to read. For this reason, do not mix any formatting
fixes or code moves with actual code changes.

Make sure each individual commit is hygienic: that it builds successfully on its
own without warnings, errors, regressions, or test failures.
This means tests must be updated in the same commit that changes the behavior.

Commit messages should be verbose by default consisting of a short subject line
(50 chars max), a blank line and detailed explanatory text as separate
paragraph(s), unless the title alone is self-explanatory.

### Creating the Pull Request

The title of the pull request should be prefixed by the component or area that
the pull request affects. Valid areas as:

  - `consensus` for changes to consensus critical code
  - `doc` for changes to the documentation
  - `qt` or `gui` for changes to the GUI
  - `mining` for changes to the mining code
  - `net` or `p2p` for changes to the peer-to-peer network code
  - `refactor` for structural changes that do not change behavior
  - `test` or `ci` for changes to the unit tests, QA tests or CI code
  - `util` or `lib` for changes to the utils or libraries
  - `wallet` for changes to the wallet code
  - `build` for changes to the build system

The body of the pull request should contain sufficient description of *what* the
patch does, and even more importantly, *why*, with justification and reasoning.

### Work in Progress Changes and Requests for Comments

If a pull request is not to be considered for merging (yet), please
prefix the title with [WIP] or use Task Lists
in the body of the pull request to indicate tasks are pending.

### Address Feedback

At this stage, one should expect comments and review from other contributors. You
can add more commits to your pull request by committing them locally and pushing
to your fork.

You are expected to reply to any review comments before your pull request is
merged. You may update the code or reject the feedback if you do not agree with
it, but you should express so in a reply. If there is outstanding feedback and
you are not actively working on it, your pull request may be closed.

Pull Request Philosophy
-----------------------

Patchsets should always be focused. For example, a pull request could add a
feature, fix a bug, or refactor code; but not a mixture. Please also avoid super
pull requests which attempt to do too much, are overly large, or overly complex
as this makes review difficult.

### Features

When adding a new feature, thought must be given to the long term technical debt
and maintenance that feature may require after inclusion. Before proposing a new
feature that will require maintenance, please consider if you are willing to
maintain it (including bug fixing).

### Refactoring

Refactoring is a necessary part of any software project's evolution.
Refactoring PRs must not change the behaviour of code within the pull request (bugs must be preserved as is).

"Decision Making" Process
-------------------------

Whether a pull request is merged into Raqcoin Core rests with the project maintainers.

Maintainers will take into consideration if a patch is in line with the general
principles of the project; meets the minimum standards for inclusion; and will
judge the general consensus of contributors.

In general, all pull requests must:

  - Have a clear use case, fix a demonstrable bug or serve the greater good of
    the project;
  - Be reviewed;
  - Have unit tests, where appropriate;
  - Follow code style guidelines ([doc/coding.md](doc/coding.md));
  - Not break the existing test suite;

Patches that change Raqcoin consensus rules are considerably more involved than
normal because they affect the entire ecosystem.

Copyright
---------

Raqcoin Core is released under the terms of the GNU GPL v. 3 license.
See [COPYING](COPYING) for more information or https://www.gnu.org/licenses/gpl-3.0.en.html.
