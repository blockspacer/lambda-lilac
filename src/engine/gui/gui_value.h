#pragma once
#include <containers/containers.h>

namespace lambda
{
	namespace gui
	{
		class JSVal;
		typedef bool MarbleBool;
		typedef float MarbleNumber;
		typedef const char* MarbleString;

		enum JSValType
		{
			val_none,
			val_bool,
			val_number,
			val_string,
		};

		class JSVal
		{
		public:
			JSVal();
			JSVal(const MarbleBool& boolean);
			JSVal(const MarbleNumber& number);
			JSVal(const MarbleString& string);
			JSVal(const JSVal& other);
			~JSVal();
			JSVal& operator=(const JSVal& other);
			bool operator==(const JSVal& other) const;
			bool operator<(const JSVal& other) const;
			bool operator>(const JSVal& other) const;
			bool operator<=(const JSVal& other) const;
			bool operator>=(const JSVal& other) const;
			bool operator!=(const JSVal& other) const;
			JSVal operator+(const JSVal& other) const;
			JSVal operator-(const JSVal& other) const;
			JSVal operator*(const JSVal& other) const;
			JSVal operator/(const JSVal& other) const;
			JSVal operator-() const;

			bool isBool() const;
			bool isNumber() const;
			bool isString() const;

			const MarbleBool& asBool() const;
			const MarbleNumber& asNumber() const;
			const MarbleString& asString() const;

			MarbleBool& asBool();
			MarbleNumber& asNumber();
			MarbleString& asString();

		private:
			void release();

			void* data_;
			JSValType type_;
		};
	}
}