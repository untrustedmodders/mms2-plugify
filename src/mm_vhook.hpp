#pragma once

#include <plugify/vector.hpp>
#include <sourcehook/sourcehook_pibuilder.h>
#include <sourcehook/sourcehook_impl.h>
#include <sourcehook/sourcehook_impl_chookmaninfo.h>

namespace mm {
	enum class DataType : uint8_t {
		Void,
		Bool,
		Int8,
		UInt8,
		Int16,
		UInt16,
		Int32,
		UInt32,
		Int64,
		UInt64,
		Float,
		Double,
		Pointer,
		String,
		Vector
		// TODO: Add support of POD types
	};

	class HookCallback final : public SourceHook::ISHDelegate {
	public:
		bool IsEqual(ISHDelegate*) override { return false; };
		void DeleteThis() override;

		virtual void Call() {};

		static HookCallback* Create(void* function);

	public:
		void** newvtable;
		void** oldvtable;
	};

	class HookManager {
	public:
		HookManager(void* iface, mm::DataType returnType, std::span<const mm::DataType> paramsType, void* function, int offset, bool post);
		~HookManager();

	public:
		int m_hookid;
		HookCallback* m_callback;
		SourceHook::HookManagerPubFunc m_manager;
	};
} // namespace mm
