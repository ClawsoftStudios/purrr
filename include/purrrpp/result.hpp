#ifndef   _PURRR_RESULT_HPP_
#define   _PURRR_RESULT_HPP_

namespace purrr {

class Result {
public:
  enum Value {
    FileSystemError  = -4,
    InternalError    = -3,
    InvalidArgsError = -2,
    BuyMoreRam       = -1,
    Success          =  0,
    True             =  1,
    Minimized        =  2,
    Inactive         =  3,
  };
public:
  Result() = default;

  inline Result(Purrr_Result result)
    : mValue((Value)result)
  {}

  inline Result(Value value)
    : mValue(value)
  {}

  inline Result(const Result &other)
  {
    *this = other;
  }

  inline Result(Result &&other)
    : mValue(other.mValue)
  {
    other.mValue = Success;
  }

  inline Result &operator=(const Result &other) {
    if (this == &other) return *this;

    mValue = other.mValue;

    return *this;
  }

  const char *toCstr() const {
    switch (mValue) {
    case FileSystemError: return "FileSystemError";
    case InternalError: return "InternalError";
    case InvalidArgsError: return "InvalidArgsError";
    case BuyMoreRam: return "BuyMoreRam";
    case Success: return "Success";
    case True: return "True";
    case Minimized: return "Minimized";
    case Inactive: return "Inactive";
    }

    return nullptr;
  }

  inline operator bool() { return mValue >= Success; }
  inline operator bool() const { return mValue >= Success; }

  inline bool operator==(const Value &other) { return mValue == other; }
  inline bool operator==(const Result &other) { return mValue == other.mValue; }

  friend std::ostream &operator<<(std::ostream &os, const Result &result);

  inline operator Value &() { return mValue; }
  inline operator const Value &() const { return mValue; }

  inline Value &value() { return mValue; }
  inline const Value &value() const { return mValue; }
private:
  Value mValue = Success;
};

inline std::ostream &operator<<(std::ostream &os, const Result &result) {
  return os << result.toCstr();
}

class ResultException {
public:
  inline ResultException(const Result &result)
    : mResult(result)
  {}

  inline operator const Result &() { return mResult; }
  inline const Result &result() const { return mResult; }
private:
  Result mResult;
};

}

#endif // _PURRR_RESULT_HPP_