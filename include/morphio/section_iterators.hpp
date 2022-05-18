#pragma once

#include <algorithm>  // std::copy
#include <deque>      // std::deque
#include <iterator>   // std::back_inserter / std::front_inserter
#include <memory>     // std::shared_ptr
#include <vector>     // std::vector

#include <morphio/exceptions.h>
#include <morphio/types.h>

namespace detail {

template <typename SectionT, typename MorphologyT>
std::vector<SectionT> getChildren(const MorphologyT& morphology) noexcept {
    return morphology.rootSections();
}

template <typename SectionT>
std::vector<SectionT> getChildren(const SectionT& section) {
    return section.children();
}

template <typename SectionT>
std::vector<std::shared_ptr<SectionT>> getChildren(const std::shared_ptr<SectionT>& section) {
    return section->children();
}

template <typename SectionT>
bool isRoot(const SectionT& current) {
    return current.isRoot();
}

template <typename SectionT>
bool isRoot(const std::shared_ptr<SectionT>& current) {
    return current->isRoot();
}

template <typename SectionT>
SectionT getParent(const SectionT& current) {
    return current.parent();
}

template <typename SectionT>
std::shared_ptr<SectionT> getParent(const std::shared_ptr<SectionT>& current) {
    return current->parent();
}
}  // namespace detail

namespace morphio {

template <typename SectionT, typename MorphologyT>
class breadth_iterator_t
{
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = SectionT;
    using difference_type = std::ptrdiff_t;
    using pointer = SectionT*;
    using reference = SectionT&;

    breadth_iterator_t() = default;

    inline explicit breadth_iterator_t(const SectionT& section);
    inline explicit breadth_iterator_t(const MorphologyT& morphology);
    inline breadth_iterator_t(const breadth_iterator_t& other);

    inline SectionT operator*() const;
    inline SectionT const* operator->() const;

    inline breadth_iterator_t& operator++();
    inline breadth_iterator_t operator++(int);

    inline bool operator==(const breadth_iterator_t& other) const;
    bool operator!=(const breadth_iterator_t& other) const;

  private:
    std::deque<SectionT> deque_;
};

template <typename SectionT, typename MorphologyT>
class depth_iterator_t
{
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = SectionT;
    using difference_type = std::ptrdiff_t;
    using pointer = SectionT*;
    using reference = SectionT&;

    depth_iterator_t() = default;

    inline explicit depth_iterator_t(const SectionT& section);
    inline explicit depth_iterator_t(const MorphologyT& morphology);
    inline depth_iterator_t(const depth_iterator_t& other);

    inline SectionT operator*() const;
    inline SectionT const* operator->() const;

    inline depth_iterator_t& operator++();
    inline depth_iterator_t operator++(int);

    inline bool operator==(const depth_iterator_t& other) const;
    inline bool operator!=(const depth_iterator_t& other) const;

  private:
    std::deque<SectionT> deque_;
};

template <typename SectionT>
class upstream_iterator_t
{
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = SectionT;
    using difference_type = std::ptrdiff_t;
    using pointer = SectionT*;
    using reference = SectionT&;

    inline upstream_iterator_t();
    inline explicit upstream_iterator_t(const SectionT& section);
    inline upstream_iterator_t(const upstream_iterator_t& other);
    inline ~upstream_iterator_t();

    inline SectionT operator*() const;
    inline SectionT const* operator->() const;

    inline upstream_iterator_t& operator++();
    inline upstream_iterator_t operator++(int);

