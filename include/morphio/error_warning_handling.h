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


class ErrorAndWarningHandlerCollector : public ErrorAndWarningHandler
{
public:
    void emit(const enums::Warning& warning, const std::string& msg) final {
        std::cout << "************emit: " << msg << '\n';
        m.emplace_back(warning, msg);
    }

    void printAll(){
        std::cout << "printing all errors:\n";
        for(const auto& e : m){
            std::cout << "************error: " << std::get<1>(e) << '\n';
        }
    }

    std::vector<std::pair<enums::Warning, std::string>> m;
};


}  // namespace morphio
