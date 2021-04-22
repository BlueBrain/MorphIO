# Contribution Guide

We would love for you to contribute to the MorphIO project and help make it better than it is today.
As a contributor, here are the guidelines we would like you to follow:

 - [Question or Problem?](#got-a-question)
 - [Issues and Bugs](#found-a-bug)
 - [Feature Requests](#missing-a-feature)
 - [Submissions](#submission-guidelines)
 - [Development Guidelines](#development)
 - [Release Procedure](#release)

# Got a Question?

Please do not hesitate to raise an issue on [github project page][github].

# Found a Bug?

If you find a bug in the source code, you can help us by [submitting an issue](#issues)
to our [GitHub Repository][github]. Even better, you can [submit a Pull Request](#pull-requests) with a fix.

#  Missing a Feature?

You can *request* a new feature by [submitting an issue](#issues) to our GitHub Repository.
If you would like to *implement* a new feature, please submit an issue with a proposal for your 
work first, to be sure that we can use it.

Please consider what kind of change it is:

* For a **Major Feature**, first open an issue and outline your proposal so that it can be
discussed. This will also allow us to better coordinate our efforts, prevent duplication of work,
and help you to craft the change so that it is successfully accepted into the project.
* **Small Features** can be crafted and directly [submitted as a Pull Request](#pull-requests).

# Submission Guidelines

## Issues

Before you submit an issue, please search the issue tracker, maybe an issue for your problem
already exists and the discussion might inform you of workarounds readily available.

We want to fix all the issues as soon as possible, but before fixing a bug we need to reproduce
and confirm it. In order to reproduce bugs we will need as much information as possible, and
preferably with an example.

## Pull Requests

When you wish to contribute to the code base, please consider the following guidelines:

* Make a [fork](https://guides.github.com/activities/forking/) of this repository.
* Make your changes in your fork, in a new git branch:

     ```shell
     git checkout -b my-fix-branch master
     ```
* Create your patch, **including appropriate Python test cases**.
  Please check the coding [conventions](#coding-conventions) for more information.
* Run the full test suite, and ensure that all tests pass.
* Commit your changes using a descriptive commit message.

     ```shell
     git commit -a
     ```
* Push your branch to GitHub:

    ```shell
    git push origin my-fix-branch
    ```
* In GitHub, send a Pull Request to the `master` branch of the upstream repository of the relevant component.
* If we suggest changes then:
  * Make the required updates.
  * Re-run the test suites to ensure tests are still passing.
  * Rebase your branch and force push to your GitHub repository (this will update your Pull Request):

       ```shell
        git rebase master -i
        git push -f
       ```

That’s it! Thank you for your contribution!

### After your pull request is merged

After your pull request is merged, you can safely delete your branch and pull the changes from
the main (upstream) repository:

* Delete the remote branch on GitHub either through the GitHub web UI or your local shell as follows:

    ```shell
    git push origin --delete my-fix-branch
    ```
* Check out the master branch:

    ```shell
    git checkout master -f
    ```
* Delete the local branch:

    ```shell
    git branch -D my-fix-branch
    ```
* Update your master with the latest upstream version:

    ```shell
    git pull --ff upstream master
    ```

[github]: https://github.com/BlueBrain/MorphIO

# Development Environment

Please make sure to install the project requirements,
see the [dependencies](./README.md#dependencies) section in top README.

This section applies to both Python versions 2 and 3.

## Setup

It is recommended to use `virtualenv` to develop in a sandbox environment:

```
virtualenv venv
. venv/bin/activate
pip install -r tests/requirement_tests.txt
```

## Build

Run the following command to build incrementally the project: `pip install -e .`

## Test

Run the following command to run the Python unit-tests: `pytest tests`

## Coding conventions

The code coverage of the Python unit-tests may not decrease over time.
It means that every change must go with their corresponding Python unit-tests to
validate the library behavior as well as to demonstrate the API usage. 

