#pragma once

//====================================================
//
// システム系のヘッダーファイル
//
//====================================================

// ウィンドウ
#include "Window/KdWindow.h"

// 便利機能
#include "Utility/KdUtility.h"
#include "Utility/KdCSVData.h"
#include "Utility/KdFPSController.h"
#include"Utility/KdFrameTimer.h"
#include "Utility/KdRandom.h"

// 音関連
#include "Audio/KdAudio.h"

// バッファ
#include "Direct3D/KdBuffer.h"

// テクスチャ
#include "Direct3D/KdTexture.h"
// シェーダー描画用マテリアル
#include "Direct3D/KdMaterial.h"
// メッシュ
#include "Direct3D/KdMesh.h"
// モデル
#include "Direct3D/KdModel.h"
// データ保管庫：テンプレート
#include "Utility/KdDataStorage.h"

// ポリゴン基底
#include "Direct3D/Polygon/KdPolygon.h"
// 板ポリゴン
#include "Direct3D/Polygon/KdSquarePolygon.h"
// 軌跡ポリゴン
#include "Direct3D/Polygon/KdTrailPolygon.h"

// Direct3D
#include "Direct3D/KdDirect3D.h"

// カメラ
#include "Direct3D/KdCamera.h"

// アニメーション
#include "Math/KdAnimation.h"
// コマ送りアニメーション
#include "Math/KdUVAnimation.h"
// メッシュとポリゴンの接触判定
#include "Math/KdCollision.h"
// 当たり判定登録
#include "Math/KdCollider.h"
// 数値に緩急を付ける機能
#include "Math/KdEasing.h"

// 入力関連
#include "Input/KdInput.h"

// レンダーターゲット切替
#include "Shader/KdRenderTargetChange.h"

// シェーダ
#include "Shader/KdAmbientController.h"
#include "Shader/KdShaderManager.h"

// デバッグ機能
#include "Utility/KdDebugWireFrame.h"
#include "Utility/KdDebugGUI.h"

// ゲームオブジェクト関連
#include "GameObject/KdGameObject.h"
#include "GameObject/KdGameObjectFactory.h"

// Effekseer管理クラス
#include "Effekseer/KdEffekseerManager.h"