    inline bool operator==(const upstream_iterator_t& other) const;
    inline bool operator!=(const upstream_iterator_t& other) const;

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
    bool end = false;
};

// breath_iterator_t class definition

template <typename SectionT, typename MorphologyT>
inline breadth_iterator_t<SectionT, MorphologyT>::breadth_iterator_t(const SectionT& section) {
    deque_.push_front(section);
}

template <typename SectionT, typename MorphologyT>
inline breadth_iterator_t<SectionT, MorphologyT>::breadth_iterator_t(
    const MorphologyT& morphology) {
    const auto& children = detail::getChildren<SectionT, MorphologyT>(morphology);
    std::copy(children.begin(), children.end(), std::back_inserter(deque_));
}

template <typename SectionT, typename MorphologyT>
inline breadth_iterator_t<SectionT, MorphologyT>::breadth_iterator_t(
    const breadth_iterator_t& other)
    : deque_(other.deque_) {}

template <typename SectionT, typename MorphologyT>
inline SectionT breadth_iterator_t<SectionT, MorphologyT>::operator*() const {
    return deque_.front();
}

template <typename SectionT, typename MorphologyT>
inline SectionT const* breadth_iterator_t<SectionT, MorphologyT>::operator->() const {
    return &deque_.front();
}

template <typename SectionT, typename MorphologyT>
inline breadth_iterator_t<SectionT, MorphologyT>&
breadth_iterator_t<SectionT, MorphologyT>::operator++() {
    if (deque_.empty()) {
        throw MorphioError("Can't iterate past the end");
    }

    const auto& children = detail::getChildren(deque_.front());
    deque_.pop_front();
    std::copy(children.begin(), children.end(), std::back_inserter(deque_));

    return *this;
}

template <typename SectionT, typename MorphologyT>
inline breadth_iterator_t<SectionT, MorphologyT>
breadth_iterator_t<SectionT, MorphologyT>::operator++(int) {
    breadth_iterator_t ret(*this);
    ++(*this);
    return ret;
}

template <typename SectionT, typename MorphologyT>
inline bool breadth_iterator_t<SectionT, MorphologyT>::operator==(
    const breadth_iterator_t& other) const {
    return deque_ == other.deque_;
}

template <typename SectionT, typename MorphologyT>
inline bool breadth_iterator_t<SectionT, MorphologyT>::operator!=(
    const breadth_iterator_t& other) const {
    return !(*this == other);
}

// depth_iterator_t class definition

template <typename SectionT, typename MorphologyT>
inline depth_iterator_t<SectionT, MorphologyT>::depth_iterator_t(const SectionT& section) {
    deque_.push_front(section);
}

template <typename SectionT, typename MorphologyT>
inline depth_iterator_t<SectionT, MorphologyT>::depth_iterator_t(const MorphologyT& morphology) {
    const auto& children = detail::getChildren<SectionT, MorphologyT>(morphology);
    std::copy(children.rbegin(), children.rend(), std::front_inserter(deque_));
}

template <typename SectionT, typename MorphologyT>
inline depth_iterator_t<SectionT, MorphologyT>::depth_iterator_t(const depth_iterator_t& other)
    : deque_(other.deque_) {}

template <typename SectionT, typename MorphologyT>
inline SectionT depth_iterator_t<SectionT, MorphologyT>::operator*() const {
    return deque_.front();
}

template <typename SectionT, typename MorphologyT>
inline SectionT const* depth_iterator_t<SectionT, MorphologyT>::operator->() const {
    return &deque_.front();
}

template <typename SectionT, typename MorphologyT>
inline depth_iterator_t<SectionT, MorphologyT>&
depth_iterator_t<SectionT, MorphologyT>::operator++() {
    if (deque_.empty()) {
        throw MorphioError("Can't iterate past the end");
    }

    const auto children = detail::getChildren(deque_.front());
    deque_.pop_front();
    std::copy(children.rbegin(), children.rend(), std::front_inserter(deque_));

    return *this;
}

template <typename SectionT, typename MorphologyT>
inline depth_iterator_t<SectionT, MorphologyT> depth_iterator_t<SectionT, MorphologyT>::operator++(
    int) {
    depth_iterator_t ret(*this);
    ++(*this);
    return ret;
}

template <typename SectionT, typename MorphologyT>
inline bool depth_iterator_t<SectionT, MorphologyT>::operator==(
    const depth_iterator_t& other) const {
    return deque_ == other.deque_;
}

template <typename SectionT, typename MorphologyT>
inline bool depth_iterator_t<SectionT, MorphologyT>::operator!=(
    const depth_iterator_t& other) const {
    return !(*this == other);
}

// upstream_iterator_t class definition

template <typename SectionT>
inline upstream_iterator_t<SectionT>::upstream_iterator_t()
    : unused(0)
    , end(true) {}

template <typename SectionT>
inline upstream_iterator_t<SectionT>::upstream_iterator_t(const SectionT& section)
    : current(section) {}

template <typename SectionT>
inline upstream_iterator_t<SectionT>::upstream_iterator_t(const upstream_iterator_t& other) {
    end = other.end;
    if (!other.end) {
        new (&current) SectionT(other.current);
    }
}

template <typename SectionT>
inline upstream_iterator_t<SectionT>::~upstream_iterator_t() {
    if (!end) {
        this->current.~SectionT();
    }
}

template <typename SectionT>
inline SectionT upstream_iterator_t<SectionT>::operator*() const {
    return current;
}

template <typename SectionT>
inline SectionT const* upstream_iterator_t<SectionT>::operator->() const {
    return &current;
}

template <typename SectionT>
inline upstream_iterator_t<SectionT>& upstream_iterator_t<SectionT>::operator++() {
    if (end) {
        throw MissingParentError("Cannot call iterate upstream past the root node");
    } else if (detail::isRoot(current)) {
        end = true;
        this->current.~SectionT();
    } else {
        current = detail::getParent(current);
    }
    return *this;
}

template <typename SectionT>
inline upstream_iterator_t<SectionT> upstream_iterator_t<SectionT>::operator++(int) {
    upstream_iterator_t ret(*this);
    ++(*this);
    return ret;
}

template <typename SectionT>
inline bool upstream_iterator_t<SectionT>::operator==(const upstream_iterator_t& other) const {
    if (end || other.end) {
        return end == other.end;
    }
    return current == other.current;
}

template <typename SectionT>
inline bool upstream_iterator_t<SectionT>::operator!=(const upstream_iterator_t& other) const {
    return !(*this == other);
}

}  // namespace morphio
