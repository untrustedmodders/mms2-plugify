#include "mm_vhook.hpp"

#include <ISmmPlugin.h>

using namespace mm;
using namespace SourceHook;

extern SourceHook::ISourceHook* g_SHPtr;
extern SourceHook::IHookManagerAutoGen* g_pHookManager;
extern PluginId g_PLID;

struct vector_t {
	float x;
	float y;
	float z;
};

std::pair<size_t, PassInfo::PassType> GetParamInfo(DataType type) {
	switch (type) {
		case DataType::Void:
			return { 0, SourceHook::PassInfo::PassType_Unknown };
		case DataType::Bool:
			return { sizeof(bool), PassInfo::PassType_Basic };
		case DataType::Int8:
			return { sizeof(int8_t), PassInfo::PassType_Basic };
		case DataType::UInt8:
			return { sizeof(uint8_t), PassInfo::PassType_Basic };
		case DataType::Int16:
			return { sizeof(int16_t), PassInfo::PassType_Basic };
		case DataType::UInt16:
			return { sizeof(uint16_t), PassInfo::PassType_Basic };
		case DataType::Int32:
			return { sizeof(int32_t), PassInfo::PassType_Basic };
		case DataType::UInt32:
			return { sizeof(uint32_t), PassInfo::PassType_Basic };
		case DataType::Int64:
			return { sizeof(int64_t), PassInfo::PassType_Basic };
		case DataType::UInt64:
			return { sizeof(uint64_t), PassInfo::PassType_Basic };
		case DataType::Float:
			return { sizeof(float), PassInfo::PassType_Float };
		case DataType::Double:
			return { sizeof(double), PassInfo::PassType_Float };
		case DataType::Pointer:
			return { sizeof(void*), PassInfo::PassType_Basic };
		case DataType::String:
			return { sizeof(string_t), PassInfo::PassType_Object };
		case DataType::Vector:
			return { sizeof(vector_t), PassInfo::PassType_Object };
	}

	return { 0, PassInfo::PassType_Unknown };
}

void HookCallback::DeleteThis() {
	*reinterpret_cast<void***>(this) = this->oldvtable;
	delete this->newvtable;
	delete this;
}

HookCallback* HookCallback::Create(void* function) {
	HookCallback* dg = new HookCallback();
	dg->oldvtable = *reinterpret_cast<void***>(dg);
	dg->newvtable = new void *[3];
	dg->newvtable[0] = dg->oldvtable[0];
	dg->newvtable[1] = dg->oldvtable[1];
	dg->newvtable[2] = function;
	*reinterpret_cast<void***>(dg) = dg->newvtable;
	return dg;
}

HookManager::HookManager(void* iface, mm::DataType returnType, std::span<const mm::DataType> paramsType, void* function, int offset, bool post) {
	CProtoInfoBuilder protoInfo(ProtoInfo::CallConv_ThisCall);

	for (const auto& paramType : paramsType) {
		auto [size, type] = GetParamInfo(paramType);
		protoInfo.AddParam(size, type, size != 0 ? PassInfo::PassFlag_ByVal : 0, nullptr, nullptr, nullptr, nullptr);
	}
	auto [size, type] = GetParamInfo(returnType);
	protoInfo.SetReturnType(size, type, size != 0 ? PassInfo::PassFlag_ByVal : 0, nullptr, nullptr, nullptr, nullptr);

	m_callback = HookCallback::Create(function);
	m_manager = g_pHookManager->MakeHookMan(protoInfo, 0, offset);
	m_hookid = g_SHPtr->AddHook(g_PLID, ISourceHook::Hook_Normal, iface, 0, m_manager, m_callback, post);
}

HookManager::~HookManager() {
	if (m_hookid) {
		g_SHPtr->RemoveHookByID(m_hookid);
		if (m_manager) {
			g_pHookManager->ReleaseHookMan(m_manager);
		}
	}
}
