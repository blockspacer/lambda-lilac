#include "gui_value.h"
#include <memory/memory.h>
#include <utils/console.h>

namespace lambda
{
	namespace gui
	{
		JSVal::JSVal()
			: type_(val_none)
			, data_(nullptr)
		{
		}

		JSVal::JSVal(const MarbleBool& boolean)
			: type_(val_bool)
		{
			data_ = foundation::Memory::allocate(sizeof(MarbleBool));
			new (data_) MarbleBool();
			(*(MarbleBool*)data_) = boolean;
		}

		JSVal::JSVal(const MarbleNumber& number)
			: type_(val_number)
		{

			data_ = foundation::Memory::allocate(sizeof(MarbleNumber));
			new (data_) MarbleNumber();
			(*(MarbleNumber*)data_) = number;
		}

		JSVal::JSVal(const MarbleString& string)
			: type_(val_string)
		{
			size_t len = strlen(string) + 1;
			data_ = foundation::Memory::allocate(len);
			memcpy(data_, string, len);
		}

		JSVal::JSVal(const JSVal& other)
			: type_(other.type_)
		{
			switch (type_)
			{
			case val_bool:
				data_ = foundation::Memory::allocate(sizeof(MarbleBool));
				new (data_) MarbleBool();
				(*(MarbleBool*)data_) = other.asBool();
				break;
			case val_number:
				data_ = foundation::Memory::allocate(sizeof(MarbleNumber));
				new (data_) MarbleNumber();
				(*(MarbleNumber*)data_) = other.asNumber();
				break;
			case val_string:
			{
				size_t len = strlen(other.asString()) + 1;
				data_ = foundation::Memory::allocate(len);
				memcpy(data_, other.asString(), len);
				break;
			}
			}
		}

		JSVal::~JSVal()
		{
			release();
		}

		JSVal& JSVal::operator=(const JSVal& other)
		{
			release();

			type_ = other.type_;

			switch (type_)
			{
			case val_bool:
				data_ = foundation::Memory::allocate(sizeof(MarbleBool));
				new (data_) MarbleBool();
				(*(MarbleBool*)data_) = other.asBool();
				break;
			case val_number:
				data_ = foundation::Memory::allocate(sizeof(MarbleNumber));
				new (data_) MarbleNumber();
				(*(MarbleNumber*)data_) = other.asNumber();
				break;
			case val_string:
			{
				size_t len = strlen(other.asString()) + 1;
				data_ = foundation::Memory::allocate(len);
				memcpy(data_, other.asString(), len);
				break;
			}
			}

			return *this;
		}

		bool JSVal::operator==(const JSVal& other) const
		{
			if (type_ != other.type_)
				return false;

			switch (type_)
			{
			case val_bool:
				return asBool() == other.asBool();
			case val_number:
				return asNumber() == other.asNumber();
			case val_string:
				return (strlen(asString()) == strlen(other.asString()) &&
					memcmp((void*)asString(), (void*)other.asString(), strlen(asString())));
			}

			return false;
		}

		bool JSVal::operator<(const JSVal& other) const
		{
			if (type_ != other.type_)
				return false;

			switch (type_)
			{
			case val_number:
				return asNumber() < other.asNumber();
			}

			return false;
		}

		bool JSVal::operator>(const JSVal& other) const
		{
			if (type_ != other.type_)
				return false;

			switch (type_)
			{
			case val_number:
				return asNumber() > other.asNumber();
			}

			return false;
		}

		bool JSVal::operator<=(const JSVal& other) const
		{
			if (type_ != other.type_)
				return false;

			switch (type_)
			{
			case val_number:
				return asNumber() <= other.asNumber();
			}

			return false;
		}

		bool JSVal::operator>=(const JSVal& other) const
		{
			if (type_ != other.type_)
				return false;

			switch (type_)
			{
			case val_number:
				return asNumber() >= other.asNumber();
			}

			return false;
		}

		bool JSVal::operator!=(const JSVal& other) const
		{
			if (type_ != other.type_)
				return true;

			switch (type_)
			{
			case val_bool:
				return asBool() != other.asBool();
			case val_number:
				return asNumber() != other.asNumber();
			case val_string:
				return (strlen(asString()) != strlen(other.asString()) ||
					!memcmp((void*)asString(), (void*)other.asString(), strlen(asString())));
			}

			return true;
		}

		JSVal JSVal::operator+(const JSVal& other) const
		{
			switch (type_)
			{
			case val_number:
				return JSVal(asNumber() + other.asNumber());
			case val_string:
			{
				MarbleString concat = (MarbleString)foundation::Memory::allocate(strlen(asString()) + strlen(other.asString()) + 1);

				memcpy((void*)concat, asString(), strlen(asString()));
				memcpy((void*)(concat + strlen(asString())), other.asString(), strlen(other.asString()) + 1);

				JSVal val(concat);
				foundation::Memory::deallocate((void*)concat);
				return val;
			}
			}

			return JSVal();
		}

		JSVal JSVal::operator-(const JSVal& other) const
		{
			switch (type_)
			{
			case val_number:
				return JSVal(asNumber() - other.asNumber());
			}

			return JSVal();
		}

		JSVal JSVal::operator*(const JSVal& other) const
		{
			switch (type_)
			{
			case val_number:
				return JSVal(asNumber() * other.asNumber());
			}

			return JSVal();
		}

		JSVal JSVal::operator/(const JSVal& other) const
		{
			switch (type_)
			{
			case val_number:
				return JSVal(asNumber() / other.asNumber());
			}

			return JSVal();
		}

		JSVal JSVal::operator-() const
		{
			switch (type_)
			{
			case val_bool:
				return JSVal(!asBool());
			case val_number:
				return JSVal(-asNumber());
			}

			return JSVal();
		}

		bool JSVal::isBool() const
		{
			return type_ == val_bool;
		}

		bool JSVal::isNumber() const
		{
			return type_ == val_number;
		}

		bool JSVal::isString() const
		{
			return type_ == val_string;
		}

		const MarbleBool& JSVal::asBool() const
		{
			LMB_ASSERT(type_ == val_bool, "Was not a boolean");
			return *(MarbleBool*)data_;
		}

		const MarbleNumber& JSVal::asNumber() const
		{
			LMB_ASSERT(type_ == val_number, "Was not a number");
			return *(MarbleNumber*)data_;
		}

		const MarbleString& JSVal::asString() const
		{
			LMB_ASSERT(type_ == val_string, "Was not a string");
			return (const char*&)data_;
		}

		MarbleBool& JSVal::asBool()
		{
			LMB_ASSERT(type_ == val_bool, "Was not a boolean");
			return *(MarbleBool*)data_;
		}

		MarbleNumber& JSVal::asNumber()
		{
			LMB_ASSERT(type_ == val_number, "Was not a number");
			return *(MarbleNumber*)data_;
		}

		MarbleString& JSVal::asString()
		{
			LMB_ASSERT(type_ == val_string, "Was not a string");
			return (const char*&)data_;
		}

		void JSVal::release()
		{
			if (type_ == val_none)
				return;

			foundation::Memory::deallocate(data_);

			type_ = val_none;
			data_ = nullptr;
		}
	}
}