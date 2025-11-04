#include"Field.h"
#include"../../Scene/SceneManager.h"
const uint32_t Field::TypeID = KdGameObject::GenerateTypeID();

void Field::DrawLit()
{
	//KdShaderManager::Instance().ChangeRasterizerState(KdRasterizerState::CullFrontWireframe);
	//KdShaderManager::Instance().m_StandardShader.SetDitherEnable(false);
	SelectDraw3dModel::DrawLit();
	//KdShaderManager::Instance().UndoRasterizerState();
}

void Field::DrawGrayScale()
{
	if (!SceneManager::Instance().GetDrawGrayScale()) return;

	SelectDraw3dModel::DrawGrayScale();
}

