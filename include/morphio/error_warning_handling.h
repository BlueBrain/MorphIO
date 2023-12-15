/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <set>
#include <string>
#include <vector>
#include <iostream>

#include <morphio/enums.h>

namespace morphio {

class ErrorAndWarningHandler
{
  public:
    ErrorAndWarningHandler() = default;
    ErrorAndWarningHandler(ErrorAndWarningHandler&&) = default;
    ErrorAndWarningHandler& operator=(const ErrorAndWarningHandler&) = default;
    ErrorAndWarningHandler& operator=(ErrorAndWarningHandler&&) = default;
    ErrorAndWarningHandler(ErrorAndWarningHandler&) = default;
    virtual ~ErrorAndWarningHandler() = default;

    virtual void emit(const enums::Warning& warning, const std::string& msg) = 0;

    bool isIgnored(enums::Warning warning) {
        return ignoredWarnings_.find(warning) != ignoredWarnings_.end();
    }

    void setIgnoredWarning(enums::Warning warning, bool ignore) {
        if (ignore) {
            ignoredWarnings_.insert(warning);
        } else {
            ignoredWarnings_.erase(warning);
        }
    }

    int getMaxWarningCount() const {
        return maxWarningCount_;
    }
    void setMaxWarningCount(int warningCount) {
        maxWarningCount_ = warningCount;
    }

    bool getRaiseWarnings() const {
        return raiseWarnings_;
    }
    void setRaiseWarnings(bool raise) {
        raiseWarnings_ = raise;
    }

  private:
    int maxWarningCount_ = 100;
    bool raiseWarnings_ = false;
    std::set<enums::Warning> ignoredWarnings_;
};


struct Caution {
    Caution() = default;
    Caution(enums::Warning level_, std::string msg_)
        : level(level_)
        , msg(msg_) {}
    enums::Warning level = enums::Warning::UNDEFINED;
    std::string msg;
};

class ErrorAndWarningHandlerCollector : public ErrorAndWarningHandler
{
public:
    void emit(const enums::Warning& warning, const std::string& msg) final {
        m.emplace_back(warning, msg);
    }

    void printAll(){
        std::cout << "printing all errors:\n";
        for(const auto& e : m){
            std::cout << "************error: " << e.msg << '\n';
        }
    }

    std::vector<Caution> getAll() const {
        return m;
    }

  private:
    std::vector<Caution> m;
};

}  // namespace morphio
