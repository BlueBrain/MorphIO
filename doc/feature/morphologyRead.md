Modification of the brion::Morphology read API {#morphologyRead}
============

## Motivation

The current read API is incorrect. It does return a shared_ptr to mutable data,
but is marked const. For example, the brain::Morphology takes a const
brion::Morphology&, but the given object is modified in case of SWC with global
transformation.

## API variants:

We chose option 5 since it clarifies and unifies the behaviour of all plugins
with a clean API:

1. ConstVectorPtr readFoo() const
2. Vector readFoo() const
3. VectorPtr readFoo()
4. std::future<Vector> readFoo() const
5. Vector& getFoo() and const Vector& getFoo() const

Some implementations do read multiple vectors at the same time (SWC, ZeroEQ).

Option 1 is rejected: Client code always needs to make a copy if it wants to
modify the data.

Option 2 either complicates the implementation of SWC/ZeroEQ which need to
reread data if a given read function is called multiple times, or would
constrain the API to throw if a read function is called multiple times.

Option 3 is awkward since it is a mutable read API, and delays the problem to
the user of the API.

Option 4 adds the future complexity (with shared state between readFoo/readBar)
on top of Option 2.

Option 5 parses always the data in the ctor using a future and thread pool
synchronized in all getFoo() calls. This forces that the brion::Morphology has
state and data, and we should remove the write API at the same time. It has the
disadvantage of loading all data deferred from the ctor, which requires out of
core loading (or batching) for large amounts of morphologies. Client code using
the brain API already has this constraint. While this seems to be more complex,
actual plugin implementations become simpler since all of the threading and
synchronization will be done in the plugin base class.
