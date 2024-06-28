#pragma once

#include <stdint.h>
#include <assert.h>

typedef int32_t AddonSignature;


// basically rusts option type

template<typename T>
class Option {
	bool hasValue;

public:
	/// don't access this directly
	T value;

	Option(T&& value_) :value(value_), hasValue(true) {  }
	Option() : hasValue(false) {  }

	inline bool HasValue() const {
		return this->hasValue;
	}

	inline T& Access() {
		assert(this->hasValue);
		return this->value;
	}
};

template<typename T>
class Option<T*> {

public:
	/// don't access this directly
	T* value;

	Option(T* value_) :value(value_) {  }
	Option() : value(0) {  }

	inline bool HasValue() {
		return this->value != 0;
	}

	inline T* Access() {
		assert(this->value != 0);
		return this->value;
	}
};

static_assert(sizeof(Option<char*>) == sizeof(char*));

template<typename T>
inline Option<T> Some(T& value) { return Option<T>(value); }

template<size_t S>
inline Option<char const*> Some(char const(&value)[S]) { return Option((char const *)value); }

template<typename T>
inline Option<T> None() { return Option<T>(); }

#define IF_SOME(expr, body) { \
 auto opt = expr; \
 auto& it = opt.value; \
 if(opt.HasValue()) body \
}
