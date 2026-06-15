#include "rebarsmart/space_list/SpaceListParser.h"

#include <cmath>
#include <limits>
#include <utility>

namespace tsrs::rebarsmart {
namespace {

constexpr std::string_view kOk = "RS_OK";
constexpr std::string_view kEmpty = "RS_SPACE_LIST_EMPTY";
constexpr std::string_view kTokenInvalid = "RS_SPACE_LIST_TOKEN_INVALID";
constexpr std::string_view kRepeatZero = "RS_SPACE_LIST_REPEAT_ZERO";
constexpr std::string_view kRepeatInvalid = "RS_SPACE_LIST_REPEAT_INVALID";
constexpr std::string_view kRepeatTooLarge = "RS_SPACE_LIST_REPEAT_TOO_LARGE";
constexpr std::string_view kSeparatorUnsupported = "RS_SPACE_LIST_SEPARATOR_UNSUPPORTED";
constexpr int kMaxRepeatCount = 100000;
constexpr std::size_t kMaxOutputCount = 100000;

SpaceListParseResult success(std::vector<double> valuesM)
{
    return SpaceListParseResult{true, std::move(valuesM), std::string{kOk}, -1};
}

SpaceListParseResult failure(std::string_view diagnosticCode, std::size_t offset)
{
    const auto safeOffset = offset > static_cast<std::size_t>(std::numeric_limits<int>::max())
                                ? std::numeric_limits<int>::max()
                                : static_cast<int>(offset);
    return SpaceListParseResult{false, {}, std::string{diagnosticCode}, safeOffset};
}

bool isAsciiSpace(char value)
{
    return value == ' ' || value == '\t' || value == '\r' || value == '\n';
}

std::size_t skipLeadingSpaces(std::string_view text, std::size_t offset)
{
    while (offset < text.size() && isAsciiSpace(text[offset])) {
        ++offset;
    }
    return offset;
}

std::size_t trimTrailingSpaces(std::string_view text, std::size_t begin, std::size_t end)
{
    while (end > begin && isAsciiSpace(text[end - 1])) {
        --end;
    }
    return end;
}

bool isChineseCommaAt(std::string_view text, std::size_t offset)
{
    return offset + 3 <= text.size()
        && static_cast<unsigned char>(text[offset]) == 0xEF
        && static_cast<unsigned char>(text[offset + 1]) == 0xBC
        && static_cast<unsigned char>(text[offset + 2]) == 0x8C;
}

bool isAsciiDigit(char value)
{
    return value >= '0' && value <= '9';
}

bool parsePositiveDouble(std::string_view token, double& value)
{
    if (token.empty()) {
        return false;
    }

    double integerPart = 0.0;
    std::size_t offset = 0;
    bool hasIntegerDigit = false;
    while (offset < token.size() && isAsciiDigit(token[offset])) {
        hasIntegerDigit = true;
        integerPart = integerPart * 10.0 + static_cast<double>(token[offset] - '0');
        if (!std::isfinite(integerPart)) {
            return false;
        }
        ++offset;
    }

    double fractionalPart = 0.0;
    double fractionalScale = 1.0;
    bool hasFractionalDigit = false;
    if (offset < token.size() && token[offset] == '.') {
        ++offset;
        while (offset < token.size() && isAsciiDigit(token[offset])) {
            hasFractionalDigit = true;
            fractionalScale *= 0.1;
            fractionalPart += static_cast<double>(token[offset] - '0') * fractionalScale;
            ++offset;
        }
    }

    if (offset != token.size() || (!hasIntegerDigit && !hasFractionalDigit)) {
        return false;
    }

    value = integerPart + fractionalPart;
    return std::isfinite(value) && value > 0.0;
}

enum class RepeatParseStatus {
    Ok,
    Zero,
    Invalid,
};

RepeatParseStatus parseRepeatCount(std::string_view token, int& repeatCount)
{
    if (token.empty()) {
        return RepeatParseStatus::Invalid;
    }

    int parsed = 0;
    for (const char value : token) {
        if (!isAsciiDigit(value)) {
            return RepeatParseStatus::Invalid;
        }

        const int digit = value - '0';
        if (parsed > (std::numeric_limits<int>::max() - digit) / 10) {
            return RepeatParseStatus::Invalid;
        }
        parsed = parsed * 10 + digit;
    }

    if (parsed == 0) {
        return RepeatParseStatus::Zero;
    }

    repeatCount = static_cast<int>(parsed);
    return RepeatParseStatus::Ok;
}

SpaceListParseResult appendTokenValues(std::string_view token,
                                       std::size_t tokenOffset,
                                       double unitScaleToM,
                                       std::vector<double>& valuesM)
{
    const std::size_t star = token.find('*');
    if (star != std::string_view::npos && token.find('*', star + 1) != std::string_view::npos) {
        return failure(kTokenInvalid, tokenOffset + star + 1);
    }

    double value = 0.0;
    int repeatCount = 1;

    if (star == std::string_view::npos) {
        if (!parsePositiveDouble(token, value)) {
            return failure(kTokenInvalid, tokenOffset);
        }
    } else {
        const std::string_view valueToken = token.substr(0, star);
        const std::string_view repeatToken = token.substr(star + 1);

        if (!parsePositiveDouble(valueToken, value)) {
            return failure(kTokenInvalid, tokenOffset);
        }

        switch (parseRepeatCount(repeatToken, repeatCount)) {
        case RepeatParseStatus::Ok:
            break;
        case RepeatParseStatus::Zero:
            return failure(kRepeatZero, tokenOffset + star + 1);
        case RepeatParseStatus::Invalid:
            return failure(repeatToken.empty() ? kTokenInvalid : kRepeatInvalid,
                           tokenOffset + star + 1);
        }
    }

    if (repeatCount > kMaxRepeatCount
        || valuesM.size() > kMaxOutputCount - static_cast<std::size_t>(repeatCount)) {
        return failure(kRepeatTooLarge, star == std::string_view::npos ? tokenOffset
                                                                       : tokenOffset + star + 1);
    }

    const double valueM = value * unitScaleToM;
    if (!std::isfinite(valueM) || valueM <= 0.0) {
        return failure(kTokenInvalid, tokenOffset);
    }

    valuesM.insert(valuesM.end(), static_cast<std::size_t>(repeatCount), valueM);
    return SpaceListParseResult{true, {}, std::string{kOk}, -1};
}

} // namespace

SpaceListParseResult parseSpaceList(std::string_view text, SpaceListParseOptions options)
{
    if (!std::isfinite(options.unitScaleToM) || options.unitScaleToM <= 0.0) {
        return failure(kTokenInvalid, 0);
    }

    std::vector<double> valuesM;
    std::size_t tokenBegin = skipLeadingSpaces(text, 0);
    if (tokenBegin == text.size()) {
        return failure(kEmpty, 0);
    }

    while (tokenBegin < text.size()) {
        std::size_t tokenEnd = tokenBegin;
        while (tokenEnd < text.size() && text[tokenEnd] != ',') {
            if (isChineseCommaAt(text, tokenEnd)) {
                if (!options.allowChineseComma) {
                    return failure(kSeparatorUnsupported, tokenEnd);
                }
                break;
            }
            ++tokenEnd;
        }

        const std::size_t trimmedEnd = trimTrailingSpaces(text, tokenBegin, tokenEnd);
        if (trimmedEnd == tokenBegin) {
            return failure(kTokenInvalid, tokenBegin);
        }

        auto tokenResult = appendTokenValues(text.substr(tokenBegin, trimmedEnd - tokenBegin),
                                             tokenBegin,
                                             options.unitScaleToM,
                                             valuesM);
        if (!tokenResult.ok) {
            return tokenResult;
        }

        if (tokenEnd >= text.size()) {
            break;
        }

        const std::size_t separatorOffset = tokenEnd;
        tokenBegin = isChineseCommaAt(text, tokenEnd) ? tokenEnd + 3 : tokenEnd + 1;
        tokenBegin = skipLeadingSpaces(text, tokenBegin);
        if (tokenBegin == text.size()) {
            return failure(kTokenInvalid, separatorOffset);
        }
    }

    return valuesM.empty() ? failure(kEmpty, 0) : success(std::move(valuesM));
}

} // namespace tsrs::rebarsmart
