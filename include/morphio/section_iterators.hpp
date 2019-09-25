#pragma once

#include <algorithm> // std::copy
#include <deque> // std::deque
#include <iterator> // std::back_inserter / std::front_inserter
#include <memory> // std::shared_ptr
#include <vector> // std::vector

#include <morphio/types.h>
#include <morphio/exceptions.h>

namespace detail {

template<typename SectionT, typename MorphologyT>
std::vector<SectionT> getChildren(const MorphologyT& morphology)
{
    return morphology.rootSections();
}

template<typename SectionT>
std::vector<SectionT> getChildren(const SectionT& section)
{
    return section.children();
}

template<typename SectionT>
std::vector<std::shared_ptr<SectionT>> getChildren(const std::shared_ptr<SectionT>& section)
{
    return section->children();
}

template<typename SectionT>
bool isRoot(const SectionT& current)
{
    return current.isRoot();
}

template<typename SectionT>
bool isRoot(const std::shared_ptr<SectionT>& current)
{
    return current->isRoot();
}

template<typename SectionT>
SectionT getParent(const SectionT& current)
{
    return current.parent();
}

template<typename SectionT>
std::shared_ptr<SectionT> getParent(const std::shared_ptr<SectionT>& current)
{
    return current->parent();
}
} // namespace detail

namespace morphio {

template<typename SectionT, typename MorphologyT>
class breadth_iterator_t {
public:
    breadth_iterator_t() = default;

    explicit breadth_iterator_t(const SectionT& section)
    {
        deque_.push_front(section);
    }

    explicit breadth_iterator_t(const MorphologyT& morphology)
    {
        const auto& children = detail::getChildren<SectionT, MorphologyT>(morphology);
        std::copy(children.begin(), children.end(), std::back_inserter(deque_));
    }

    breadth_iterator_t(const breadth_iterator_t& other)
    : deque_(other.deque_)
    {}

    SectionT operator*() const
    {
        return deque_.front();
    }

    breadth_iterator_t operator++()
    {
        if (deque_.empty()) {
            LBTHROW(MorphioError("Can't iterate past the end"));
        }

        const auto children = detail::getChildren(deque_.front());
        deque_.pop_front();
        std::copy(children.begin(), children.end(), std::back_inserter(deque_));

        return *this;
    }

    breadth_iterator_t operator++(int)
    {
        breadth_iterator_t ret(*this);
        ++(*this);
        return ret;
    }

    bool operator==(const breadth_iterator_t& other) const
    {
        return deque_ == other.deque_;
    }

    bool operator!=(const breadth_iterator_t& other) const
    {
        return !(*this == other);
    }

private:
    std::deque<SectionT> deque_;
};

template<typename SectionT, typename MorphologyT>
class depth_iterator_t {
public:
    depth_iterator_t() = default;

    explicit depth_iterator_t(const SectionT& section)
    {
        deque_.push_front(section);
    }

    explicit depth_iterator_t(const MorphologyT& morphology)
    {
        const auto& children = detail::getChildren<SectionT, MorphologyT>(morphology);
        std::copy(children.rbegin(), children.rend(), std::front_inserter(deque_));
    }

    depth_iterator_t(const depth_iterator_t& other)
    : deque_(other.deque_)
    {}

    SectionT operator*() const
    {
        return deque_.front();
    }

    depth_iterator_t operator++()
    {
        if (deque_.empty()) {
            LBTHROW(MorphioError("Can't iterate past the end"));
        }

        const auto children = detail::getChildren(deque_.front());
        deque_.pop_front();
        std::copy(children.rbegin(), children.rend(), std::front_inserter(deque_));

        return *this;
    }

    depth_iterator_t operator++(int)
    {
        depth_iterator_t ret(*this);
        ++(*this);
        return ret;
    }

    bool operator==(const depth_iterator_t& other) const
    {
        return deque_ == other.deque_;
    }

    bool operator!=(const depth_iterator_t& other) const
    {
        return !(*this == other);
    }

private:
    std::deque<SectionT> deque_;
};

template<typename SectionT>
class upstream_iterator_t {
public:
    upstream_iterator_t()
    : unused(0)
    , end(true)
    { }

    explicit upstream_iterator_t(const SectionT& section)
    : current(section)
    , end(false)
    { }

    upstream_iterator_t(const upstream_iterator_t& other)
    {
        end = other.end;
        if (!other.end) {
            new (&current) SectionT(other.current);
        }
    }

    ~upstream_iterator_t()
    {
        if (!end) {
            this->current.~SectionT();
        }
    }

    SectionT operator*() const
    {
        return current;
    }

    upstream_iterator_t operator++()
    {
        if (end) {
            LBTHROW(MissingParentError("Cannot call iterate upstream past the root node"));
        } else if (detail::isRoot(current)) {
            end = true;
            this->current.~SectionT();
        } else {
            current = detail::getParent(current);
        }
        return *this;
    }

    upstream_iterator_t operator++(int)
    {
        upstream_iterator_t ret(*this);
        ++(*this);
        return ret;
    }

    bool operator==(const upstream_iterator_t& other) const
    {
        if (end) {
            return end == other.end;
        }
        return current == other.current;
    }

    bool operator!=(const upstream_iterator_t& other) const
    {
        return !(*this == other);
    }

private:
    // This is a workaround for not having std::optional until c++17.
    // Need to have a copy of a section, not a pointer, as the latter are created
    // on the fly, so we'd have to heap allocate; however, to signal the end()
    // of iteration, we need a 'default' section or a sentinel.  Since the concept
    // of a default section is nebulous, a sentinel was used instead.
    union {
        char unused;
        SectionT current;
    };
    bool end;
};

} // namespace morphio
