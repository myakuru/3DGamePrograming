#include"SpecialAttackSmoke.h"
#include"../../../../main.h"
#include"../../../../../MyFramework/Manager/ImGuiManager/ImGuiManager.h"

const uint32_t SpecialAttackSmoke::TypeID = KdGameObject::GenerateTypeID();

void SpecialAttackSmoke::DrawLit()
{
	if (!IMGUI_MANAGER.GetShowEffect()) return;


	KdShaderManager::Instance().ChangeDepthStencilState(KdDepthStencilState::ZWriteDisable);

	KdEffekseerManager::GetInstance().Draw();

	KdShaderManager::Instance().UndoDepthStencilState();
}
